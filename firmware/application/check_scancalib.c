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
#define MAX_ONTIME_VI_IR	0xadc
#define MAX_ONTIME_UV		0xadc
#define MIN_ONTIME_ALL		0

#define MIN_PIXELS_NUM		10
#define MAX_PIXELS_NUM		100
#define MAX_PIXELS_VAL		230
#define MIN_PIXELS_VAL		220
#define MAX_LIGHT_VAL		230

#define MAX_CALIBRATE_SCANLINES	100
#define CIS_UP_START		200
#define CIS_DOWM_START		(CHECK_SCANLINE_PIXELS-CIS_UP_START)
#define INACTIVE_PIXELS_NUM	0x510

#define RED_ONTIME_MASK		0x40
#define LIGHT_CALIBRATE_NUM	7

#define MAX_CALI_CNT		20
#define CIS_BAR1_START		0
#define CIS_BAR1_WIDTH		20
#define CIS_BAR2_START		(CHECK_SCANLINE_PIXELS-20)
#define CIS_BAR2_WIDTH		20
#define DISTR_SIZE		256

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
#define CALI_EXTRA_SIZE		(2+CALI_CHECKSUM_SIZE)
#define CALIBUF_LEN		(14*3*CIS_PIXELS_200DPI)

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

DEFINE_NVM_DATA(SCAN_DPI_200_ADC_A, sizeof(dpi_200_adc_current_a), dpi_200_adc_current_a);
DEFINE_NVM_DATA(SCAN_DPI_200_CIS_A, sizeof(dpi_200_cis_current_a), dpi_200_cis_current_a);
DEFINE_NVM_DATA(SCAN_DPI_200_CALIBDATA, CALIBUF_LEN, NULL);


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

enum three_scan_index_order {
	THREE_LIGHT_VI_A,
	THREE_LIGHT_IR_A,
	THREE_LIGHT_UV_A,
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

const unsigned int six_img_scan_ida[7] = {
	SIX_LIGHT_VI_A,		// R
	SIX_LIGHT_VI_A,		// G
	SIX_LIGHT_VI_A,		// B
	SIX_LIGHT_IR_A,		// IR
	SIX_LIGHT_IRT_A,	// IRT
	SIX_LIGHT_UV_A,		// UV
	SIX_LIGHT_UVT_A,	// UVT
};

const unsigned int six_img_scan_idb[3] = {
	SIX_LIGHT_VI_B,		// R
	SIX_LIGHT_VI_B,		// G
	SIX_LIGHT_VI_B,		// B
};

const unsigned int ten_img_scan_ida[7] = {
	TEN_LIGHT_VI_A,		// R
	TEN_LIGHT_VI_A,		// G
	TEN_LIGHT_VI_A,		// B
	TEN_LIGHT_IR_A,		// IR
	TEN_LIGHT_IRT_A,	// IRT
	TEN_LIGHT_UV_A,		// UV
	TEN_LIGHT_UVT_A,	// UVT
};

const unsigned int ten_img_scan_idb[7] = {
	TEN_LIGHT_VI_B,		// R
	TEN_LIGHT_VI_B,		// G
	TEN_LIGHT_VI_B,		// B
	TEN_LIGHT_IR_B,		// IR
	TEN_LIGHT_IRT_B,	// IRT
	TEN_LIGHT_UV_B,		// UV
	TEN_LIGHT_UVT_B,	// UVT
};

const unsigned char ten_light_flag[10] = {
	0x08,		// VI_A
	0x48,		// VI_B
	0x10,		// IR_A
	0x50,		// IR_B			
	0x18,		// IRT_A
	0x58,		// IRT_B
	0x20,		// UV_A
	0x60,		// UV_B
	0x28,		// UVT_A
	0x68,		// UVT_B
};

const unsigned char six_light_flag[6] = {
	0x08,		// VI_A
	0x48,		// VI_B
	0x10,		// IR_A
	0x18,		// IRT_A
	0x20,		// UV_A
	0x28,		// UVT_A
};

const unsigned char three_light_flag[3] = {
	0x08,		// VI_A
	0x10,		// IR_A
	0x20,		// UV_A
};

const unsigned int three_img_scan_ida[5] = {
	THREE_LIGHT_VI_A,		// R
	THREE_LIGHT_VI_A,		// G
	THREE_LIGHT_VI_A,		// B
	THREE_LIGHT_IR_A,		// IR
	THREE_LIGHT_UV_A,		// UV
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
	{CONFIG_ADC, SIDE_A, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV},	//200DPI TENMODE ADC OFFSET ADJUST
	{CONFIG_CIS, SIDE_A, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV},	//200DPI TENMODE CIS ONTIME ADJUST
};
#define ADJUSTABLE_CONFIG_NUM	(sizeof(check_adjust_para_t)/sizeof(struct scanner_adjustable_parameters_config))

scanning_scanid_t pastport_lightsourcetoscanid_table[] = {
	{EXPOSURE_LIGHTSOURCE_RED,	0},
	{EXPOSURE_LIGHTSOURCE_GREEN, 	1},
	{EXPOSURE_LIGHTSOURCE_BLUE,	2},
	{EXPOSURE_LIGHTSOURCE_IR,	3},
	{EXPOSURE_LIGHTSOURCE_UV,	4},
};

static unsigned char pastport_lightsource_to_scanid(unsigned char lightsource)
{
	int i; 

	for (i = 0; i < sizeof(pastport_lightsourcetoscanid_table)/sizeof(scanning_scanid_t); i++) {
		if (pastport_lightsourcetoscanid_table[i].lightsource == lightsource) {
			return pastport_lightsourcetoscanid_table[i].scanid;
		}
	}
	return 0;
}

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
		printf("check_scanner_load_ajustable_config fail, rs=%d\r\n", rs);
		memcpy(dpi_200_adc_current_a, dpi_200_adc_default_a, sizeof(dpi_200_adc_default_a));
		memcpy(dpi_200_cis_current_a, dpi_200_cis_default_a, sizeof(dpi_200_cis_default_a));
	}

        //set afe config
	scanner_set_afe_offset(&checkscanner, SIDE_A, dpi_200_adc_current_a[0]);
        printf("adc offset=%x\r\n", dpi_200_adc_current_a[0]);
	//set cis a config
	config.regcount = CHECK_LIGHT_SOURCE_NUM;
	for(i = 0; i < config.regcount; i++)
	{
		check_cis_light_config[i].value = dpi_200_cis_current_a[i];
		printf("cis %s ontime=%x\r\n", check_light_name[i], dpi_200_cis_current_a[i]);
	}

	config.regconfig = (struct scan_reg_config *)check_cis_light_config;
	scanunit_set_sensor_config(&checkscanner, checkscanner.hwinfo.sensor_a, &config);

	return rs;
}


