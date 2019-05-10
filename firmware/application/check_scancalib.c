#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <scanunit.h>

#include "imagelib.h"
#include "scanner.h"
#include "scandrv.h"
#include "scancalib.h"
#include "fpga.h"
#include "FreeRTOS.h"
#include "nvmapi.h"
#include "nvmdata.h"
#include "command.h"
#include "debug.h"
#include "task.h"


//cis light calibrate
#define MAX_ONTIME_VI_IR	0x1ca
#define MAX_ONTIME_UV		0x1ca
#define MIN_ONTIME_ALL		0

#define MIN_PIXELS_NUM		10
#define MAX_PIXELS_NUM		100
#define MAX_PIXELS_VAL		229
#define MIN_PIXELS_VAL		220
#define MAX_LIGHT_VAL		229

#define MAX_CALIBRATE_SCANLINES	50
#define CIS_UP_START		100
#define CIS_DOWM_START		(CHECK_SCANLINE_PIXELS-CIS_UP_START)
#define INACTIVE_PIXELS_NUM	0x510

#define R_ONTIME_MASK		0x40
#define LIGHT_CALIBRATE_NUM	7

#define MAX_CALI_CNT		20
#define CIS_BAR1_START		0
#define CIS_BAR1_WIDTH		20
#define CIS_BAR2_START		CHECK_SCANLINE_PIXELS-20
#define CIS_BAR2_WIDTH		20

//adc offset calibrate
#define MAX_AVER_VAL		50
#define MIN_AVER_VAL		5
#define MIN_DISTR_NUM		1
#define MAX_DISTR_NUM		50
#define MAX_OFFSET_VAL		0xff
#define MIN_OFFSET_VAL		0x80
#define BOUNDARY_DISTR		2
#define CIS_PIXELS_200DPI	1008
#define CALI_CHECKSUM_SIZE	2
#define CALI_EXTRA_SIZE		6+CALI_CHECKSUM_SIZE

extern struct scanunit checkscanner;
extern struct scancalib_data_buf buf;
extern int scanmode;

const uint8_t dpi_200_adc_default_a[] = {0x80};
const uint8_t dpi_200_adc_default_b[] = {0x80};
uint8_t dpi_200_adc_current_a[] = {0x80};
uint8_t dpi_200_adc_current_b[] = {0x80};
//				       	R,   	 G,    	 B,  	 ir,	irt,	   uv,      uvt,   uvp,   uvpt  
const uint16_t dpi_200_cis_default_a[] = {0x0076, 0x0076, 0x0076, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030};
const uint16_t dpi_200_cis_default_b[] = {0x0076, 0x0076, 0x0076, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030};
uint16_t dpi_200_cis_current_a[] =	{0x0076, 0x0076, 0x0076, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030};
uint16_t dpi_200_cis_current_b[] = 	{0x0076, 0x0076, 0x0076, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030};

unsigned char check_config[256] = {0};
int check_distr[256] = {0};

DEFINE_NVM_DATA(SCAN_DPI_200_ADC_A, sizeof(dpi_200_adc_current_a), dpi_200_adc_current_a);
DEFINE_NVM_DATA(SCAN_DPI_200_ADC_B, sizeof(dpi_200_adc_current_b), dpi_200_adc_current_b);
DEFINE_NVM_DATA(SCAN_DPI_200_CIS_A, sizeof(dpi_200_cis_current_a), dpi_200_cis_current_a);
DEFINE_NVM_DATA(SCAN_DPI_200_CIS_B, sizeof(dpi_200_cis_current_b), dpi_200_cis_current_b);
DEFINE_NVM_DATA(SCAN_DPI_200_CALIBDATA_COLOR_DARK_A, CIS_PIXELS_200DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_200_CALIBDATA_COLOR_DARK_B, CIS_PIXELS_200DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_200_CALIBDATA_COLOR_A, CIS_PIXELS_200DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_200_CALIBDATA_COLOR_B, CIS_PIXELS_200DPI*3+CALI_EXTRA_SIZE, NULL);

enum ten_scan_index_order {
	TEN_LIGHT_VI_A,
	TEN_LIGHT_VI_B,
	TEN_LIGHT_IR_A,
	TEN_LIGHT_IR_B,
	TEN_LIGHT_IRT_A,
	TEN_LIGHT_IRT_B,
	TEN_LIGHT_UV_A,
	TEN_LIGHT_UV_B,
	TEN_LIGHT_UVT_A,
	TEN_LIGHT_UVT_B,
};

enum six_scan_index_order {
	SIX_LIGHT_VI_A,
	SIX_LIGHT_VI_B,
	SIX_LIGHT_IR_A,
	SIX_LIGHT_IRT_A,
	SIX_LIGHT_UV_A,
	SIX_LIGHT_UVT_A,
};

enum light_index_order {
	LIGHT_VI_RED,
	LIGHT_VI_GREEN,
	LIGHT_VI_BLUE,
	LIGHT_VI_IR,
	LIGHT_VI_IRTR,
	LIGHT_VI_UV,
	LIGHT_VI_UVTR,
};

enum{
	WhiteA = 0xa0000000,
	IrA = 0xa0400000,
	IrtA = 0xa0500000,
	UvA = 0xa0400000,
	UvtA = 0xa0500000,

	WhiteB = 0xa0000000,
	IrB = 0xa0400000,
	IrtB = 0xa0500000,
	UvB = 0xa0400000,
	UvtB = 0xa0500000,
};



