#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <information.h>
#include <command.h>
#include "cmdprocs.h"
#include "response.h"
#include "respcode.h"
#include "fpgadrv.h"
#include "paperdrv.h"
#include "mechdrv.h"
#include "version.h"
#include "misc.h"
#include "media.h"
#include "statistics.h"

#define DATA_BUFFER_LEN (1024*1024)
comm_packet_head_t reply_nodata;

unsigned char testbuff[1024*128];
extern code_discriptions_t statistics_code_discriptions;

unsigned short get_media_status(void)
{
	extern int media_state;
	unsigned short rs;

	switch (media_state) {
	case MEDIA_UNAVAILABLE:
		rs = RES_NO_MEDIA;
		break;
	case MEDIA_AT_FRONT:
		rs = RES_MEDIA_AT_FRONT;
		break;
	case MEDIA_ALIGNED:
		rs = RES_MEDIA_ALIGNED;
		break;
	case MEDIA_JAM:
		rs = RES_MEDIA_JAM;
		break;
	case MEDIA_LOADED:
	case MEDIA_IN_ACCEPTOR:
		rs = RES_MEDIA_LOADED;
		break;
	case MEDIA_ACCEPTED:
		rs = RES_MEDIA_ACCEPTED;
		break;
	}
	return rs;
}


int cmd_nop(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = get_media_status();
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_undefined(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	err_info_init();
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	reply_nodata.status = RESN_APP_COMMON_UNDEFINED_COMMAND;
	err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_UNDEFINED_COMMAND);
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_get_device_info(comm_packet_head_t *cmdhead, void *data)
{
	extern device_info_t deviceinfo;

	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = get_media_status();
	reply_nodata.datalength = sizeof(device_info_t);
	host_send_response(&reply_nodata, &deviceinfo, reply_nodata.datalength); 
	return 0;
}


int cmd_get_device_status(comm_packet_head_t *cmdhead, void *data)
{
	int sen0,sen1,sen2;
	device_status_t device_status;

	resp_packet_init(&reply_nodata, cmdhead);

	memset(&device_status,0,sizeof(device_status_t));

	if (paperpath_is_coveropen())
		device_status.cover_open[0] = '1';
	else
		device_status.cover_open[0] = '0';

	if (acceptor_is_coveropen())
		device_status.cover_open[1] = '1';
	else
		device_status.cover_open[1] = '0';

	if (acceptor_pusher_is_reset())
		device_status.mediabox[0] = (acceptor_is_full())? '2':'1';
	else
		device_status.mediabox[0] = (acceptor_is_full())? '6':'5';

	if (paparpath_aligner_is_open())
		device_status.aligner = '1';
	else
		device_status.aligner = '0';

	sen0 = paperpath_paper_is_at_sen0x();
	sen1 = paperpath_paper_is_at_sen1x();
	sen2 = paperpath_paper_is_at_sen2x();

	if ((sen0 && (!sen1) && sen2) ||((!sen0) && sen1 && (!sen2)))
		device_status.scanunit = DEVSTATUS_PART_ERROR;
	else if (sen0 || sen1 || sen2) 
		device_status.scanunit = '1';
	else
		device_status.scanunit = '0';

	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = get_media_status();
	reply_nodata.datalength = sizeof(device_status_t);
	host_send_response(&reply_nodata, &device_status, reply_nodata.datalength);
	return 0;
}


int cmd_get_sensor_status(comm_packet_head_t *cmdhead, void *data)
{
	int mode,length;
	mech_sen_logic_input_t *mech_sen_logic_input;
	mech_sen_raw_input_t *mech_sen_raw_input;
	void *sensor_status;

	mode = cmdhead->para;
	resp_packet_init(&reply_nodata, cmdhead);

	if (mode == 0 || mode == 1) {
		if (mode == 0) {
			mech_sen_logic_input = sensor_get_input_logic();
			length = mech_sen_logic_input->mech_part_num*sizeof(unsigned char);
			sensor_status = (void *)mech_sen_logic_input->p_logic_input_value;
		}
		else if (mode == 1) {
			mech_sen_raw_input = sensor_get_input_raw();
			length = mech_sen_raw_input->mech_part_num*sizeof(unsigned long);
			sensor_status = (void *)mech_sen_raw_input->p_raw_input_value;
		}
		reply_nodata.status = get_media_status();
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;;
		reply_nodata.datalength = length;
		host_send_response(&reply_nodata,sensor_status,reply_nodata.datalength);
	}
	else{
	reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
		reply_nodata.ptype = PACKET_RESPONSE_ERROR;
		reply_nodata.datalength = 0;
		host_send_response(&reply_nodata, NULL, 0);
	}

	return 0;
}

