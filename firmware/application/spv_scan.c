#include <stdio.h>

#include <mechlib.h>
#include "spv_states.h"
#include "paperdrv.h"
#include "scanner.h"
#include "command.h"
#include "scandrv.h"
#include "scancalib.h"
#include "debug.h"
#include "media.h" 
#include "resp_voucher_scanner.h"
#include "statistics.h"
#include "FreeRTOS.h"
#include "task.h"
#include "actions.h"

#define SCANNING_CALIBRATE_MODE		SCANNING_TEN_LIGHTSOURCE_MODE

extern int quit;

extern volatile int spv_status;
extern int media_state;
extern int transmit_mode;
extern unsigned char *calibuf;

int last_spv_status;

static int check_scanning_path()
{
	int rs = 0 ;
	#if 0
	/* make sure that paperpath is clear. */
	if (paperpath_paper_is_at_sen0x() || paperpath_paper_is_at_sen1x() ||
	    paperpath_paper_is_at_sen2x()) 
	{
		log_err("check_scanning_path: paperpath path is not clear\n");
		return -RESN_APP_CUSTOM_SCAN_PATH_NOT_CLEAR;
	}
	/* make sure that acceptor path is clear. */
	if (acceptor_paper_is_inpath()) 
	{
		log_err("check_scanning_path: acceptor path is not clear\n");
		return -RESN_APP_CUSTOM_SCAN_PATH_NOT_CLEAR;
	}
	/* make sure that acceptor is reset. */
	if (!acceptor_pusher_is_reset()) {
		log_err("check_scanning_path: acceptor is NOT reset. NO action!\n");
		return -RESN_APP_CUSTOM_ACCEPTOR_NOT_RESET;
	}
	/* make sure that media is aligned. */
	if (!paperpath_paper_is_at_aligner())
	{
		log_err("check_scanning_path: media is not aligned\n");
		return -RESN_APP_CUSTOM_MEDIA_NOT_ALIGNED;
	}
	#endif
	return 0;
}


int spv_do_scanning(struct scanunit *scanner, int sidetype, scanning_control_t scan_ctrl, int timeout)
{                                                      
	int rs, sensor_st = 0;
        int time = 5;
	/* make sure scanning path is clear */
	/*rs = check_scanning_path();
	if (rs != 0)
		return rs;*/
        
	check_scanner_setup_ajustable_config(scan_ctrl.hwresolution, scan_ctrl.hwmode);
	
	if(sidetype == IAM_A_PAGE)
		scanner_set_cis_lightsource(scanner, scanner->hwinfo.sensor_a, scan_ctrl.lightsource.side_a);
	else if(sidetype == IAM_AB_PAGE)
	{
		scanner_set_cis_lightsource(scanner, scanner->hwinfo.sensor_a, scan_ctrl.lightsource.side_a);
		scanner_set_cis_lightsource(scanner, scanner->hwinfo.sensor_b, scan_ctrl.lightsource.side_b);
	}

	scanner_prepare_scanning(scanner, scan_ctrl.hwresolution, scan_ctrl.hwmode, MAX_SCAN_LINES, scan_ctrl.scanflag);

	if(scan_ctrl.scanflag & SCANNING_FLAG_LIGHTS_ON)
		scanunit_turnon_lights(scanner);
        check_scanner_setup_ajustable_config(scan_ctrl.hwresolution, scan_ctrl.hwmode);
	scanner_stop_scanning(scanner, sidetype);
	scanner_start_scanning(scanner, sidetype);
	rs = Actions_Move(0);
	Actions_SensorAllSt(&sensor_st);

	while((rs == !0) && !(sensor_st & SENSOR_ST_REAR)&&(timeout))
	{
		vTaskDelay(10/ portTICK_RATE_MS);
		time--;
	}
	
        scanner_stop_scanning(scanner, sidetype);  
	Actions_Move(1);
	return 0;
}


int spv_do_scan_calibration(struct scanunit *scanner, unsigned char *pdata, scanning_control_t scan_ctrl, int sidetype, int mode, int scanlines)
{
	int rs;

	if (scanlines > MAX_SCAN_LINES/2)
		return -RESN_APP_CUSTOM_SCANLINES_OVER_RANGE;
        
	check_scanner_setup_ajustable_config(scan_ctrl.hwresolution, scan_ctrl.hwmode);
	if(sidetype == IAM_A_PAGE)
		scanner_set_cis_lightsource(scanner, scanner->hwinfo.sensor_a, scan_ctrl.lightsource.side_a);
	else if(sidetype == IAM_AB_PAGE)
	{
		scanner_set_cis_lightsource(scanner, scanner->hwinfo.sensor_a, scan_ctrl.lightsource.side_a);
		scanner_set_cis_lightsource(scanner, scanner->hwinfo.sensor_b, scan_ctrl.lightsource.side_b);
	}
	/* scanning with all lights OFF to get darking image data. No media movement during scanning. */
	scanner_prepare_scanning(scanner, scan_ctrl.hwresolution, scan_ctrl.hwmode, scanlines, 0);
	scanner_stop_scanning(scanner, sidetype);
	scanunit_turnoff_lights(scanner);
	scanner_start_scanning(scanner, sidetype);
	vTaskDelay(800/ portTICK_RATE_MS);
	scanner_stop_scanning(scanner, sidetype);
	printf("%d lines of Darking image data scanned\n", scanlines);
        memcpy(pdata, (unsigned char *)(0xa0000fc0+100*CHECK_SCANLINE_SIZE), CHECK_SCANLINE_SIZE*scanlines);  

	//set lightsource	
	scanner_set_cis_lightsource(scanner, SIDE_A, scanning_ctrl.lightsource.side_a);

	/* scanning with all lights ON to get shading image data. Media should be moving during scanning. */
	scanner_prepare_scanning(scanner, scan_ctrl.hwresolution, scan_ctrl.hwmode, scanlines, SCANNING_FLAG_LIGHTS_ON);
	scanner_stop_scanning(scanner, sidetype);
	scanunit_turnon_lights(scanner);
	scanner_start_scanning(scanner, sidetype);
	Actions_Move(0);
	vTaskDelay(800/ portTICK_RATE_MS);
	scanner_stop_scanning(scanner, sidetype);
	Actions_Move(1);
	printf("%d lines of shanding image data scanned\n", scanlines);
	memcpy(pdata+CHECK_SCANLINE_SIZE*scanlines, (unsigned char *)(0xa0000fc0+100*CHECK_SCANLINE_SIZE), CHECK_SCANLINE_SIZE*scanlines);
	//if (mode == 1) {
	///	return scanner_create_calibratedata(scanbuffer, buffer2, buffer3, scanlines);
	//}
	return 0;
}


int spv_do_scanning_adjustment(void)
{
	int rs;

        motor1_run(0, 200);
	rs = check_scanner_calibrate_adjustable_paramters();
	motor1_run(1, 200);
	printf("spv_do_scanning_adjustment completed, spv_status=%d\n", spv_status);

	return rs;
}