struct scan_reg_config check_cis_light_config[] = {
	{0x0040, 0x00, 0xffff},		//LED_Red_On_Time
	{0x0044, 0x00, 0xffff},		//LED_Green_On_Time
	{0x0048, 0x00, 0xffff},		//LED_Blue_On_Time
	{0x004C, 0x00, 0xffff},		//LED_IR_On_Time
	{0x0050, 0x00, 0xffff},		//LED_IR_T_On_Time
	{0x0054, 0x00, 0xffff},		//LED_UV_On_Time
	{0x0058, 0x00, 0xffff}, 	//LED_UV_T_On_Time
	{0x005c, 0x00, 0xffff},		//LED_UVP_On_Time
};
const unsigned char check_light_name[10][9] = {"red", "green", "blue", "ir", "irt", "uv", "uvt", "uvP", "uvPt"};
const unsigned char check_lightsel[7] = {
	LIGHTSOURCE_RED,
	LIGHTSOURCE_GREEN,
	LIGHTSOURCE_BLUE,
	LIGHTSOURCE_IR,
	LIGHTSOURCE_IRT,
	LIGHTSOURCE_UV,
	LIGHTSOURCE_UVT,
};

const struct scanner_adjustable_parameters_config check_adjust_para_t[] = {
	{CONFIG_ADC, CHECK_SIDE_A, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_SCANMODE_TEN_LIGHTS},	//200DPI TENMODE ADC OFFSET ADJUST
	{CONFIG_ADC, CHECK_SIDE_B, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_SCANMODE_TEN_LIGHTS},	//200DPI TENMODE ADC OFFSET ADJUST
	{CONFIG_CIS, CHECK_SIDE_A, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_SCANMODE_TEN_LIGHTS},	//200DPI TENMODE CIS ONTIME ADJUST
	{CONFIG_CIS, CHECK_SIDE_B, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_SCANMODE_TEN_LIGHTS},	//200DPI TENMODE CIS ONTIME ADJUST
};
#define ADJUSTABLE_CONFIG_NUM	(sizeof(check_adjust_para_t)/sizeof(struct scanner_adjustable_parameters_config))

scancalib_lightaddr_t check_configlightside_to_lightaddr_table[] = {
	{(CHECK_SIDE_A<<8)|LIGHT_VI_RED,	WhiteA},
	{(CHECK_SIDE_A<<8)|LIGHT_VI_GREEN,	WhiteA},
	{(CHECK_SIDE_A<<8)|LIGHT_VI_BLUE,	WhiteA},
	{(CHECK_SIDE_A<<8)|LIGHT_VI_IR,		IrA},
	{(CHECK_SIDE_A<<8)|LIGHT_VI_IRTR, 	IrtA},
	{(CHECK_SIDE_A<<8)|LIGHT_VI_UV,		UvA},
	{(CHECK_SIDE_A<<8)|LIGHT_VI_UVTR, 	UvtA},	
	{(CHECK_SIDE_B<<8)|LIGHT_VI_RED,  	WhiteB},
	{(CHECK_SIDE_B<<8)|LIGHT_VI_GREEN,	WhiteB},
	{(CHECK_SIDE_B<<8)|LIGHT_VI_BLUE,	WhiteB},
	{(CHECK_SIDE_B<<8)|LIGHT_VI_IR,		IrB},
	{(CHECK_SIDE_B<<8)|LIGHT_VI_IRTR,	IrtB},
	{(CHECK_SIDE_B<<8)|LIGHT_VI_UV, 	UvB},
	{(CHECK_SIDE_B<<8)|LIGHT_VI_UVTR, 	UvtB},
};
/*
lightsource = 0 -- R
lightsource = 1 -- g
lightsource = 2 -- b
lightsource = 3 -- ir
lightsource = 4 -- irtr
lightsource = 5 -- uv
lightsource = 6 -- uvtr
*/

static int max_ontime_get(int lightsource)
{
	int max_ontime;	
	if((lightsource > 6)||(lightsource < 0))
	{
		return 0;
	}
	else if(lightsource < 5)
		max_ontime = MAX_ONTIME_VI_IR;
	else
		max_ontime = MAX_ONTIME_UV;
	return max_ontime;
}

void check_scanner_reset_cisled_config(unsigned int flag)
{
	int i;
	if (flag == 0) {
		//set cis a config
		for(i = 0; i < CHECK_LIGHT_SOURCE_NUM; i++)
		{
			check_cis_light_config[i].value = 0;
		}
		set_cis_ontime(&checkscanner, checkscanner.hwinfo.sensor_a, check_cis_light_config, CHECK_LIGHT_SOURCE_NUM);
		//set cis b config
		for(i = 0; i < CHECK_LIGHT_SOURCE_NUM; i++)
		{
			check_cis_light_config[i].value = 0;
		}
		set_cis_ontime(&checkscanner, checkscanner.hwinfo.sensor_b, check_cis_light_config, CHECK_LIGHT_SOURCE_NUM);
	}else if(flag == 1){
		//set cis a config
		for(i = 0; i < CHECK_LIGHT_SOURCE_NUM; i++)
		{
			check_cis_light_config[i].value = dpi_200_cis_current_a[i];
		}
		set_cis_ontime(&checkscanner, checkscanner.hwinfo.sensor_a, check_cis_light_config, CHECK_LIGHT_SOURCE_NUM);
		//set cis b config
		for(i = 0; i < CHECK_LIGHT_SOURCE_NUM; i++)
		{
			check_cis_light_config[i].value = dpi_200_cis_current_b[i];
		}
		set_cis_ontime(&checkscanner, checkscanner.hwinfo.sensor_b, check_cis_light_config, CHECK_LIGHT_SOURCE_NUM);
	}
}


