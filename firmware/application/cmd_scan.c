#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <command.h>
#include "cmdprocs.h"
#include "response.h"
#include "paperdrv.h"
#include "scanner.h"
#include "scancalib.h"
#include "fpga.h"
#include "debug.h"
#include "spv_fun.h"
#include "respcode.h"
#include "resp_voucher_scanner.h"

extern unsigned short get_media_status(void);
extern const unsigned char dpitohwdpi[];


#define XMIT_BLOCK_SIZE		(2*1024*1024)

int transmit_mode = IMGDATA_XMIT_IN_SCANNING;
unsigned char calibuf[4*1024*1024];

int cmd_reset_scanunit(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = 0;//get_media_status();
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_set_scan_mode(comm_packet_head_t *cmdhead, void *data)
{
	int mode;
	resp_packet_init(&reply_nodata, cmdhead);
	mode = cmdhead->para;
	if (mode != IMGDATA_XMIT_IN_SCANNING && mode != IMGDATA_XMIT_AFTER_SCANNING) {
//		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_INVALID_PARAMETER); 
		reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
	}
	else {
		transmit_mode = mode;
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = 0;//get_media_status();
	}
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_get_scan_feature(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = 0;//get_media_status();
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_get_image_calib_data(comm_packet_head_t *cmdhead, void *data)
{
	int rs, size = 0;
	int initid =  cmdhead->para;

	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
//	err_info_init();
        scanning_ctrl.hwresolution = FPGA_REG_CIS_DPI_200;

	if(initid == CHECK_SCANUNIT)
	{
//		scanning_ctrl.hwresolution = dpitohwdpi[databuf[3]];
		rs = check_scanner_read_calidata(calibuf, &size, CHECK_SCANUNIT, scanning_ctrl.hwresolution);
		printf("rs=%d\r\n", rs);
		if (rs != 0){
//				err_info_fill(RESN_CLASS_APP_CUSTOM, -RESN_APP_CUSTOM_READ_CALI_DATA_ERR); 
			reply_nodata.status = RESN_APP_CUSTOM_READ_CALI_DATA_ERR;
		}
		else
		{
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
			reply_nodata.status = 0;//get_media_status();
			reply_nodata.datalength = size;
			host_send_response(&reply_nodata, calibuf, reply_nodata.datalength);
			return 0;
		}		
	}
	else
		reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_set_image_calib_data(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
	int initid =  cmdhead->para;	

	memcpy((void *)calibuf, data, cmdhead->datalength);
	//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.status = 0;//get_media_status();
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
        scanning_ctrl.hwresolution = FPGA_REG_CIS_DPI_200;
	if((initid == CHECK_SCANUNIT)&&(cmdhead->datalength > 0)){
		rs = check_scanner_write_calidata(calibuf, cmdhead->datalength, CHECK_SCANUNIT, scanning_ctrl.hwresolution); 
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
		reply_nodata.status = 0;//get_media_status();
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_scanning_calibration(comm_packet_head_t *cmdhead, void *data)
{
	int rs;

//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	if (cmdhead->datalength == 0 || cmdhead->datalength == sizeof(scanning_lightsource_t))
	{
		if (cmdhead->datalength == 0)
			scanning_ctrl.lightsource.side_a = scanning_ctrl.lightsource.side_b = LIGHTSOURCE_ALL;
		else if (cmdhead->datalength == sizeof(scanning_lightsource_t))
			memcpy((void *)&scanning_ctrl.lightsource,data,sizeof(scanning_lightsource_t));
		rs = spv_do_scan_calibration(&checkscanner, calibuf, scanning_ctrl, IAM_A_PAGE, 1, 100); 
		if (rs != 0){
			reply_nodata.status = -rs;
//			err_info_fill_by_code(reply_nodata.status);
		}
		else{
			reply_nodata.status = 0;//get_media_status();
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
	int rs;

//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	if (cmdhead->datalength == 0 || cmdhead->datalength == sizeof(scanning_lightsource_t))
	{
		if (cmdhead->datalength == 0)
			scanning_ctrl.lightsource.side_a = scanning_ctrl.lightsource.side_b = LIGHTSOURCE_ALL;
		else if (cmdhead->datalength == sizeof(scanning_lightsource_t))
			memcpy((void *)&scanning_ctrl.lightsource,data,sizeof(scanning_lightsource_t));
		rs = spv_do_scan_calibration(&checkscanner, calibuf, scanning_ctrl, IAM_A_PAGE, 0, 50); 
		if (rs == 0) {
			int datalen = CHECK_SCANLINE_SIZE*50*2;
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
			reply_nodata.status =0;
			reply_nodata.datalength = datalen;
			host_send_response(&reply_nodata, calibuf, datalen);
			return 0;
		}
		reply_nodata.status = -rs;
//		err_info_fill_by_code(reply_nodata.status);
	}
	else{
//		err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_INVALID_PARAMETER);
		reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
	}

	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_start_scanning(comm_packet_head_t *cmdhead, void *data)
{
	int rs, mode, sidetype = 0;
	scanning_setting_t *setting;
        unsigned char *databuf = (unsigned char *)data;

//	err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);

	setting = (scanning_setting_t *)&cmdhead->para;
	scanning_ctrl.mode = setting->mode;
	scanning_ctrl.resolution = setting->resolution;
	if(databuf[2] == CHECK_SCANUNIT)
	{
                if((scanning_ctrl.mode & 0x1) == 1)
			sidetype = IAM_AB_PAGE;
		else
			sidetype = IAM_A_PAGE;

		if (!((scanning_ctrl.mode == SCANMODE_SINGLE_COLOR_IR_UV || scanning_ctrl.mode == SCANMODE_SINGLE_COLOR_IR)
		    && (scanning_ctrl.resolution == DPI_200)))
		{
//			err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_INVALID_PARAMETER); 
			reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;
		}
		else 
		{
			if (scanning_ctrl.mode == SCANMODE_SINGLE_COLOR_IR_UV)
				scanning_ctrl.hwmode = FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV;
			else if (scanning_ctrl.mode == SCANMODE_SINGLE_COLOR_IR)
				scanning_ctrl.hwmode = FPGA_REG_CIS_SCANMODE_EN_RGB_IR;

			if (cmdhead->datalength == 0)
				scanning_ctrl.lightsource.side_a = scanning_ctrl.lightsource.side_b = LIGHTSOURCE_ALL;
			else
				memcpy((void *)&scanning_ctrl.lightsource, data, sizeof(scanning_lightsource_t));
                        scanning_ctrl.scanflag = SCANNING_FLAG_LIGHTS_ON;
			rs = spv_do_scanning(&checkscanner, sidetype, scanning_ctrl, 0);
			if (rs == 0) {
				int datalen;
				unsigned int scanlines = 0;
				reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
				scanlines = scanner_get_wraddr(&checkscanner)/CHECK_SCANLINE_SIZE;
                                datalen = scanlines*CHECK_SCANLINE_SIZE;
				//if(datalen > 0x90b400)
					datalen = 0x90b400;//0x90b400,784 lines
				reply_nodata.status = 0;//get_media_status();
				reply_nodata.datalength = datalen;
				host_send_response(&reply_nodata, (unsigned char *)(0xa0000000+0xfc0), datalen);
				printf("sent scandata packet, total %d bytes of %d lines\n", datalen, scanning_ctrl.linecount);
				return 0;
			}
			reply_nodata.status = -rs;
	//		err_info_fill_by_code(reply_nodata.status);
		}
	}else
		reply_nodata.status = RESN_APP_COMMON_INVALID_PARAMETER;

	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 
	host_send_response(&reply_nodata, NULL, 0);
	return 0;
}


int cmd_stop_scanning(comm_packet_head_t *cmdhead, void *data)
{
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
	reply_nodata.status = 0;//get_media_status();
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
	
	//err_info_init();
	reply_nodata.status = 0;//get_media_status();
	if (cmdhead->datalength == sizeof(int)) {
		char *ptr;
		ptr = data;

		maxlen = MAX_SCAN_LINES * CHECK_SCANLINE_DATA_CNT * CHECK_MAX_SCAN_LIGHTSOURCES;
		count = *((int *)data);
		count = (count > maxlen) ? maxlen : count;
		if ((count < maxlen)&&(count > 0)) {
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
			memset(calibuf, pattern, count);
			host_send_response(&reply_nodata, calibuf, count);
			return 0;
		}
		//err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR); 
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}
	else{
		//err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_MISSING_DATA);
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

	//err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR; 

	lightsource_set = (scanning_exposure_time_setting_t *)&cmdhead->para;

	rs = check_scanner_get_exposure_time(lightsource_set, &exposuretime); 
	reply_nodata.datalength = 2;

	if (rs != 0){
		reply_nodata.status = -rs;
		//err_info_fill_by_code(reply_nodata.status);
	}
	else
	{
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = 0;//get_media_status();
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

	//err_info_init();
	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;

	if(cmdhead->datalength > 0) {
		memcpy((void *)&exposuretime, data, cmdhead->datalength);
		exposuretime &= 0xffff;
		rs = check_scanner_set_exposure_time(lightsource_set, exposuretime); 
		if (rs != 0){
			reply_nodata.status = -rs;
			//err_info_fill_by_code(reply_nodata.status);
		}
		else{
			reply_nodata.status = 0;//get_media_status();
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		}
	}
	else{
		//err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR); 
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}

	host_send_response(&reply_nodata, NULL, 0);

	return 0;
}

int cmd_get_img_afe_gain(comm_packet_head_t *cmdhead, void *data)
{
	int rs;
	unsigned int gain;

	resp_packet_init(&reply_nodata, cmdhead);
	reply_nodata.ptype = PACKET_RESPONSE_ERROR;
	//err_info_init();

	rs = scanner_get_afe_gainmsb_config(&checkscanner, 0, &gain);
	reply_nodata.datalength = sizeof(gain);

	if (rs != 0){
		reply_nodata.status = RESN_APP_COMMON_SCANNING_ERROR;
		//err_info_fill(RESN_CLASS_APP_COMMON,-RESN_APP_COMMON_SCANNING_ERROR);
	}
	else{
		reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		reply_nodata.status = 0;//get_media_status();
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

	//err_info_init();
	if(cmdhead->datalength > 0) {
		memcpy((void *)&gain, data, cmdhead->datalength);
		rs = scanner_set_afe_gainmsb_config(&checkscanner, 0, gain);
		if (rs != 0){
			reply_nodata.status = RESN_APP_COMMON_SCANNING_ERROR;
			//err_info_fill(RESN_CLASS_APP_COMMON,-RESN_APP_COMMON_SCANNING_ERROR);
		} else
		{
			reply_nodata.status = 0;//get_media_status();
			reply_nodata.ptype = PACKET_RESPONSE_SUCCESS;
		}
	}
	else{
		//err_info_fill(RESN_CLASS_APP_COMMON, -RESN_APP_COMMON_DATA_ERROR);
		reply_nodata.status = RESN_APP_COMMON_DATA_ERROR;
	}

	host_send_response(&reply_nodata, NULL, 0);

	return 0;
}