int check_scanner_load_ajustable_config(int hwresolution, int hwmode)
{
	int rs;

	paraconfig.config.mode = hwmode;
	paraconfig.config.hwresolution = hwresolution;
	paraconfig.config.unitid = CHECK_SCANUNIT;
	printf("scanner_load_ajustable_config: hwresolution=%x, scanmode=%x\r\n", hwresolution, hwmode);

	paraconfig.config.config_tpye = CONFIG_ADC;
	paraconfig.config.sideid = SIDE_A;
	rs = scanner_read_config_from_nvm(paraconfig.para, buf.config, sizeof(dpi_200_adc_default_a));
	if (rs == -1) {
		printf("read afe offset config from nvm fail!\r\n");
		return rs;
	}
	memcpy(dpi_200_adc_current_a, buf.config, sizeof(dpi_200_adc_current_a));

	paraconfig.config.config_tpye = CONFIG_CIS;
	paraconfig.config.sideid= SIDE_A;
	rs = scanner_read_config_from_nvm(paraconfig.para, buf.config, sizeof(dpi_200_cis_current_a));
	if (rs == -1) {
		printf("read cis config from nvm fail!\r\n");
		return rs;
	}
	memcpy(dpi_200_cis_current_a, buf.config, sizeof(dpi_200_cis_current_a));

	return 0;
}