int check_scanner_setup_ajustable_config(int hwresolution, int hwmode)
{
	int i, rs;
	struct scanunit_config config;

	rs = check_scanner_load_ajustable_config(hwresolution, hwmode);
	if (rs != 0) {
		dbg_printf("check_scanner_load_ajustable_config fail, rs=%d\r\n", rs);
		memcpy(dpi_200_adc_current_a, dpi_200_adc_default_a, sizeof(dpi_200_adc_default_a));
		memcpy(dpi_200_adc_current_b, dpi_200_adc_default_b, sizeof(dpi_200_adc_default_b));
		memcpy(dpi_200_cis_current_a, dpi_200_cis_default_a, sizeof(dpi_200_cis_default_a));
		memcpy(dpi_200_cis_current_b, dpi_200_cis_default_b, sizeof(dpi_200_cis_default_b));
	}

        //set afe config
	scanner_set_afe_offset(&checkscanner, checkscanner.hwinfo.sectinfo_a[0].digitiser_id, dpi_200_adc_current_a[0]);
	scanner_set_afe_offset(&checkscanner, checkscanner.hwinfo.sectinfo_b[0].digitiser_id, dpi_200_adc_current_b[0]);

	//set cis a config
	config.regcount = CHECK_LIGHT_SOURCE_NUM;
	for(i = 0; i < config.regcount; i++)
	{
		check_cis_light_config[i].value = dpi_200_cis_current_a[i];
	}

	config.regconfig = (struct scan_reg_config *)check_cis_light_config;
	scanunit_set_sensor_config(&checkscanner, checkscanner.hwinfo.sensor_a, &config);

	//set cis b config
	config.regcount = CHECK_LIGHT_SOURCE_NUM;
	for(i = 0; i < config.regcount; i++)
	{
		check_cis_light_config[i].value = dpi_200_cis_current_b[i];
	}

	config.regconfig = (struct scan_reg_config *)check_cis_light_config;
	scanunit_set_sensor_config(&checkscanner, checkscanner.hwinfo.sensor_b, &config);
	return rs;
}


int check_scanner_load_ajustable_config(int hwresolution, int hwmode)
{
	int rs;

	check_paraconfig.config.mode = hwmode;
	check_paraconfig.config.hwresolution = hwresolution;
	dbg_printf("scanner_load_ajustable_config: hwresolution=%x, scanmode=%x\r\n", hwresolution, hwmode);

	check_paraconfig.config.config_tpye = CONFIG_ADC;
	check_paraconfig.config.sideid = CHECK_SIDE_A;
	rs = scanner_read_config_from_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_adc_default_a));
	if (rs == -1) {
		dbg_printf("read afe offset check_config from nvm fail!\r\n");
		return rs;
	}
	memcpy(dpi_200_adc_current_a, check_config, sizeof(dpi_200_adc_current_a));

	check_paraconfig.config.config_tpye = CONFIG_ADC;
	check_paraconfig.config.sideid= CHECK_SIDE_B;
	rs = scanner_read_config_from_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_adc_default_b));
	if (rs == -1) {
		dbg_printf("read afe offset check_config from nvm fail!\r\n");
		return rs;
	}
	memcpy(dpi_200_adc_current_b, check_config, sizeof(dpi_200_adc_current_b));

	check_paraconfig.config.config_tpye = CONFIG_CIS;
	check_paraconfig.config.sideid= CHECK_SIDE_A;
	rs = scanner_read_config_from_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_cis_current_a));
	if (rs == -1) {
		dbg_printf("read cis check_config from nvm fail!\r\n");
		return rs;
	}
	memcpy(dpi_200_cis_current_a, check_config, sizeof(dpi_200_cis_current_a));

	check_paraconfig.config.config_tpye = CONFIG_CIS;
	check_paraconfig.config.sideid = CHECK_SIDE_B;
	rs = scanner_read_config_from_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_cis_current_b));
	if (rs == -1) {
		dbg_printf("read cis check_config from nvm fail!\r\n");
		return rs;
	}
	memcpy(dpi_200_cis_current_b, check_config, sizeof(dpi_200_cis_current_b));

	return 0;
}


int check_scanner_save_ajustable_config(int hwresolution, int hwmode, int para_type, int side)
{
	int rs;

	check_paraconfig.config.mode = hwmode;
	check_paraconfig.config.hwresolution = hwresolution;
	check_paraconfig.config.config_tpye = para_type;
	check_paraconfig.config.sideid = side;

	if (para_type == CONFIG_ADC) {
		if(side == CHECK_SIDE_A)
		{
			memcpy(check_config, dpi_200_adc_current_a, sizeof(dpi_200_adc_current_a));
			rs = scanner_write_config_to_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_adc_current_a));
			if (rs == -1)
				return rs;
		}
		else
		{
			memcpy(check_config, dpi_200_adc_current_b, sizeof(dpi_200_adc_current_b));
			rs = scanner_write_config_to_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_adc_current_b));
			if (rs == -1)
				return rs;
		}

	}else if(para_type == CONFIG_CIS){
		if(side == CHECK_SIDE_A)
		{
			memcpy(check_config, dpi_200_cis_current_a, sizeof(dpi_200_cis_current_a));
			rs = scanner_write_config_to_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_cis_current_a));
			if (rs == -1) {
				return rs;
			}
		}
		else
		{
			memcpy(check_config, dpi_200_cis_current_b, sizeof(dpi_200_cis_current_b));
			rs = scanner_write_config_to_nvm(check_paraconfig.para, check_config, sizeof(dpi_200_cis_current_b));
			if (rs == -1) {
				return rs;
			}
		}
	}else
		return -1;

	return 0;
}

