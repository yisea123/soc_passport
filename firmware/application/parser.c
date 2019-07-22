#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <errno.h>

#include <command.h>
#include "cmdprocs.h"
#include "response.h"
#include "tasksync.h"
#include "debug.h"
#include "diag.h"
#include "usbdrv.h"

extern int quit;

#ifdef USE_LOCAL_BUFFER
static int data_saved = 0;
static uint8_t *rdataptr;
static uint8_t recvbuffer[2048] = {0};
#endif
uint8_t commandbuff[2*1024*1024];

struct command_entry{
	uint16_t command;
	int (*execute)(comm_packet_head_t *cmdhead, void *data);
};


const struct command_entry command_table[]={
//	{CMD_GET_DEVICE_STATUS,		cmd_get_device_status},
//	{CMD_GET_SENSOR_STATUS,		cmd_get_sensor_status},
//	{CMD_GET_DEVICE_ERROR,		cmd_get_device_error},
//	{CMD_GET_LIFETIME_INFO,		cmd_get_lifetime_info},
//	{CMD_GET_STATISTICS,		cmd_get_statistics},
//	{CMD_CLEAR_STATISTICS,		cmd_clear_statistics},
//	{CMD_GET_DEVICE_INFO,		cmd_get_device_info},
//	{CMD_GET_DEVICE_PARTLIST,	cmd_get_device_partlist},
	{CMD_RESET,			cmd_reset},
//	{CMD_EJECT_FRONT,		cmd_eject_front},
//	{CMD_EJECT_REAR,		cmd_eject_rear},
//	{CMD_RETURN_FRONT,		cmd_return_front},
//	{CMD_MEDIA_ALIGNMENT,		cmd_media_alignment},
//	{CMD_ENTER_WAITALIGNMENT,	cmd_enter_wait_alignment},
	{CMD_START_SCANNING,		cmd_start_scanning},
	{CMD_STOP_SCANNING,		cmd_stop_scanning},
	{CMD_SET_SCANMODE,		cmd_set_scan_mode},
	{CMD_GET_IMG_CALIB_DATA,	cmd_get_image_calib_data},
	{CMD_SET_IMG_CALIB_DATA,	cmd_set_image_calib_data},
//	{CMD_GET_SCAN_FEATURE,		cmd_get_scan_feature }, 
//	{CMD_DUMMY_SCANNING,		cmd_dummy_scanning},
	{CMD_SCANNING_ADJUSTMENT,	cmd_scanning_adjustment},
	{CMD_SCAN_CALIB_DATA,		cmd_scan_calibrate_data},
	{CMD_SCANNING_CALIBRATION,	cmd_scanning_calibration},
//	{CMD_RESET_ACCEPTOR,		cmd_reset_acceptor},
//	{CMD_ACCEPT_MEDIA,		cmd_accept_media},
#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
//	{CMD_SELECT_ACCEPTOR,		cmd_acceptor_path_select},
#endif
//	{CMD_CALIBRATE_SENSOR,		cmd_calibrate_sensor},
//	{CMD_UPGRADE_FW,		cmd_upgrade_firmware},
//	{CMD_MOVE_MEDIA,		cmd_move_media},
//	{CMD_STOP_MEDIA,		cmd_stop_media},
//	{CMD_OPEN_ALIGNER,		cmd_open_aligner},
//	{CMD_CLOSE_ALIGNER,		cmd_close_aligner},
	{CMD_RESET_SCANUNIT,		cmd_reset_scanunit},
	{CMD_GET_IMG_EXPOSURE_TIME,	cmd_get_img_exposure_time},
	{CMD_SET_IMG_EXPOSURE_TIME,	cmd_set_img_exposure_time},
	{CMD_GET_IMG_AFE_GAIN,		cmd_get_img_afe_gain},
	{CMD_SET_IMG_AFE_GAIN,		cmd_set_img_afe_gain},
//	{CMD_GET_CLOCKTIME,		cmd_get_clocktime},
	{CMD_SET_CLOCKTIME,		cmd_nop},
//	{CMD_GET_LOG,			cmd_get_log},
#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES_PRINTER)
//	{CMD_PRINT_INKJETPATH_LOCATE,	cmd_inkjetpath_locate},	
//	{CMD_PRINT_INKJETPATH_RESET,	cmd_inkjetpath_reset},
//	{CMD_FRONT_GATE,		cmd_prepare},
#endif
#if defined(MX6Q_VOUCHER_SCANNER_PRINTER)
	{CMD_PRINTER_INITIALIZE,	cmd_printer_initialize},
	{CMD_PRINTER_STARTPRING,	cmd_printer_startprint},
	{CMD_PRINTER_STOPPRING,		cmd_printer_stopprint},
	{CMD_PRINTER_SENDMSG,		cmd_printer_send_msg},
	{CMD_PRINTER_DYNAMICIMAGE,	cmd_printer_dynamic_image},
	{CMD_PRINTER_DYNAMICTEXT,	cmd_printer_dynamictext},
	{CMD_PRINTER_EJECT_TO_ALIGNER,	cmd_printer_eject_to_aligner},
	{CMD_PRINTER_MOVE_TO_PRINT,	cmd_printer_move_to_print},
#endif
};
#define NUM_COMMANDS	(sizeof(command_table)/sizeof(struct command_entry))

