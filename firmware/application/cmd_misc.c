#include <stdio.h>
#include <string.h>


#include <command.h>
#include "cmdprocs.h"
#include "config.h"
#include "response.h"
#include "paperdrv.h"
#include "spv_fun.h"
#include "error.h"
#include "actions.h"

extern unsigned short get_media_status(void);

int cmd_reset(comm_packet_head_t *cmdhead, void *data)
{
	para_reset_t *reset;

	resp_packet_init(&reply_nodata, cmdhead);

	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reset = (para_reset_t *)&cmdhead->para;
	Actions_Move(1);

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_calibrate_sensor(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
	resp_packet_init(&reply_nodata, cmdhead);
//	err_info_init();

	rs =0;// spv_do_sensor_calibration(cmdhead->para);
	if (rs == 0) {
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
		reply_nodata.status = -rs;
//		err_info_fill_by_code(reply_nodata.status);
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_open_aligner(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	rs = 0;//aligner_open_close(1);
	if (rs == 0) {
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
		reply_nodata.status = -rs;
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_close_aligner(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	rs = 0;//aligner_open_close(0);
	if (rs == 0) {
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
		reply_nodata.status = -rs;
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_reset_acceptor(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
	resp_packet_init(&reply_nodata, cmdhead);
//	err_info_init();

	rs = 0;//spv_do_accept_media();
	if (rs == 0) {
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_ERROR;
		reply_nodata.status = -rs;
//		err_info_fill_by_code(reply_nodata.status);
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_accept_media(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
	resp_packet_init(&reply_nodata, cmdhead);
//	err_info_init();

	rs = 0;//spv_do_accept_media();
	if (rs == 0) {
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = get_media_status();
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
		reply_nodata.status = -rs;
//		err_info_fill_by_code(reply_nodata.status);
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