void check_scanner_set_all_led_ontime(int side)
{
	struct scanunit_config config;
	int i;
	int cisid = (side == checkscanner.hwinfo.sensor_a)? checkscanner.hwinfo.sensor_a:checkscanner.hwinfo.sensor_b;

	config.regcount = CHECK_LIGHT_SOURCE_NUM;

	if(side == checkscanner.hwinfo.sensor_a)
	{
		for(i = 0; i < config.regcount; i++)
		{
			check_cis_light_config[i].value =  dpi_200_cis_default_a[i];
		}
		config.regconfig = (struct scan_reg_config *)check_cis_light_config;
	}
	else if(side == checkscanner.hwinfo.sensor_b)
	{
		for(i = 0; i < config.regcount; i++)
		{
			check_cis_light_config[i].value =  dpi_200_cis_default_b[i];
		}
		config.regconfig = (struct scan_reg_config *)check_cis_light_config;
	}

	scanunit_set_sensor_config(&checkscanner, cisid, &config);
}

void check_scanner_set_led_ontime(int side, unsigned int value, int lightsource)
{
	struct scanunit_config config;
	struct scan_reg_config cis_ontime_config[] = {0x0000, 0x00, 0xffff};
	int cisid = (side == checkscanner.hwinfo.sensor_a)? checkscanner.hwinfo.sensor_a:checkscanner.hwinfo.sensor_b;

	cis_ontime_config[0].address = R_ONTIME_MASK + lightsource*4;
	cis_ontime_config[0].value = value;
	config.regcount = 1;
	config.regconfig = (struct scan_reg_config *)cis_ontime_config;
	scanunit_set_sensor_config(&checkscanner, cisid, &config);
}

unsigned int check_get_scan_addr(int para)
{
	int i;
	for (i = 0; i < sizeof(check_configlightside_to_lightaddr_table)/sizeof(scancalib_lightaddr_t); i++) {
		if (check_configlightside_to_lightaddr_table[i].para == para) {
			return check_configlightside_to_lightaddr_table[i].lightaddr;
		}
	}
	return 0;
}


int check_get_current_brightness_distr(int hwresolution, int scanmode, int side, int lightsource, int *red_distr, int *green_distr, int *blue_distr, int flags)
{
	int scanlines, active_cnt, active_linepixel = 0;
	unsigned int lightaddr = 0; 

	//scan image data
	lightaddr = check_get_scan_addr((side<<8)|lightsource);
	memset((void *)buf.calibuf, 0, buf.calilen);


	check_scanner_set_sensor_config(hwresolution, scanmode, side);

	if(flags == SCANNING_FLAG_LIGHTS_ON)
		scanunit_turnon_lights(&checkscanner);
	else
	    	scanunit_turnoff_lights(&checkscanner);

	scanunit_stop_scanning(&checkscanner);
	scanunit_start_scanning(&checkscanner);
	scanlines = 0;

//	while (scanlines < MAX_CALIBRATE_SCANLINES){
//		scanlines = scanunit_get_scanlines(side);
//	}
	vTaskDelay(300/ portTICK_RATE_MS);
	scanunit_stop_scanning(&checkscanner);

	if(lightaddr != 0)
	{
		img_rawdata_section((unsigned char*)lightaddr, buf.sectionbuf, MAX_CALIBRATE_SCANLINES*CHECK_SCANLINE_DATA_CNT, CHECK_SCANLINE_DATA_CNT, CIS_UP_START*4, CIS_DOWM_START*4);
		active_linepixel = CIS_DOWM_START - CIS_UP_START;
	}else
		active_linepixel = CHECK_SCANLINE_PIXELS;
	active_cnt = MAX_CALIBRATE_SCANLINES;

	memcpy(buf.buffer, buf.sectionbuf, active_cnt*active_linepixel*4);
	img_rawdata_color_distribution((unsigned char *)buf.buffer, active_cnt*active_linepixel, red_distr, \
  			green_distr, blue_distr);
	return 0;
}