#define SET_HSUSB_SOFTRESET	_IOW('s', 17, unsigned char)

#ifdef USE_LOCAL_BUFFER
static inline int read_usb(void *buffer, size_t nbytes)
{
	int rs, cnt;

	if (data_saved == 0) 
	{
		rs = usbd_read(recvbuffer, sizeof(recvbuffer));
		if(rs <= 0)
			return rs;

		data_saved = rs;
		rxdata_count = 0;
		rdataptr = recvbuffer;
	}
	cnt = (nbytes < data_saved) ? nbytes : data_saved;
	memcpy(buffer, rdataptr, cnt);
	rdataptr += cnt;
	data_saved -= cnt;
	return cnt;
}
#else
#define read_usb(buffer, len)	usbd_read(buffer, len)
#endif

int host_get_command(void *buffer, int buffsize)
{
	comm_packet_head_t *cmd = buffer;
	int expected, count = 0;
	unsigned char *bufptr;

	bufptr = buffer;
	expected = sizeof(comm_packet_head_t);
	while (expected > 0)
	{
		int rlen;

		if ((count+expected) > buffsize)	// buffer boundary check
			break;

//		printf("try to read %d bytes: \r\n", expected);
		rlen = read_usb(bufptr, expected);
		
//		printf("rlen = %d.\r\n", rlen);
		if (rlen == -1)
			return (count == 0) ? -1 : count;

		if (count > 0)
		{
			count += rlen;
			bufptr += rlen;
			expected -= rlen;
		}
		else if (cmd->deviceid== DEVICE_ID)
		{
			count = rlen;
			bufptr += rlen;
			expected = cmd->datalength;
		}
	}
	return count;
}


int _host_send_response(int mode, comm_packet_head_t *replyhead, void *data, const int count)
{
	unsigned char *dataptr;
	int sent, left;
	int rs;

	//printf("_host_send_response: %d bytes of data to send, pid = %d:\n", count, replyhead->pid);

	replyhead->datalength = count;
	if (mode == SEND_RESP_SINGLE)
		replyhead->pflag = PACKET_FLAG_XMIT_END;

	// send response packet head
	rs =  usbd_write((const uint8_t*)replyhead, sizeof(comm_packet_head_t));
	if(rs < 0)
		return rs;	// return error sending packet head to host
	if (count == 0)
		return 0;	// no data to send, return immediately

	sent = 0;
	left = count;
	dataptr = data;
	while (sent < count)
	{
		rs = usbd_write(dataptr, left);
		if (rs < 0)
			return rs;	// error sending data to host
		dataptr += rs;
		sent += rs;
		left -= rs;
	};

	if (mode == SEND_RESP_MULTIPLE)
		++replyhead->pid;	// update PID number for sending next packet of a multi-packet response
//	printf("_host_send_response: send end!\n");
	return rs;
}

int task_parser(int mode)
{
	comm_packet_head_t *cmd_head;
	void *cmd_data;

	cmd_head = (comm_packet_head_t *)commandbuff;
	cmd_data = commandbuff + sizeof(comm_packet_head_t);

	while(1)
	{
		int i, rxlen, rs;
		uint16_t command;

		memset(commandbuff, 0, sizeof(comm_packet_head_t));
		rxlen = host_get_command(commandbuff, sizeof(commandbuff));	/*receive command packet*/
		if(rxlen <= 0)
			continue;
		command = cmd_head->command;
		printf("got a command '%c' (%02X), total %d bytes:\r\n", isprint(command)?command:' ', command, rxlen);
		for (i=0; i<NUM_COMMANDS; i++) {
			if (command == command_table[i].command)
				break;
		}
		if (i >= NUM_COMMANDS) {	// undefined command	
//			printf("undefined command '%c%c'(%04X)\n", CMD_BYTE1(command), CMD_BYTE2(command), command);
			rs = cmd_undefined(cmd_head, cmd_data);
		}
		else
		{
			/* execute a command */
			if (command_table[i].execute != NULL)
				rs = command_table[i].execute(cmd_head, cmd_data);
		}
	}
//	printf("task parser exit!\n");
	return 0;
}


