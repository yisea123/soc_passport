#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <command.h>
#include "cmdprocs.h"
#include "response.h"
#include "paperdrv.h"
#include "scanner.h"
#include "debug.h"
#include "spv_fun.h"
#include "respcode.h"
#include "resp_voucher_scanner.h"

extern unsigned short get_media_status(void);


#define CALIBUF_LEN		(14*3*SCANLINE_PIXELS)
#define XMIT_BLOCK_SIZE		(2*1024*1024)

int transmit_mode = IMGDATA_XMIT_IN_SCANNING;


int cmd_reset_scanunit(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = get_media_status();
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_set_scan_mode(comm_packet_head_t *cmdhead, void *data)
{
	int mode;
	resp_packet_init(&reply_nodata, cmdhead);
	mode = cmdhead->para;
	if (mode != IMGDATA_XMIT_IN_SCANNING && mode != IMGDATA_XMIT_AFTER_SCANNING) {
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_INVALID_PARAMETER); 
		reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
	}
	else {
		transmit_mode = mode;
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_get_scan_feature(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = get_media_status();
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_get_image_calib_data(comm_packet_head_t *cmdhead, void *data)
{
	unsigned char *cali_buf = NULL;
//	struct stat buf;
	int rs;
	int size;

	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
//	err_info_init();

//	rs = stat("/home/config/calibrate.dat",&buf);
	if (rs == 0) {
		cali_buf = (unsigned char *)0xa2000000;
		if (cali_buf == NULL) {
			err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_MEM_ALLOC_ERROR);
			reply_nodata.status = RESN_APP_COMMON_MEM_ALLOC_ERROR;
			host_send_response(&reply_nodata, NULL, 0);
			return 0;
		}
		rs = 0;//scanner_read_calidata(cali_buf, 4032);
		if (rs != 0){
			free(cali_buf);
//			err_info_fill(RESN_CLASS_APP_CUSTOM, -RESN_APP_CUSTOM_READ_CALI_DATA_ERR); 
			reply_nodata.status = RESN_APP_CUSTOM_READ_CALI_DATA_ERR;
		}
		else
		{
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
			reply_nodata.status = get_media_status();
			reply_nodata.datalength = 4032;
			host_send_response(&reply_nodata, cali_buf, reply_nodata.datalength);
			free(cali_buf);
			return 0;
		}
	}
	else//filename not exists
	{
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_FILE_NOT_EXIST);
		reply_nodata.status = RESN_APP_COMMON_FILE_NOT_EXIST;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_set_image_calib_data(comm_packet_head_t *cmdhead, void *data)
{
	int rs;

//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.status = get_media_status();
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;

	if(cmdhead->datalength > 0) {
		rs = scanner_write_calidata(data, cmdhead->datalength); 
		if (rs != 0){
			reply_nodata.status = -rs;
//			err_info_fill_by_code(reply_nodata.status);
		}
		else
		{
			reply_nodata.status = RES_MEDIA_LOADED;
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		}
	}
	else{
//		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR); 
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}

	host_send_response(&reply_nodata, NULL, 0);

	return rs;
}


int cmd_scanning_adjustment(comm_packet_head_t *cmdhead, void *data)
{
	int rs;

	resp_packet_init(&reply_nodata, cmdhead);
//	err_info_init();

	rs = spv_do_scanning_adjustment();
	if (rs != 0)
	{
		reply_nodata.status = -rs;
//		err_info_fill_by_code(reply_nodata.status);
		reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	}
	else
	{
		reply_nodata.status = get_media_status();
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_scanning_calibration(comm_packet_head_t *cmdhead, void *data)
{
	int rs, mode;

//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	if (cmdhead->datalength == 0 || cmdhead->datalength == sizeof(scanning_lightsource_t))
	{
		if (cmdhead->datalength == 0)
			check_scanning_ctrl.lightsource.side_a = check_scanning_ctrl.lightsource.side_b = LIGHTSOURCE_ALL;
		else if (cmdhead->datalength == sizeof(scanning_lightsource_t))
			memcpy((void *)&check_scanning_ctrl.lightsource,data,sizeof(scanning_lightsource_t));
		rs = spv_do_scan_calibration(1, 100); 
		if (rs != 0){
			reply_nodata.status = -rs;
//			err_info_fill_by_code(reply_nodata.status);
		}
		else{
			reply_nodata.status = get_media_status();
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		}
	}
	else{
//		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_INVALID_PARAMETER);
		reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_scan_calibrate_data(comm_packet_head_t *cmdhead, void *data)
{
	int rs, lines;

	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	if (cmdhead->datalength == 0 || cmdhead->datalength == sizeof(scanning_lightsource_t)){
		if (cmdhead->datalength == 0)
			check_scanning_ctrl.lightsource.side_a = check_scanning_ctrl.lightsource.side_b = LIGHTSOURCE_ALL;
		else if (cmdhead->datalength == sizeof(scanning_lightsource_t))
			memcpy((void *)&check_scanning_ctrl.lightsource,data,sizeof(scanning_lightsource_t));
		{
			lines = 500;
//			rs = spv_do_scan_calibration(0, lines);
			if (rs == 0) {
				int datalen = check_scanning_ctrl.linecount * check_scanning_ctrl.linedlen;
				reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
				reply_nodata.status = get_media_status();
				host_send_response(&reply_nodata, scanbuffer, datalen);
				return 0;
			}
			reply_nodata.status = -rs;
			err_info_fill_by_code(reply_nodata.status);
		}
	}
	else{
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR);
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}

	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_start_scanning(comm_packet_head_t *cmdhead, void *data)
{
	int rs, mode;
	scanning_setting_t *setting;

//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
/*
	setting = (scanning_setting_t *)&cmdhead->para;
	check_scanning_ctrl.mode = setting->mode;
	check_scanning_ctrl.resolution = check_scanning_ctrl.hwresolution = setting->resolution;

	if (!((check_scanning_ctrl.mode == SCANMODE_SIX_LIGHTSOURCE || check_scanning_ctrl.mode == SCANMODE_TEN_LIGHTSOURCE)
	    && (check_scanning_ctrl.resolution == DPI_200)))
	{
//		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_INVALID_PARAMETER); 
		reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
	}
	else if (cmdhead->datalength != 0 && cmdhead->datalength != sizeof(scanning_lightsource_t)) {
//		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR); 
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}
	else {
		if (check_scanning_ctrl.mode == SCANMODE_SIX_LIGHTSOURCE)
			check_scanning_ctrl.hwmode = SCANNING_SIX_LIGHTSOURCE_MODE;
		else if (check_scanning_ctrl.mode == SCANMODE_TEN_LIGHTSOURCE)
			check_scanning_ctrl.hwmode = SCANNING_TEN_LIGHTSOURCE_MODE;

		if (cmdhead->datalength == 0)
			check_scanning_ctrl.lightsource.side_a = check_scanning_ctrl.lightsource.side_b = LIGHTSOURCE_ALL;
		else
			memcpy((void *)&check_scanning_ctrl.lightsource,data,sizeof(scanning_lightsource_t));
*/
		rs = 0;//spv_do_scanning(check_scanning_ctrl.hwmode, 0);
		if (rs == 0) {
			int datalen;
			unsigned char *bufptr = check_scanning_ctrl.buffer;
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
//			if (transmit_mode == IMGDATA_XMIT_AFTER_SCANNING)
			{
				datalen = 0x5c4900;//check_scanning_ctrl.linecount * check_scanning_ctrl.linedlen;
				reply_nodata.status = 0;//get_media_status();
				host_send_response(&reply_nodata, (unsigned char *)(0xa0000000), datalen);
				printf("sent scandata packet, total %d bytes of %d lines\n", datalen, check_scanning_ctrl.linecount);
				return 0;
			}
/*			else {
				int linecount = 0, bulk_lines;
				bulk_lines = XMIT_BLOCK_SIZE / check_scanning_ctrl.linedlen;
				do {
					int lines, datalen;
//					if (scanner_is_state(SCANNING_END) && (linecount >= check_scanning_ctrl.linecount))
//						break;	// no more scanned data: exit loop
//					if (scanner_wait_scandata() != 0)
//					{	// timeout while waiting for data
//						if (scanner_is_state(SCANNING_END))
//							break;
//					}
					lines = check_scanning_ctrl.linecount - linecount;
//					if (lines < bulk_lines && !scanner_is_state(SCANNING_END))
//						continue;

//					if (lines > bulk_lines && !scanner_is_state(SCANNING_END))
//						lines = bulk_lines;
					datalen = lines * check_scanning_ctrl.linedlen;
					host_send_response_multiple(&reply_nodata, bufptr, datalen);
					linecount += lines;
					bufptr += datalen;
//					dbg_printf("sent %4d lines of %4d, pid = %d\n", linecount, check_scanning_ctrl.linecount, reply_nodata.pid);
				} while (1);

//				rs = spv_do_scanning_end();
				if (rs != 0)
				{
					reply_nodata.status = -rs;
					reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
				}
				// send an empty response to terminate current command/response session
				reply_nodata.pflag = PACKET_FLAG_XMIT_END;
				host_send_response_multiple(&reply_nodata, NULL, 0);
				printf("sent ending packet, pid = %d. Total %d of %d lines\n", reply_nodata.pid, linecount, check_scanning_ctrl.linecount); 
				return 0;
			}  */
		}
		reply_nodata.status = -rs;
//		err_info_fill_by_code(reply_nodata.status);
//	}

	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_stop_scanning(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = get_media_status();
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_dummy_scanning(comm_packet_head_t *cmdhead, void *data)
{
	int rs, mode, count, maxlen;
	char pattern;

	mode = cmdhead->para;
	if (mode >= 0 && mode <= 0xff)	// mode = 0x00 - 0xff, send dummy scan data with pattern
		pattern = mode;
	resp_packet_init(&reply_nodata, cmdhead);
	
	err_info_init();
	reply_nodata.status = get_media_status();
	if (cmdhead->datalength == sizeof(int)) {
		char *ptr;
		ptr = data;

		maxlen = CHECK_MAX_SCAN_LINES * CHECK_SCANLINE_DATA_CNT * CHECK_MAX_SCAN_LIGHTSOURCES;
		count = *((int *)data);
		count = (count > maxlen) ? maxlen : count;
		if ((count < maxlen)&&(count > 0)) {
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
			memset(scanbuffer, pattern, count);
			host_send_response(&reply_nodata, scanbuffer, count);
			return 0;
		}
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR); 
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}
	else{
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_MISSING_DATA);
		reply_nodata.status = RESN_APP_COMMON_MISSING_DATA;
	}

	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}

int cmd_get_img_exposure_time(comm_packet_head_t *cmdhead, void *data)
{
	int rs, exposuretime = 0;
	scanning_exposure_time_setting_t *lightsource_set;

	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 

	lightsource_set = (scanning_exposure_time_setting_t *)&cmdhead->para;

	rs = scanner_get_exposure_time(lightsource_set, &exposuretime); 
	reply_nodata.datalength = 2;

	if (rs != 0){
		reply_nodata.status = -rs;
		err_info_fill_by_code(reply_nodata.status);
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
		host_send_response(&reply_nodata, &exposuretime, reply_nodata.datalength);
		return 0;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}

int cmd_set_img_exposure_time(comm_packet_head_t *cmdhead, void *data)
{
	int rs, exposuretime;
	scanning_exposure_time_setting_t *lightsource_set;

	lightsource_set = (scanning_exposure_time_setting_t *)&cmdhead->para;

	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;

	if(cmdhead->datalength > 0) {
		memcpy((void *)&exposuretime, data, cmdhead->datalength);
		exposuretime &= 0xffff;
		rs = scanner_set_exposure_time(lightsource_set, exposuretime); 
		if (rs != 0){
			reply_nodata.status = -rs;
			err_info_fill_by_code(reply_nodata.status);
		}
		else{
			reply_nodata.status = get_media_status();
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		}
	}
	else{
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR); 
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}

	host_send_response(&reply_nodata, NULL, 0);

	return 0;
}

int cmd_get_img_afe_gain(comm_packet_head_t *cmdhead, void *data)
{
	int rs, gain;

	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	err_info_init();

	rs = scanner_get_afe_gainmsb_config(&gain);
	reply_nodata.datalength = sizeof(gain);

	if (rs != 0){
		reply_nodata.status = RESN_APP_COMMON_SCANNING_ERROR;
		err_info_fill(RESN_CLASS_APP_COMMON,-RESN_APP_COMMON_SCANNING_ERROR);
	}
	else{
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
		host_send_response(&reply_nodata, &gain, reply_nodata.datalength);
		return 0;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}

int cmd_set_img_afe_gain(comm_packet_head_t *cmdhead, void *data)
{
	int rs, gain;

	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;

	err_info_init();
	if(cmdhead->datalength > 0) {
		memcpy((void *)&gain, data, cmdhead->datalength);
		rs = scanner_set_afe_gainmsb_config(gain);
		if (rs != 0){
			reply_nodata.status = RESN_APP_COMMON_SCANNING_ERROR;
			err_info_fill(RESN_CLASS_APP_COMMON,-RESN_APP_COMMON_SCANNING_ERROR);
		} else
		{
			reply_nodata.status = get_media_status();
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		}
	}
	else{
		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR);
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}

	host_send_response(&reply_nodata, NULL, 0);

	return 0;
}