int check_scanner_adjust_cis_led_brightness(int hwresolution, unsigned char scanmode, int cisid, unsigned int light_sel)
{
	int i, lights_cnt = LIGHT_CALIBRATE_NUM;
	struct scanunit_config config;
	int min_ontime, max_ontime, cur_ontime;
	int checkcnt, errrs;
	int max_distr[3];
	int ontime_raw[3] = {0};
	int distr_red[256] = {0}, distr_green[256] = {0}, distr_blue[256] = {0};
	unsigned int best_ontime[LIGHT_CALIBRATE_NUM] = {0};
	unsigned int light_index[LIGHT_CALIBRATE_NUM] = {0};
	bool rgb_step2_flag = false;

	if (scanmode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS) {
		if(cisid == checkscanner.hwinfo.sensor_b)
			light_sel &= 0x7;
	}


	//reset other side ontime 
	if(cisid == checkscanner.hwinfo.sensor_a) 
		scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_b, LIGHTSOURCE_IRT|LIGHTSOURCE_UVT);
	else if(cisid == checkscanner.hwinfo.sensor_b)
		scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_a, LIGHTSOURCE_IRT|LIGHTSOURCE_UVT);


	check_scanner_setup_ajustable_config(hwresolution, scanmode);

	errrs = 0;

	for (i = LIGHT_CALIBRATE_NUM; i > 0; i--)
	{
		if(light_sel & (1 << (i-1)))
		{
			light_index[i-1] = 1;
			lights_cnt++;
		}
	}

	if (light_index[0] && light_index[1] && light_index[2]) {
		rgb_step2_flag = true;
	}

	for (i = 0; i < lights_cnt; i++) 
	{
		int max_red_distr, max_green_distr, max_blue_distr;

		if (light_index[i] == 1) 
		{
			int actual_cisid;
			//enable currrent lightsource

			if((i == LIGHT_VI_UVTR) || (i == LIGHT_VI_IRTR)) 
				actual_cisid = (cisid == checkscanner.hwinfo.sensor_a)? checkscanner.hwinfo.sensor_b : checkscanner.hwinfo.sensor_a;
			else
				actual_cisid = cisid;
			scanner_set_cis_lightsource(&checkscanner, actual_cisid, check_lightsel[i]);

			config.regcount = 1;
			max_ontime = max_ontime_get(i);
			min_ontime = MIN_ONTIME_ALL;

			checkcnt = 0;
			while(best_ontime[i] == 0)
			{
				int max_light_distr, actual_cisid;
				if((i == LIGHT_VI_UVTR) || (i == LIGHT_VI_IRTR)) 
					actual_cisid = (cisid == checkscanner.hwinfo.sensor_a)? checkscanner.hwinfo.sensor_b : checkscanner.hwinfo.sensor_a;
				else
					actual_cisid = cisid;


				//check the MAX boundary
				check_scanner_set_led_ontime(actual_cisid, max_ontime_get(i), i);

				check_get_current_brightness_distr(hwresolution, scanmode, cisid, i, distr_red, distr_green, distr_blue, SCANNING_FLAG_LIGHTS_ON);

				if ((i == LIGHT_VI_RED) || (i == LIGHT_VI_IR) || (i == LIGHT_VI_IRTR))
					max_light_distr = max_value_get(distr_red, sizeof(distr_red)/sizeof(int));
				else if(i == LIGHT_VI_GREEN)
					max_light_distr = max_value_get(distr_green, sizeof(distr_green)/sizeof(int));
				else if((i == LIGHT_VI_BLUE) || (i == LIGHT_VI_UV) || (i == LIGHT_VI_UVTR))
					max_light_distr = max_value_get(distr_blue, sizeof(distr_blue)/sizeof(int));
				if (max_light_distr < MIN_PIXELS_VAL) {
					best_ontime[i] = max_ontime_get(i);
					dbg_printf("max_light_distr=%d,MAX boundary light %s on_time is %x!\n", max_light_distr, check_light_name[i], max_ontime_get(i));
					break;
				}

				//check the  MIN boundary
				check_scanner_set_led_ontime(actual_cisid, MIN_ONTIME_ALL, i);
				check_get_current_brightness_distr(hwresolution, scanmode, cisid, i, distr_red, distr_green, distr_blue, SCANNING_FLAG_LIGHTS_ON);

				if ((i == LIGHT_VI_RED) || (i == LIGHT_VI_IR) || (i == LIGHT_VI_IRTR))
					max_light_distr = max_value_get(distr_red, sizeof(distr_red)/sizeof(int));
				else if(i == LIGHT_VI_GREEN)
					max_light_distr = max_value_get(distr_green, sizeof(distr_green)/sizeof(int));
				else if((i == LIGHT_VI_BLUE) || (i == LIGHT_VI_UV) || (i == LIGHT_VI_UVTR))
					max_light_distr = max_value_get(distr_blue, sizeof(distr_blue)/sizeof(int));
				if (max_light_distr > MAX_PIXELS_VAL) {
					best_ontime[i] = max_ontime_get(i);
					dbg_printf("MIN boundary light %s on_time is %x!\n", check_light_name[i], MIN_ONTIME_ALL);
					break;
				}

				max_ontime = max_ontime_get(i);
				min_ontime = MIN_ONTIME_ALL;
				dbg_printf("this is %d time to calibrate %s light\n", checkcnt, check_light_name[i]);
				while((abs(max_ontime - min_ontime) != 1)&&(max_ontime != min_ontime))
				{
					cur_ontime = (min_ontime + max_ontime)/2;
					dbg_printf("max_ontime = %02x, min_ontime = %02x, cur_ontime = %02x\n", \
						max_ontime, min_ontime, cur_ontime);

					check_scanner_set_led_ontime(actual_cisid, cur_ontime, i);

					check_get_current_brightness_distr(hwresolution, scanmode, cisid, i, distr_red, distr_green, distr_blue, SCANNING_FLAG_LIGHTS_ON);

					if ((i == LIGHT_VI_RED) || (i == LIGHT_VI_IR) || (i == LIGHT_VI_IRTR)) //red,ir(irtr must 0x133F)
					{
						max_red_distr = max_value_get(distr_red, sizeof(distr_red)/sizeof(int));
						if(max_red_distr < MIN_PIXELS_VAL)//pixels 230 are　too few, turn big
						{
							min_ontime = cur_ontime;
//							dbg_printf("turn big\n");
							continue;
						}
						else if(max_red_distr > MAX_PIXELS_VAL){//pixels 230 are too many, turn small
							max_ontime = cur_ontime;
//							dbg_printf("turn small\n");
							continue;
						}
						else
						{
							best_ontime[i] = cur_ontime;
							dbg_printf("best cur_ontime for %s is %x!\n", check_light_name[i], cur_ontime);
							dbg_printf("max_red_distr = %d\n", max_red_distr);
							break;
						}
					}

					if((i == LIGHT_VI_GREEN))//green
					{
						max_green_distr = max_value_get(distr_green, sizeof(distr_green)/sizeof(int));
						if((max_green_distr < MIN_PIXELS_VAL))//pixels 230 are too few, turn big
						{
							min_ontime = cur_ontime;
							continue;
						}
						else if((max_green_distr > MAX_PIXELS_VAL)){//pixels 230 are too many, turn small
							max_ontime = cur_ontime;
							continue;
						}
						else
						{
							best_ontime[i] = cur_ontime;
							dbg_printf("best cur_ontime for %s is %x!\n", check_light_name[i], cur_ontime);
							dbg_printf("max_green_distr = %d\n", max_green_distr);
							break;
						}
					}

					if((i == LIGHT_VI_BLUE) || (i == LIGHT_VI_UV))//blue,uv(uvtr must 0x7ea4)
					{
						max_blue_distr = max_value_get(distr_blue, sizeof(distr_blue)/sizeof(int));
						if((max_blue_distr < MIN_PIXELS_VAL))//pixels 230 are　too few, turn big
						{
//							dbg_printf("turn big\n");
							min_ontime = cur_ontime;
							continue;
						}
						else if((max_blue_distr > MAX_PIXELS_VAL))//pixels 230 are　too many, turn small
						{
//							dbg_printf("turn small\n");
							max_ontime = cur_ontime;
							continue;
						}
						else
						{
							best_ontime[i] = cur_ontime;
							dbg_printf("best cur_ontime for %s is %x!\n", check_light_name[i], cur_ontime);
							dbg_printf("max_blue_distr = %d\n", max_blue_distr);
							break;
						}
					}

					if(i == LIGHT_VI_UVTR)
					{
						best_ontime[i] = MAX_ONTIME_UV;
						break;
					}
				}

				checkcnt++;
				if (checkcnt > MAX_CALI_CNT) {
					errrs = (1 << i)|(cisid << 7);
					break;
				}
			}
		}
	}

	if (rgb_step2_flag == true) {
		dbg_printf("enter to step 2 to calibrate rgb!\n");
		/*step 2 rgb calibrate together,first scan image use current rgb ontime */
		memcpy(ontime_raw, best_ontime, sizeof(ontime_raw));

		//change cis rgb on_time
		check_cis_light_config[0].value = ontime_raw[0];
		check_cis_light_config[1].value = ontime_raw[1];
		check_cis_light_config[2].value = ontime_raw[2];
		config.regcount = 3;
		config.regconfig = (struct scan_reg_config *)check_cis_light_config; 

		scanunit_set_sensor_config(&checkscanner, cisid, &config);

		//scan image data
		check_get_current_brightness_distr(hwresolution, scanmode, cisid, 0, distr_red, distr_green, distr_blue, SCANNING_FLAG_LIGHTS_ON);

		max_distr[0] = max_value_get(distr_red, sizeof(distr_red)/sizeof(int));
		max_distr[1] = max_value_get(distr_green, sizeof(distr_green)/sizeof(int));
		max_distr[2] = max_value_get(distr_blue, sizeof(distr_blue)/sizeof(int));
		dbg_printf("max_dist= %d,%d,%d\n", max_distr[0], max_distr[1], max_distr[2]);

		checkcnt = 0;
		if ((max_distr[0] > MAX_PIXELS_VAL) && (max_distr[1] > MAX_PIXELS_VAL) && (max_distr[2] > MAX_PIXELS_VAL)) {
			/*step 2 rgb　calibrate together,second scan image use changed current rgb ontime */
			while((max_distr[0] < MIN_PIXELS_VAL)||(max_distr[1] < MIN_PIXELS_VAL)||(max_distr[2] < MIN_PIXELS_VAL) \
			      ||(max_distr[0] > MAX_PIXELS_VAL)||(max_distr[1] > MAX_PIXELS_VAL)||(max_distr[2] > MAX_PIXELS_VAL))
			{
				int rgb_ontime[3] = {0};
				int calibrate_id;
				if((max_distr[0] < MIN_PIXELS_VAL)||(max_distr[0] > MAX_PIXELS_VAL))
					calibrate_id = 0;
				else if((max_distr[1] < MIN_PIXELS_VAL)||(max_distr[1] > MAX_PIXELS_VAL))
					calibrate_id = 1;
				else if((max_distr[2] < MIN_PIXELS_VAL)||(max_distr[2] > MAX_PIXELS_VAL))
					calibrate_id = 2;
				max_ontime = ontime_raw[calibrate_id];
				min_ontime = ontime_raw[calibrate_id]/5;

				dbg_printf("this is %d time to calibrate rgb light at step 2\n", checkcnt); 
				memcpy(rgb_ontime, best_ontime, sizeof(rgb_ontime));

				while((abs(max_ontime - min_ontime) != 1)&&(max_ontime != min_ontime))
				{
					//change cis light on_time 
					rgb_ontime[calibrate_id] = (min_ontime + max_ontime)/2;
					check_cis_light_config[0].value = rgb_ontime[0];
					check_cis_light_config[1].value = rgb_ontime[1];
					check_cis_light_config[2].value = rgb_ontime[2];					config.regcount = 3;
					config.regcount = 3;
					config.regconfig = (struct scan_reg_config *)check_cis_light_config; 
					scanunit_set_sensor_config(&checkscanner, cisid, &config);
					check_get_current_brightness_distr(hwresolution, scanmode, cisid, 0, distr_red, distr_green, distr_blue, SCANNING_FLAG_LIGHTS_ON);
					max_distr[0] = max_value_get(distr_red, sizeof(distr_red)/sizeof(int));
					max_distr[1] = max_value_get(distr_green, sizeof(distr_green)/sizeof(int));
					max_distr[2] = max_value_get(distr_blue, sizeof(distr_blue)/sizeof(int));

					if((max_distr[calibrate_id] < MIN_PIXELS_VAL))//pixels 220 are　too few, turn big
					{
						min_ontime = rgb_ontime[calibrate_id];
						continue;
					}
					else if((max_distr[calibrate_id] > MAX_PIXELS_VAL))//pixels 230 are　too many, turn small
					{
						max_ontime = rgb_ontime[calibrate_id];
						continue;
					}
					else
					{
						best_ontime[calibrate_id] = rgb_ontime[calibrate_id];
						dbg_printf("best cur_ontime for %s is %x.\n", check_light_name[calibrate_id], best_ontime[calibrate_id]);
						break;
					}
				}

				dbg_printf("MAX_DISTR_VAL pixels: red=%d, green=%d, blue=%d\n", max_distr[0], max_distr[1], max_distr[2]);
				checkcnt++;
				if (checkcnt > MAX_CALI_CNT-1)
				{
					errrs = (1 << i)|(cisid << 13);
					break;
				}
			}
		}
	}

	if (errrs == 0) {
			//save the final on_time to cis device
			check_paraconfig.config.config_tpye = CONFIG_CIS;	
			check_paraconfig.config.mode= scanmode;
			check_paraconfig.config.hwresolution = hwresolution;

			check_paraconfig.config.sideid = CHECK_SIDE_A;	
			scanner_read_config_from_nvm(check_paraconfig.para, (unsigned char*)dpi_200_cis_current_a, sizeof(dpi_200_cis_current_a));
			check_paraconfig.config.sideid = CHECK_SIDE_B;
			scanner_read_config_from_nvm(check_paraconfig.para, (unsigned char*)dpi_200_cis_current_b, sizeof(dpi_200_cis_current_b));

			if (cisid == CHECK_SIDE_A) {
				memcpy(dpi_200_cis_current_a, best_ontime, sizeof(best_ontime));
				if (best_ontime[LIGHT_VI_IRTR] > 0)
					dpi_200_cis_current_b[LIGHT_VI_IRTR] = best_ontime[LIGHT_VI_IRTR];
				if (best_ontime[LIGHT_VI_UVTR] > 0)
					dpi_200_cis_current_b[LIGHT_VI_UVTR] = best_ontime[LIGHT_VI_UVTR];
			}
			else
			{
				memcpy(dpi_200_cis_current_b, best_ontime, sizeof(best_ontime));
				if (best_ontime[LIGHT_VI_IRTR] > 0)
					dpi_200_cis_current_a[LIGHT_VI_IRTR] = best_ontime[LIGHT_VI_IRTR];
				if (best_ontime[LIGHT_VI_UVTR] > 0)
					dpi_200_cis_current_a[LIGHT_VI_UVTR] = best_ontime[LIGHT_VI_UVTR];
			}


		for (i = 0; i < lights_cnt; i++) {
			if (light_index[i] == 1)
				dbg_printf("light %s best on_ontime = %x\n", check_light_name[i], best_ontime[i]);
		}
	}
	check_scanner_set_all_led_ontime(checkscanner.hwinfo.sensor_a);
	check_scanner_set_all_led_ontime(checkscanner.hwinfo.sensor_b);
	scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_a, LIGHTSOURCE_ALL);
	scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_b, LIGHTSOURCE_ALL);

	return errrs;
}