int check_scanner_save_ajustable_config(int hwresolution, int hwmode, int para_type, int side)
{
	int rs;

	paraconfig.config.unitid = CHECK_SCANUNIT;
	paraconfig.config.mode = hwmode;
	paraconfig.config.hwresolution = hwresolution;
	paraconfig.config.config_tpye = para_type;
	paraconfig.config.sideid = side;

	if (para_type == CONFIG_ADC) {
		if(side == SIDE_A)
		{
			memcpy(buf.config, dpi_200_adc_current_a, sizeof(dpi_200_adc_current_a));
			rs = scanner_write_config_to_nvm(paraconfig.para, buf.config, sizeof(dpi_200_adc_current_a));
			if (rs == -1)
				return rs;
		}
		else
		{
			memcpy(buf.config, dpi_200_adc_current_b, sizeof(dpi_200_adc_current_b));
			rs = scanner_write_config_to_nvm(paraconfig.para, buf.config, sizeof(dpi_200_adc_current_b));
			if (rs == -1)
				return rs;
		}

	}else if(para_type == CONFIG_CIS){
		if(side == SIDE_A)
		{
			memcpy(buf.config, dpi_200_cis_current_a, sizeof(dpi_200_cis_current_a));
			rs = scanner_write_config_to_nvm(paraconfig.para, buf.config, sizeof(dpi_200_cis_current_a));
			if (rs == -1) {
				return rs;
			}
		}
		else
		{
			memcpy(buf.config, dpi_200_cis_current_b, sizeof(dpi_200_cis_current_b));
			rs = scanner_write_config_to_nvm(paraconfig.para, buf.config, sizeof(dpi_200_cis_current_b));
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

	cis_ontime_config[0].address = RED_ONTIME_MASK + lightsource*4;
	cis_ontime_config[0].value = value;
	config.regcount = 1;
	config.regconfig = (struct scan_reg_config *)cis_ontime_config;
	scanunit_set_sensor_config(&checkscanner, cisid, &config);
}



int check_get_current_brightness_distr(struct scanunit *scanner, int hwresolution, int scanmode, int lights, int side, int lightsource, int *red_distr, int *green_distr, int *blue_distr, int flags)
{
	int scan_len, count, imgmask, scanlines, active_linepixel = 0;
        unsigned char *scanptr, *ptr;
        const unsigned int *img_scan_id;
	const unsigned char *light_flag;
	
	if (lights == 6) {
		light_flag = six_light_flag;
		if(side == 0)
			img_scan_id = six_img_scan_ida;
		else if(side == 1)
			img_scan_id = six_img_scan_idb;
	}
	else if (lights == 10) {
		light_flag = ten_light_flag;
		if(side == 0)
			img_scan_id = ten_img_scan_ida;
		else if(side == 1)
			img_scan_id = ten_img_scan_idb;
	}else if (lights == 3) {
		light_flag = three_light_flag;
		img_scan_id = three_img_scan_ida;
	}

	scan_len = lights * CHECK_SCANLINE_DATA_CNT;
	imgmask = img_scan_id[lightsource];

	//scan image data
	scanner_set_sensor_config(scanner, hwresolution, scanmode, side);

	if(flags == SCANNING_FLAG_LIGHTS_ON)
		scanunit_turnon_lights(scanner);
	else
	    	scanunit_turnoff_lights(scanner);

	scanner_stop_scanning(scanner, IAM_A_PAGE);
	scanner_start_scanning(scanner, IAM_A_PAGE);
	scanlines = 0;

	while (scanlines < 2* MAX_CALIBRATE_SCANLINES){
		scanlines = scanner_get_wraddr(scanner)/CHECK_SCANLINE_SIZE;
	}
	scanner_stop_scanning(scanner, IAM_A_PAGE);

	count = 0;
	scanptr = (unsigned char *)(0xa001E840);
	ptr = (unsigned char *)buf.sectionbuf;

	while (count < MAX_CALIBRATE_SCANLINES)
	{	
		memcpy(ptr, scanptr + imgmask*CHECK_SCANLINE_DATA_CNT, CHECK_SCANLINE_DATA_CNT);
		scanptr += scan_len;
		ptr += CHECK_SCANLINE_DATA_CNT;
		++count;
	}
	img_rawdata_section((unsigned char*)buf.sectionbuf, buf.buffer, MAX_CALIBRATE_SCANLINES*CHECK_SCANLINE_DATA_CNT, CHECK_SCANLINE_DATA_CNT, CIS_UP_START*4, CIS_DOWM_START*4);
	
	active_linepixel = CIS_DOWM_START - CIS_UP_START;

//	memcpy(buf.buffer, buf.sectionbuf, active_cnt*active_linepixel*4);
	img_rawdata_color_distribution((unsigned char *)buf.buffer, MAX_CALIBRATE_SCANLINES*active_linepixel, red_distr, \
  			green_distr, blue_distr);
	return 0;
}


int check_scanner_adjust_cis_led_brightness(int hwresolution, unsigned int scanmode, int cisid, unsigned int light_sel)
{
	int i, scanid, lights_cnt = 0;
	struct scanunit_config config;
	int min_ontime, max_ontime, cur_ontime;
	int checkcnt, errrs;
	int max_distr[3];
	uint16_t ontime_raw[3] = {0};
	uint16_t best_ontime[LIGHT_CALIBRATE_NUM] = {0};
	unsigned int light_index[LIGHT_CALIBRATE_NUM] = {0};
	bool rgb_step2_flag = false;

	if (scanmode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS) {
		if(cisid == checkscanner.hwinfo.sensor_b)
			light_sel &= 0x7;
	}
    
	//reset other side ontime
	if(checkscanner.hwinfo.sides == 2)
	{
		if(cisid == checkscanner.hwinfo.sensor_a) 
			scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_b, LIGHTSOURCE_IRT|LIGHTSOURCE_UVT);
		else if(cisid == checkscanner.hwinfo.sensor_b)
		scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_a, LIGHTSOURCE_IRT|LIGHTSOURCE_UVT);
	}
	
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
#if 1
	for (i = 0; i < LIGHT_CALIBRATE_NUM; i++) 
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
                                scanid = pastport_lightsource_to_scanid(i);
				check_get_current_brightness_distr(&checkscanner, hwresolution, scanmode, 3, cisid, scanid, buf.distr_red, buf.distr_green, buf.distr_blue, SCANNING_FLAG_LIGHTS_ON);

				if ((i == LIGHT_VI_RED) || (i == LIGHT_VI_IR) || (i == LIGHT_VI_IRTR))
					max_light_distr = max_value_get(buf.distr_red, DISTR_SIZE);
				else if(i == LIGHT_VI_GREEN)
					max_light_distr = max_value_get(buf.distr_green, DISTR_SIZE);
				else if((i == LIGHT_VI_BLUE) || (i == LIGHT_VI_UV) || (i == LIGHT_VI_UVTR))
					max_light_distr = max_value_get(buf.distr_blue, DISTR_SIZE);
				if (max_light_distr < MIN_PIXELS_VAL) {
					best_ontime[i] = max_ontime_get(i);
					printf("max_light_distr=%d,MAX boundary light %s on_time is %x!\r\n", max_light_distr, check_light_name[i], max_ontime_get(i));
					break;
				}

				//check the  MIN boundary
				check_scanner_set_led_ontime(actual_cisid, MIN_ONTIME_ALL, i);
				scanid = pastport_lightsource_to_scanid(i);
				check_get_current_brightness_distr(&checkscanner, hwresolution, scanmode, 3, cisid, scanid, buf.distr_red, buf.distr_green, buf.distr_blue, SCANNING_FLAG_LIGHTS_ON);

				if ((i == LIGHT_VI_RED) || (i == LIGHT_VI_IR) || (i == LIGHT_VI_IRTR))
					max_light_distr = max_value_get(buf.distr_red, DISTR_SIZE);
				else if(i == LIGHT_VI_GREEN)
					max_light_distr = max_value_get(buf.distr_green, DISTR_SIZE);
				else if((i == LIGHT_VI_BLUE) || (i == LIGHT_VI_UV) || (i == LIGHT_VI_UVTR))
					max_light_distr = max_value_get(buf.distr_blue, DISTR_SIZE);
				if (max_light_distr > MAX_PIXELS_VAL) {
					best_ontime[i] = max_ontime_get(i);
					printf("MIN boundary light %s on_time is %x!\r\n", check_light_name[i], MIN_ONTIME_ALL);
					break;
				}

				max_ontime = max_ontime_get(i);
				min_ontime = MIN_ONTIME_ALL;
				printf("this is %d time to calibrate %s light\r\n", checkcnt, check_light_name[i]);
				while((abs(max_ontime - min_ontime) != 1)&&(max_ontime != min_ontime))
				{
					cur_ontime = (min_ontime + max_ontime)/2;
					printf("max_ontime = %02x, min_ontime = %02x, cur_ontime = %02x\r\n", \
						max_ontime, min_ontime, cur_ontime);

					check_scanner_set_led_ontime(actual_cisid, cur_ontime, i);
                                        scanid = pastport_lightsource_to_scanid(i);
					check_get_current_brightness_distr(&checkscanner, hwresolution, scanmode, 3, cisid, scanid, buf.distr_red, buf.distr_green, buf.distr_blue, SCANNING_FLAG_LIGHTS_ON);

					if ((i == LIGHT_VI_RED) || (i == LIGHT_VI_IR) || (i == LIGHT_VI_IRTR)) //red,ir(irtr must 0x133F)
					{
						max_red_distr = max_value_get(buf.distr_red, DISTR_SIZE);
						if(max_red_distr < MIN_PIXELS_VAL)//pixels 230 are　too few, turn big
						{
							min_ontime = cur_ontime;
							printf("max_red_distr=%d,turn big!\r\n", max_red_distr);
							continue;
						}
						else if(max_red_distr > MAX_PIXELS_VAL){//pixels 230 are too many, turn small
							max_ontime = cur_ontime;
							printf("max_red_distr=%d,turn small!\r\n", max_red_distr);
							continue;
						}
						else
						{
							best_ontime[i] = cur_ontime;
							printf("best cur_ontime for %s is %x!\r\n", check_light_name[i], cur_ontime);
							printf("max_red_distr = %d\r\n", max_red_distr);
							break;
						}
					}

					if((i == LIGHT_VI_GREEN))//green
					{
						max_green_distr = max_value_get(buf.distr_green, DISTR_SIZE);
						if((max_green_distr < MIN_PIXELS_VAL))//pixels 230 are too few, turn big
						{
							min_ontime = cur_ontime;
							printf("max_green_distr=%d,turn big!\r\n", max_green_distr);
							continue;
						}
						else if((max_green_distr > MAX_PIXELS_VAL)){//pixels 230 are too many, turn small
							max_ontime = cur_ontime;
							printf("max_green_distr=%d,turn small!\r\n", max_green_distr);
							continue;
						}
						else
						{
							best_ontime[i] = cur_ontime;
							printf("best cur_ontime for %s is %x!\r\n", check_light_name[i], cur_ontime);
							printf("max_green_distr = %d\r\n", max_green_distr);
							break;
						}
					}

					if((i == LIGHT_VI_BLUE) || (i == LIGHT_VI_UV))//blue,uv(uvtr must 0x7ea4)
					{
						max_blue_distr = max_value_get(buf.distr_blue, DISTR_SIZE);
						if((max_blue_distr < MIN_PIXELS_VAL))//pixels 230 are　too few, turn big
						{
							printf("max_blue_distr=%d,turn big!\r\n", max_blue_distr);
							min_ontime = cur_ontime;
							continue;
						}
						else if((max_blue_distr > MAX_PIXELS_VAL))//pixels 230 are　too many, turn small
						{
							printf("max_blue_distr=%d,turn small!\r\n", max_blue_distr);
							max_ontime = cur_ontime;
							continue;
						}
						else
						{
							best_ontime[i] = cur_ontime;
							printf("best cur_ontime for %s is %x!\r\n", check_light_name[i], cur_ontime);
							printf("max_blue_distr = %d\r\n", max_blue_distr);
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
		printf("enter to step 2 to calibrate rgb!\r\n");
		/*step 2 rgb calibrate together,first scan image use current rgb ontime */
		memcpy(ontime_raw, best_ontime, sizeof(ontime_raw));

		//change cis rgb on_time
		check_cis_light_config[0].value = ontime_raw[0];
		check_cis_light_config[1].value = ontime_raw[1];
		check_cis_light_config[2].value = ontime_raw[2];
		config.regcount = 3;
		config.regconfig = (struct scan_reg_config *)check_cis_light_config; 

		scanunit_set_sensor_config(&checkscanner, cisid, &config);
		scanner_set_cis_lightsource(&checkscanner, cisid, check_lightsel[0]|check_lightsel[1]|check_lightsel[2]);

		//scan image data
		scanid =  pastport_lightsource_to_scanid(0);
		check_get_current_brightness_distr(&checkscanner, hwresolution, scanmode, 3, cisid, scanid, buf.distr_red, buf.distr_green, buf.distr_blue, SCANNING_FLAG_LIGHTS_ON);

		max_distr[0] = max_value_get(buf.distr_red, DISTR_SIZE);
		max_distr[1] = max_value_get(buf.distr_green, DISTR_SIZE);
		max_distr[2] = max_value_get(buf.distr_blue, DISTR_SIZE);
		printf("max_dist= %d,%d,%d\r\n", max_distr[0], max_distr[1], max_distr[2]);

		checkcnt = 0;
		if ((max_distr[0] > MAX_PIXELS_VAL) && (max_distr[1] > MAX_PIXELS_VAL) && (max_distr[2] > MAX_PIXELS_VAL)) {
			/*step 2 rgb　calibrate together,second scan image use changed current rgb ontime */
			while((max_distr[0] < MIN_PIXELS_VAL)||(max_distr[1] < MIN_PIXELS_VAL)||(max_distr[2] < MIN_PIXELS_VAL) \
			      ||(max_distr[0] > MAX_PIXELS_VAL)||(max_distr[1] > MAX_PIXELS_VAL)||(max_distr[2] > MAX_PIXELS_VAL))
			{
				uint16_t rgb_ontime[3] = {0};
				int calibrate_id;
				if((max_distr[0] < MIN_PIXELS_VAL)||(max_distr[0] > MAX_PIXELS_VAL))
					calibrate_id = 0;
				else if((max_distr[1] < MIN_PIXELS_VAL)||(max_distr[1] > MAX_PIXELS_VAL))
					calibrate_id = 1;
				else if((max_distr[2] < MIN_PIXELS_VAL)||(max_distr[2] > MAX_PIXELS_VAL))
					calibrate_id = 2;
				max_ontime = ontime_raw[calibrate_id];
				min_ontime = ontime_raw[calibrate_id]/5;

				printf("this is %d time to calibrate rgb light at step 2\r\n", checkcnt); 
				memcpy(rgb_ontime, best_ontime, sizeof(rgb_ontime));

				while((abs(max_ontime - min_ontime) != 1)&&(max_ontime != min_ontime))
				{
					//change cis light on_time 
					rgb_ontime[calibrate_id] = (min_ontime + max_ontime)/2;
					check_cis_light_config[0].value = rgb_ontime[0];
					check_cis_light_config[1].value = rgb_ontime[1];
					check_cis_light_config[2].value = rgb_ontime[2];
					config.regcount = 3;
					config.regconfig = (struct scan_reg_config *)check_cis_light_config; 
					scanunit_set_sensor_config(&checkscanner, cisid, &config);
					scanid = pastport_lightsource_to_scanid(0);
					check_get_current_brightness_distr(&checkscanner, hwresolution, scanmode, 3, cisid, scanid, buf.distr_red, buf.distr_green, buf.distr_blue, SCANNING_FLAG_LIGHTS_ON);
					max_distr[0] = max_value_get(buf.distr_red, DISTR_SIZE);
					max_distr[1] = max_value_get(buf.distr_green, DISTR_SIZE);
					max_distr[2] = max_value_get(buf.distr_blue, DISTR_SIZE);

					if((max_distr[calibrate_id] < MIN_PIXELS_VAL))//pixels 220 are　too few, turn big
					{
						min_ontime = rgb_ontime[calibrate_id];
						printf("max_distr[%d]=%d, turn big\r\n", calibrate_id, max_distr[calibrate_id]);
						continue;
					}
					else if((max_distr[calibrate_id] > MAX_PIXELS_VAL))//pixels 230 are　too many, turn small
					{
						max_ontime = rgb_ontime[calibrate_id];
						printf("max_distr[%d]=%d, turn small\r\n", calibrate_id, max_distr[calibrate_id]);
						continue;
					}
					else
					{
						best_ontime[calibrate_id] = rgb_ontime[calibrate_id];
						printf("best cur_ontime for %s is %x.\r\n", check_light_name[calibrate_id], best_ontime[calibrate_id]);
						break;
					}
				}

				printf("MAX_DISTR_VAL pixels: red=%d, green=%d, blue=%d\r\n", max_distr[0], max_distr[1], max_distr[2]);
				checkcnt++;
				if (checkcnt > MAX_CALI_CNT*3-1)
				{
					errrs = (1 << i)|(cisid << 13);
					break;
				}
			}
		}
	}
#endif
	if (errrs == 0) {
			//save the final on_time to cis device
			paraconfig.config.config_tpye = CONFIG_CIS;	
			paraconfig.config.mode= scanmode;
			paraconfig.config.hwresolution = hwresolution;

			paraconfig.config.sideid = SIDE_A;	
			scanner_read_config_from_nvm(paraconfig.para, (unsigned char*)dpi_200_cis_current_a, sizeof(dpi_200_cis_current_a));

			if (cisid == SIDE_A) {
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


		for (i = 0; i < LIGHT_CALIBRATE_NUM; i++) {
			if (light_index[i] == 1)
				printf("light %s best on_ontime = %x\r\n", check_light_name[i], best_ontime[i]);
		}
	}
	check_scanner_set_all_led_ontime(checkscanner.hwinfo.sensor_a);
	scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_a, LIGHTSOURCE_ALL);

	return errrs;
}


int check_scanner_adjust_afe_offset(int hwresolution, int scanmode, int afeid)
{
	int errrs;
	int off_low, off_high, offset;
	int checkcnt, scanid, ok_flag = 0;

	offset = 0;
	checkcnt = 0;
	errrs = 0;

	while(ok_flag == 0)
	{
		off_low = 0;
		off_high = 0xff;

		printf("this is %d time to calibrate %d adc_offset\r\n", checkcnt, afeid);
		while((abs(off_high - off_low) != 1)&&(abs(off_low - off_high) != 0))
		{
			offset = (off_low + off_high)/2;
			scanner_set_afe_offset(&checkscanner, afeid, offset);
			printf("current adc_offset = %x\r\n", offset);

			//scan image data
			scanid =  pastport_lightsource_to_scanid(0);
			check_get_current_brightness_distr(&checkscanner, hwresolution, scanmode, 3, afeid, scanid, buf.distr_red, buf.distr_green, buf.distr_blue, 0);
			if(((buf.distr_blue[BOUNDARY_DISTR] < 1)&&(buf.distr_green[BOUNDARY_DISTR] < 1)&&(buf.distr_red[BOUNDARY_DISTR] < 1))\
			   && (buf.distr_red[0] == 0) && (buf.distr_green[0] == 0) && (buf.distr_blue[0] == 0))//0 too few, turn small
			{
				off_high = offset;
				printf("turn small\r\n");
				continue;
			}
			else if((buf.distr_red[0] > 0)||(buf.distr_green[0] > 0)||(buf.distr_blue[0] > 0) \
				||(buf.distr_blue[BOUNDARY_DISTR] > 1)||(buf.distr_green[BOUNDARY_DISTR] > 1)||(buf.distr_red[BOUNDARY_DISTR] > 1)){//0 too many, turn big
				off_low = offset;
				printf("turn big\r\n");
				continue;
			}
			else
			{
				printf("best offset is %x .\r\n", offset);
				printf("number of zero pixels: red=%d, green=%d, blue=%d\r\n", buf.distr_red[0], buf.distr_green[0], buf.distr_blue[0]);
				printf("number of BOUNDARY_DISTR pixels: red=%d, green=%d, blue=%d\r\n", buf.distr_red[BOUNDARY_DISTR], buf.distr_green[BOUNDARY_DISTR], buf.distr_blue[BOUNDARY_DISTR]);
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
	if(afeid == SIDE_A) {
		dpi_200_adc_current_a[0] = offset;
	}
	else if (afeid == SIDE_B) {
		dpi_200_adc_current_b[0] = offset;
	}	
	return errrs;

}

int check_scanner_write_calidata(unsigned char *pdata, int len, unsigned char unitid, unsigned char resolution)
{
	int rs;

	paraconfig.config.config_tpye = CONFIG_CALI;
	paraconfig.config.mode = 0xffff;
	paraconfig.config.hwresolution = scanning_ctrl.hwresolution;
	paraconfig.config.sideid = 0;
	paraconfig.config.unitid = unitid&0x3;

	rs = scanner_write_config_to_nvm(paraconfig.para, pdata, len);

	return rs;
}


int check_scanner_read_calidata(unsigned char *pdata, int *len, unsigned char unitid, unsigned char resolution)
{
	int rs;

	paraconfig.config.config_tpye = CONFIG_CALI;
	paraconfig.config.mode = 0xffff;
	paraconfig.config.hwresolution = scanning_ctrl.hwresolution;
	paraconfig.config.sideid = 0;
	paraconfig.config.unitid = unitid&0x3;


	*len = CALIBUF_LEN;

	rs = scanner_read_config_from_nvm(paraconfig.para, pdata, *len);

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
				if(check_adjust_para_t[i].side == SIDE_A)
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
			rs = check_scanner_adjust_cis_led_brightness(check_adjust_para_t[i].hwresolution, check_adjust_para_t[i].hwmode, check_adjust_para_t[i].side, LIGHTSOURCE_RED|LIGHTSOURCE_GREEN|LIGHTSOURCE_BLUE|LIGHTSOURCE_IR|LIGHTSOURCE_UV);
			if ((rs != 0) && (cnt > 2))
				return rs;
			else{
				if(check_adjust_para_t[i].side == SIDE_A)
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


int check_scanner_get_exposure_time(scanning_exposure_time_setting_t *setting, int *exposuretime)
{
	int rs, side, lightsource, configtype, mode;
	unsigned char config[256] = {0};

	side = setting->side;
	configtype = setting->flag;
	lightsource = setting->lightsource_index;
        mode = ((setting->reserved)<<4)|(setting->mode);
	paraconfig.config.unitid = CHECK_SCANUNIT;
	paraconfig.config.mode = check_scanner_ledmodetohwledmode(mode);
	paraconfig.config.hwresolution = check_dpitohwdpi[setting->resolution];
	paraconfig.config.config_tpye = CONFIG_CIS;
	paraconfig.config.sideid = side;
	printf("configtype=%d, side=%d, lightsource=%d\n", configtype, side, lightsource);

	if (side < 0 || side > 1)
		return -2;

	if (configtype == 1) {
                rs = scanner_read_config_from_nvm(paraconfig.para, config, sizeof(dpi_200_cis_current_a));
		if (rs == -1) {
			printf("read cis config from nvm fail!\r\n");
			return rs;
		}
		*exposuretime = config[2*lightsource]+ config[2*lightsource+1]*256;
	}else {
		if (side == 0) {
			*exposuretime = dpi_200_cis_current_a[lightsource];
		}
		else 
			*exposuretime = dpi_200_cis_current_b[lightsource];
	}
	dbg_printf("get exposuretime=%4x\n", *exposuretime);
	return 0;
}

int check_scanner_set_exposure_time(scanning_exposure_time_setting_t *setting, int exposuretime)
{
	int rs, side, save_flag, lightsource, configtype, mode;
	unsigned char config[256] = {0};

	if ((exposuretime > MAX_ONTIME_UV) ||(exposuretime < 0))
		return -1;

	side = setting->side;
	save_flag = setting->flag;
	lightsource = setting->lightsource_index;
	mode = ((setting->reserved)<<4)|(setting->mode);
	printf("side = %d, save_flag=%d, lightsource=%d, exposuretime=%04x\n", side, save_flag, lightsource, exposuretime);

	if (side < 0 || side > 1)
		return -2;

	check_scanner_set_led_ontime(side, exposuretime, lightsource);
	if (side == 0)  
		dpi_200_cis_current_a[lightsource] = exposuretime;
	else  
		dpi_200_cis_current_b[lightsource] = exposuretime;

	if (save_flag == 1) {
		paraconfig.config.unitid = CHECK_SCANUNIT;
		paraconfig.config.mode = check_scanner_ledmodetohwledmode(mode);
		paraconfig.config.hwresolution = check_dpitohwdpi[setting->resolution];
		paraconfig.config.config_tpye = CONFIG_CIS;
		paraconfig.config.sideid = side;
                rs = scanner_read_config_from_nvm(paraconfig.para, config, sizeof(dpi_200_cis_current_a));
		if (rs == -1) {
			printf("read cis config from nvm fail!\r\n");
			return rs;
		}
		config[2*lightsource] = exposuretime&0xff;
		config[2*lightsource+1] = (exposuretime&0xff00)>>8;
		rs = scanner_write_config_to_nvm(paraconfig.para, config, sizeof(dpi_200_cis_current_a));
		if (rs == -1) {
			return rs;
		}		
	}

	return 0;
}