int check_scanner_adjust_afe_offset(int hwresolution, int scanmode, int afeid)
{
		int errrs;
		int off_low, off_high, offset;
		int distr_red[256] = {0}, distr_green[256] = {0}, distr_blue[256] = {0};
		int checkcnt, ok_flag = 0;

		offset = 0;
		checkcnt = 0;
		errrs = 0;
	
		while(ok_flag == 0)
		{
			off_low = 0;
			off_high = 0xff;
	
			dbg_printf("this is %d time to calibrate %d adc_offset\n", checkcnt, afeid);
			while((abs(off_high - off_low) != 1)&&(abs(off_low - off_high) != 0))
			{
				offset = (off_low + off_high)/2;
				scanner_set_afe_offset(&checkscanner, afeid, offset);
	
				//scan image data
				check_get_current_brightness_distr(hwresolution, scanmode, afeid, 0, distr_red, distr_green, distr_blue, 0);
				if(((distr_blue[BOUNDARY_DISTR] < 1)&&(distr_green[BOUNDARY_DISTR] < 1)&&(distr_red[BOUNDARY_DISTR] < 1))\
				   && (distr_red[0] == 0) && (distr_green[0] == 0) && (distr_blue[0] == 0))//0 too few, turn small
				{
					off_high = offset;
					continue;
				}
				else if((distr_red[0] > 0)||(distr_green[0] > 0)||(distr_blue[0] > 0) \
					||(distr_blue[BOUNDARY_DISTR] > 1)||(distr_green[BOUNDARY_DISTR] > 1)||(distr_red[BOUNDARY_DISTR] > 1)){//0 too many, turn big
					off_low = offset;
					continue;
				}
				else
				{
					dbg_printf("best offset is %x .\n", offset);
					dbg_printf("number of zero pixels: red=%d, green=%d, blue=%d\n", distr_red[0], distr_green[0], distr_blue[0]);
					dbg_printf("number of BOUNDARY_DISTR pixels: red=%d, green=%d, blue=%d\n", distr_red[BOUNDARY_DISTR], distr_green[BOUNDARY_DISTR], distr_blue[BOUNDARY_DISTR]);
					errrs = 0;
					ok_flag = 1;
					break;
				}
			}
			checkcnt++;
			if (checkcnt >= MAX_CALI_CNT)
			{
				errrs = (1 << 14)|(afeid << 13);
				return errrs;
			}
		}
	//	write the final on_time to cis device
		if(afeid == CHECK_SIDE_A) {
			dpi_200_adc_current_a[0] = offset;
		}
		else if (afeid == CHECK_SIDE_B) {
			dpi_200_adc_current_b[0] = offset;
		}	
	return errrs;

}

int check_scanner_write_calidata(unsigned char *pdata, int len, int side, int type)
{
	int rs;
	check_paraconfig.config.config_tpye = CONFIG_CALI;
	check_paraconfig.config.mode = check_scanning_ctrl.scanmode;
	check_paraconfig.config.hwresolution = check_scanning_ctrl.hwresolution;
	check_paraconfig.config.sideid = side;

	if(type == 0)
	{
		if(check_scanning_ctrl.scanmode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS)
			check_paraconfig.config.mode = FAKE_CIS_SCANMODE_EN_SIX_LIGHTS_DARK;
		if(check_scanning_ctrl.scanmode == FPGA_REG_CIS_SCANMODE_TEN_LIGHTS)
			check_paraconfig.config.mode = FAKE_CIS_SCANMODE_EN_TEN_LIGHTS_DARK;
	}
	rs = scanner_write_config_to_nvm(check_paraconfig.para, pdata, len);

	return rs;
}


int check_scanner_read_calidata(unsigned char *pdata, int *len, int side, int type)
{
	int rs;
	check_paraconfig.config.config_tpye = CONFIG_CALI;
	check_paraconfig.config.mode = check_scanning_ctrl.scanmode;
	check_paraconfig.config.hwresolution = check_scanning_ctrl.hwresolution;
	check_paraconfig.config.sideid = side;

	if(type == 0)
	{
		if(check_scanning_ctrl.scanmode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS)
			check_paraconfig.config.mode = FAKE_CIS_SCANMODE_EN_SIX_LIGHTS_DARK;
		if(check_scanning_ctrl.scanmode == FPGA_REG_CIS_SCANMODE_TEN_LIGHTS)
			check_paraconfig.config.mode = FAKE_CIS_SCANMODE_EN_TEN_LIGHTS_DARK;
	}

	*len = CIS_PIXELS_200DPI*3 + CALI_EXTRA_SIZE;

	rs = scanner_read_config_from_nvm(check_paraconfig.para, pdata, *len);

	return rs;
}


int check_scanner_calibrate_adjustable_paramters(void)
{
	int i, cnt, rs = 0;

	for(i = 0; i < ADJUSTABLE_CONFIG_NUM; i++){
	    if(check_adjust_para_t[i].config_tpye == CONFIG_ADC){
		cnt = 0;
		while (cnt < 3) {
			cnt++;
			rs = check_scanner_adjust_afe_offset(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].side);
			if ((rs != 0) && (cnt > 2))
				return rs;
			else{
				if(check_adjust_para_t[i].side == CHECK_SIDE_A)
					rs = check_scanner_save_ajustable_config(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].config_tpye, checkscanner.hwinfo.sensor_a);
				else
					rs = check_scanner_save_ajustable_config(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].config_tpye, checkscanner.hwinfo.sensor_b);
				if(rs != 0)
					return -1;
				break;
			}
		}
	    }else if(check_adjust_para_t[i].config_tpye == CONFIG_CIS){
		cnt = 0;
		while (cnt < 3) {
			cnt++;
			rs = check_scanner_adjust_cis_led_brightness(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].side, LIGHTSOURCE_ALL);
			if ((rs != 0) && (cnt > 2))
				return rs;
			else{
				if(check_adjust_para_t[i].side == CHECK_SIDE_A)
					rs = check_scanner_save_ajustable_config(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].config_tpye, checkscanner.hwinfo.sensor_a);
				else
					rs = check_scanner_save_ajustable_config(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].config_tpye, checkscanner.hwinfo.sensor_b);
				if(rs != 0)
					return -1;
				break;
			}
		}
	    }else
		return -1;
	}
	return rs;
}


void check_combine_calidatablock(unsigned char *buffer, int datalen)
{
	int i;
	unsigned char *ptr = buffer;
	for(i=0; i< CHECK_MAX_SCAN_LIGHTSOURCES; i++)
	{
		if(check_configlightside_to_lightaddr_table[i+1].lightaddr != check_configlightside_to_lightaddr_table[i].lightaddr)
		{
			memcpy(ptr, (unsigned char *)check_configlightside_to_lightaddr_table[i].lightaddr, datalen);
			ptr += datalen;
		}
	}
}

