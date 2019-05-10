#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "scanunit.h"
#include "scandrv.h"
#include "scanner.h"
#include "scancalib.h"
#include "scanning.h"
#include "fpga.h"
#include "fpgadrv.h"
#include "command.h"
#include "card_scanpath_drv.h"
#include "nvmdata.h"
#include "debug.h"

#define MAX_CIS_LIGHTON_TIME	(4*1000)	/* in milliseconds */

extern struct scanunit checkscanner;
int scanmode;
scanning_control_t check_scanning_ctrl;
parameters_config_t check_paraconfig;
unsigned char *scanbuffer = (unsigned char *)0xa2000000;

const unsigned char dpitohwdpi[] = {0xff, 0xff, FPGA_REG_CIS_DPI_300, 0xff, FPGA_REG_CIS_DPI_600, 0xff, 0xff, 0xff};
scanning_ledmode_t ledmodetohwledmode_table[] = {
	{IAM_GREY_MODE, FPGA_REG_CIS_SCANMODE_EN_GREYSCALE},
	{IAM_COLOR_MODE, FPGA_REG_CIS_SCANMODE_EN_RGB},
	{IAM_RED_MODE, FPGA_REG_CIS_SCANMODE_EN_RED},
	{IAM_GREEN_MODE, FPGA_REG_CIS_SCANMODE_EN_GREEN},
	{IAM_BLUE_MODE, FPGA_REG_CIS_SCANMODE_EN_BLUE},
	{IAM_INFRARED_MODE, FPGA_REG_CIS_SCANMODE_EN_IR},
	{IAM_ULTRAVIOLET_MODE, FPGA_REG_CIS_SCANMODE_EN_UV}
};

config_match_t check_configmode_to_para_table[] = {
	{(CHECK_SIDE_A<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_SIX_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_ADC_A), 			ADC_COLOR_OFFSET, 	SCAN_TYPE_200DPI_SIX},
	{(CHECK_SIDE_B<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_SIX_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_ADC_B), 			ADC_COLOR_OFFSET, 	SCAN_TYPE_200DPI_SIX},
	{(CHECK_SIDE_A<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_TEN_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_ADC_A), 			ADC_COLOR_OFFSET, 	SCAN_TYPE_200DPI_TEN},
	{(CHECK_SIDE_B<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_TEN_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_ADC_B), 			ADC_COLOR_OFFSET, 	SCAN_TYPE_200DPI_TEN},
	{(CHECK_SIDE_A<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_SIX_LIGHTS<<4)|FPGA_REG_CIS_DPI_200,	NVM_DATA_ID(SCAN_DPI_200_CIS_A),			CIS_COLOR_OFFSET,	SCAN_TYPE_200DPI_SIX},
	{(CHECK_SIDE_B<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_SIX_LIGHTS<<4)|FPGA_REG_CIS_DPI_200,	NVM_DATA_ID(SCAN_DPI_200_CIS_B),			CIS_COLOR_OFFSET, 	SCAN_TYPE_200DPI_SIX},
	{(CHECK_SIDE_A<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_TEN_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CIS_A),			CIS_COLOR_OFFSET, 	SCAN_TYPE_200DPI_TEN},
	{(CHECK_SIDE_B<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_TEN_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CIS_B),			CIS_COLOR_OFFSET, 	SCAN_TYPE_200DPI_TEN},
	{(CHECK_SIDE_A<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_SIX_LIGHTS_DARK<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_DARK_A),	0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_B<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_SIX_LIGHTS_DARK<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_DARK_B), 	0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_A<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_SIX_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_A),		0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_B<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_SIX_LIGHTS<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_B), 		0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_A<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_TEN_LIGHTS_DARK<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_DARK_A),	0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_B<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_TEN_LIGHTS_DARK<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_DARK_B), 	0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_A<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_TEN_LIGHTS<<4)|FPGA_REG_CIS_DPI_200,	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_A),		0,			SCAN_TYPE_300DPI_COLOR},
	{(CHECK_SIDE_B<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_TEN_LIGHTS<<4)|FPGA_REG_CIS_DPI_200,	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_B),		0,			SCAN_TYPE_300DPI_COLOR},

};  
unsigned char *scanbuffer;

/* wm8235 internal registers */ 
const struct scan_reg_config afe_default_config[] = {
	{0x0003, 0x00, 0xff},	//Setup Reg1
	{0x0004, 0x40, 0xff},	//Setup Reg2
	{0x0005, 0x1c, 0xff},	//Setup Reg3	
	{0x0006, 0x0a, 0xff},	//VRLC control
	{0x0007, 0xa0, 0xff},	//output control
	{0x0008, 0x00, 0xff},	//LVDS control
	{0x0009, 0x00, 0xff},	//LVDS clock patten
	{0x000a, 0x67, 0xff},	//flag control 1
	{0x000b, 0x01, 0xff},	//flag control 2
	{0x000c, 0x00, 0xff},	//flag control 3
	{0x000d, 0x00, 0xff},	//cmos drivability control 1
	{0x000e, 0x00, 0xff},	//cmos drivability control 2
	{0x000f, 0x00, 0xff},	//cmos drivability control 3	
	{0x0010, 0x00, 0xff},	//cmos drivability control 4
	{0x0011, 0x00, 0xff},	//cmos drivability control 5
	{0x0012, 0x00, 0xff},	//cmos drivability control 6
	{0x0013, 0x00, 0xff},	//cmos drivability control 7	
	{0x0014, 0x00, 0xff},	//PG config
	{0x0015, 0x00, 0xff},	//PGCODE LSB
	{0x0016, 0x00, 0xff},	//PGCODE MSB
	{0x0017, 0x00, 0xff},	//PG width1
	{0x0018, 0x00, 0xff},	//PG width2
	{0x0019, 0x00, 0xff},	//clock monitor
	{0x0023, 0x00, 0xff},	//cycle mode control
	{0x0024, 0x90, 0xff},	//DAC IN1
	{0x0025, 0x90, 0xff},	//DAC IN2
	{0x0026, 0x90, 0xff},	//DAC IN3
	{0x0027, 0x90, 0xff},	//DAC IN4
	{0x0028, 0x90, 0xff},	//DAC IN5
	{0x0029, 0x90, 0xff},	//DAC IN6
	{0x002a, 0x90, 0xff},	//DAC IN7
	{0x002b, 0x90, 0xff},	//DAC IN8	
	{0x002c, 0x90, 0xff},	//DAC IN9
	{0x002d, 0x05, 0xff},	//AGAIN IN1
	{0x002e, 0x05, 0xff},	//AGAIN IN2
	{0x002f, 0x05, 0xff},	//AGAIN IN3
	{0x0030, 0x05, 0xff},	//AGAIN IN4
	{0x0031, 0x05, 0xff},	//AGAIN IN5
	{0x0032, 0x05, 0xff},	//AGAIN IN6
	{0x0033, 0x05, 0xff},	//AGAIN IN7
	{0x0034, 0x05, 0xff},	//AGAIN IN8
	{0x0035, 0x05, 0xff},	//AGAIN IN9
	{0x0036, 0x00, 0xff},	//DGAIN IN1 LSB
	{0x0037, 0x80, 0xff},	//DGAIN IN1 MSB
	{0x0038, 0x00, 0xff},	//DGAIN IN2 LSB
	{0x0039, 0x80, 0xff},	//DGAIN IN2 MSB
	{0x003a, 0x00, 0xff},	//DGAIN IN3 LSB
	{0x003b, 0x80, 0xff},	//DGAIN IN3 MSB	
	{0x003c, 0x00, 0xff},	//DGAIN IN4 LSB
	{0x003d, 0x80, 0xff},	//DGAIN IN4 MSB
	{0x003e, 0x00, 0xff},	//DGAIN IN5 LSB
	{0x003f, 0x80, 0xff},	//DGAIN IN5 MSB
	{0x0040, 0x00, 0xff},	//DGAIN IN6 LSB
	{0x0041, 0x80, 0xff},	//DGAIN IN6 MSB
	{0x0042, 0x00, 0xff},	//DGAIN IN7 LSB
	{0x0043, 0x80, 0xff},	//DGAIN IN7 MSB
	{0x0044, 0x00, 0xff},	//DGAIN IN8 LSB
	{0x0045, 0x80, 0xff},	//DGAIN IN8 MSB
	{0x0046, 0x00, 0xff},	//DGAIN IN9 LSB
	{0x0047, 0x80, 0xff},	//DGAIN IN9 MSB
	/*0x0048, 0x00, 0xff},	//BLC IN1 target
	{0x0049, 0x00, 0xff},	//BLC IN2 target
	{0x004a, 0x00, 0xff},	//BLC IN3 target
	{0x004b, 0x00, 0xff},	//BLC IN4 target	
	{0x004c, 0x00, 0xff},	//BLC IN5 target
	{0x004d, 0x00, 0xff},	//BLC IN6 target
	{0x004e, 0x00, 0xff},	//BLC IN7 target
	{0x004f, 0x00, 0xff},	//BLC IN8 target	
	{0x0050, 0x00, 0xff},	//BLC IN9 target
	{0x0051, 0x00, 0xff},	//BLC control1
	{0x0052, 0x00, 0xff},	//BLC control2
	{0x0053, 0x00, 0xff},	//BLC control3 
	{0x0054, 0x00, 0xff},	//BLC control4
	{0x0055, 0x00, 0xff},	//BLC control5
	{0x0056, 0x00, 0xff},	//BLC control6
	{0x0057, 0x00, 0xff},	//AGC config1
	{0x0058, 0x00, 0xff},	//AGC config2
	{0x0059, 0x00, 0xff},	//AGC target IN1 LSB
	{0x005a, 0x00, 0xff},	//AGC target IN1 MSB
	{0x005b, 0x00, 0xff},	//AGC target IN2 LSB	
	{0x005c, 0x00, 0xff},	//AGC target IN2 MSB
	{0x005d, 0x00, 0xff},	//AGC target IN3 LSB
	{0x005e, 0x00, 0xff},	//AGC target IN3 MSB
	{0x005f, 0x00, 0xff},	//AGC target IN4 LSB	
	{0x0060, 0x00, 0xff},	//AGC target IN4 MSB
	{0x0061, 0x00, 0xff},	//AGC target IN5 LSB
	{0x0062, 0x00, 0xff},	//AGC target IN5 MSB
	{0x0063, 0x00, 0xff},	//AGC target IN6 LSB
	{0x0064, 0x00, 0xff},	//AGC target IN6 MSB
	{0x0065, 0x00, 0xff},	//AGC target IN7 LSB
	{0x0066, 0x00, 0xff},	//AGC target IN7 MSB
	{0x0067, 0x00, 0xff},	//AGC target IN8 LSB
	{0x0068, 0x00, 0xff},	//AGC target IN8 MSB
	{0x0069, 0x00, 0xff},	//AGC target IN9 LSB
	{0x006a, 0x00, 0xff},	//AGC target IN9 MSB
	{0x006b, 0x00, 0xff},	//AGC peak level  IN1 LSB	
	{0x006c, 0x00, 0xff},	//AGC peak level  IN1 MSB
	{0x006d, 0x00, 0xff},	//AGC peak level  IN2 LSB
	{0x006e, 0x00, 0xff},	//AGC peak level  IN2 MSB
	{0x006f, 0x00, 0xff},	//AGC peak level  IN3 LSB	
	{0x0070, 0x00, 0xff},	//AGC peak level  IN3 MSB
	{0x0071, 0x00, 0xff},	//AGC peak level  IN4 LSB
	{0x0072, 0x00, 0xff},	//AGC peak level  IN4 MSB
	{0x0073, 0x00, 0xff},	//AGC peak level  IN5 LSB
	{0x0074, 0x00, 0xff},	//AGC peak level  IN5 MSB
	{0x0075, 0x00, 0xff},	//AGC peak level  IN6 LSB
	{0x0076, 0x00, 0xff},	//AGC peak level  IN6 MSB
	{0x0077, 0x00, 0xff},	//AGC peak level  IN7 LSB
	{0x0078, 0x00, 0xff},	//AGC peak level  IN7 MSB
	{0x0079, 0x00, 0xff},	//AGC peak level  IN8 LSB
	{0x007a, 0x00, 0xff},	//AGC peak level  IN8 MSB
	{0x007b, 0x00, 0xff},	//AGC peak level  IN9 LSB	
	{0x007c, 0x00, 0xff},	//AGC peak level  IN9 MSB*/
	{0x0082, 0x2d, 0xff},	//RSMP rise
	{0x0083, 0x35, 0xff},	//RSMP fall
	{0x0084, 0x0f, 0xff},	//VSMP rise
	{0x0085, 0x17, 0xff},	//VSMP fall
	{0x0086, 0x37, 0xff},	//TGCKO rise
	{0x0087, 0x0d, 0xff},	//CLK1 rise
	{0x0088, 0x1d, 0xff},	//CLK1 fall
	{0x0089, 0x19, 0xff},	//CLK2 rise
	{0x008a, 0x28, 0xff},	//CLK2 fall
	{0x008b, 0x19, 0xff},	//CLK3 rise	
	{0x008c, 0x28, 0xff},	//CLK3 fall
	{0x008d, 0x19, 0xff},	//CLK4 rise
	{0x008e, 0x28, 0xff},	//CLK4 fall
	{0x008f, 0x19, 0xff},	//CLK5 rise	
	{0x0090, 0x28, 0xff},	//CLK5 fall
	{0x0091, 0x19, 0xff},	//CLK6 rise
	{0x0092, 0x28, 0xff},	//CLK6 fall
	{0x00a0, 0x01, 0xff},	//TG config1
	{0x00a1, 0xc0, 0xff},	//TG config2
	{0x00a2, 0x0f, 0xff},	//TG config3
	{0x00a3, 0x00, 0xff},	//TG config4
	{0x00a4, 0x00, 0xff},	//TG config5
	{0x00a5, 0xff, 0xff},	//TG config6
	{0x00a6, 0x00, 0xff},	//TG config7
	{0x00a7, 0x00, 0xff},	//TG config8
	{0x00a8, 0x00, 0xff},	//TG config9
	{0x00a9, 0x3f, 0xff},	//TG config10
	{0x00aa, 0x00, 0xff},	//TG config11
	{0x00ab, 0xcb, 0xff},	//TG config12	
	{0x00ac, 0x9a, 0xff},	//TG config13
	{0x00ad, 0x08, 0xff},	//TG config14
	{0x00ae, 0x00, 0xff},	//TG config15
	{0x00af, 0x00, 0xff},	//TG config16	
	{0x00b0, 0x00, 0xff},	//TG config17
	{0x00b1, 0x00, 0xff},	//TG config18
	{0x00b2, 0x00, 0xff},	//TG config19
	{0x00b3, 0x00, 0xff},	//TG config20
	{0x00b4, 0x00, 0xff},	//TG config21
	{0x00b5, 0x24, 0xff},	//TG config22
	{0x00b6, 0x71, 0xff},	//TG config23
	{0x00b7, 0x27, 0xff},	//TG config24
	{0x00b8, 0x00, 0xff},	//TG config25
	{0x00b9, 0x00, 0xff},	//clamp enable rise LSB
	{0x00ba, 0x00, 0xff},	//clamp enable rise MSB
	{0x00bb, 0x00, 0xff},	//clamp enable fall LSB	
	{0x00bc, 0x00, 0xff},	//clamp enable fall MSB
	{0x00bd, 0x00, 0xff},	//OB start LSB
	{0x00be, 0x00, 0xff},	//OB start MSB
	{0x00bf, 0x00, 0xff},	//peak_det rise LSB
	{0x00c0, 0x00, 0xff},	//peak_det rise MSB
	{0x00c1, 0x00, 0xff},	//peak_det fall LSB
	{0x00c2, 0x00, 0xff},	//peak_det fall MSB
	{0x00c3, 0x00, 0xff},	//MASK pulse 1 rise LSB
	{0x00c4, 0x00, 0xff},	//MASK pulse 1 rise MSB
	{0x00c5, 0x00, 0xff},	//MASK pulse 1 fall LSB
	{0x00c6, 0x00, 0xff},	//MASK pulse 1 fall MSB
	{0x00c7, 0x00, 0xff},	//MASK pulse 2 rise LSB
	{0x00c8, 0x00, 0xff},	//MASK pulse 2 rise MSB
	{0x00c9, 0x00, 0xff},	//MASK pulse 2 fall LSB
	{0x00ca, 0x00, 0xff},	//MASK pulse 2 fall MSB
	{0x00cb, 0x00, 0xff},	//MASK pulse 3 rise LSB	
	{0x00cc, 0x00, 0xff},	//MASK pulse 3 rise MSB
	{0x00cd, 0x00, 0xff},	//MASK pulse 3 fall LSB
	{0x00ce, 0x00, 0xff},	//MASK pulse 3 fall MSB
	{0x00cf, 0x00, 0xff},	//toggle point 0 LSB	
	{0x00d0, 0x80, 0xff},	//toggle point 0 MSB
	{0x00d1, 0x04, 0xff},	//toggle point 1 LSB
	{0x00d2, 0x80, 0xff},	//toggle point 1 MSB
	{0x00d3, 0x0a, 0xff},	//toggle point 2 LSB
	{0x00d4, 0x80, 0xff},	//toggle point 2 MSB
	{0x00d5, 0x28, 0xff},	//toggle point 3 LSB
	{0x00d6, 0x80, 0xff},	//toggle point 3 MSB
	{0x00d7, 0x39, 0xff},	//toggle point 4 LSB
	{0x00d8, 0x80, 0xff},	//toggle point 4 MSB
	{0x00d9, 0x3c, 0xff},	//toggle point 5 LSB
	{0x00da, 0x80, 0xff},	//toggle point 5 MSB
	{0x00db, 0x3d, 0xff},	//toggle point 6 LSB	
	{0x00dc, 0x80, 0xff},	//toggle point 6 MSB
	{0x00dd, 0x51, 0xff},	//toggle point 7 LSB
	{0x00de, 0x80, 0xff},	//toggle point 7 MSB
	{0x00df, 0xcb, 0xff},	//toggle point 8 LSB
	{0x00e0, 0x80, 0xff},	//toggle point 8 MSB
	{0x00e1, 0x90, 0xff},	//toggle point 9 LSB
	{0x00e2, 0x81, 0xff},	//toggle point 9 MSB
	{0x00e3, 0x58, 0xff},	//toggle point 10 LSB
	{0x00e4, 0x00, 0xff},	//toggle point 10 MSB
	{0x00e5, 0x00, 0xff},	//toggle point 11 LSB
	{0x00e6, 0x00, 0xff},	//toggle point 11 MSB
	{0x00e7, 0x00, 0xff},	//toggle point 12 LSB
	{0x00e8, 0x00, 0xff},	//toggle point 12 MSB
	{0x00e9, 0x00, 0xff},	//toggle point 13 LSB
	{0x00ea, 0x00, 0xff},	//toggle point 13 MSB
	{0x00eb, 0x00, 0xff},	//toggle point 14 LSB	
	{0x00ec, 0x00, 0xff},	//toggle point 14 MSB
	{0x00ed, 0x00, 0xff},	//toggle point 15 LSB
	{0x00ee, 0x00, 0xff},	//toggle point 15 MSB
	{0x00ef, 0x00, 0xff},	//toggle point 16 LSB
	{0x00f0, 0x00, 0xff},	//toggle point 16 MSB
	{0x00f1, 0x00, 0xff},	//toggle point 17 LSB
	{0x00f2, 0x00, 0xff},	//toggle point 17 MSB
	{0x00f3, 0x00, 0xff},	//toggle point 18 LSB
	{0x00f4, 0x00, 0xff},	//toggle point 18 MSB
	{0x00f5, 0x00, 0xff},	//toggle point 19 LSB
	{0x00f6, 0x00, 0xff},	//toggle point 19 MSB
	{0x00f7, 0x00, 0xff},	//toggle point 20 LSB
	{0x00f8, 0x00, 0xff},	//toggle point 20 MSB
	{0x00f9, 0x00, 0xff},	//toggle point 21 LSB
	{0x00fa, 0x00, 0xff},	//toggle point 21 MSB
	{0x00fb, 0x00, 0xff},	//toggle point 22 LSB	
	{0x00fc, 0x00, 0xff},	//toggle point 22 MSB
	{0x00fd, 0x00, 0xff},	//toggle point 23 LSB
	{0x00fe, 0x00, 0xff},	//toggle point 23 MSB
	{0x00ff, 0x00, 0xff},	//toggle point 24 LSB
	{0x0100, 0x00, 0xff},	//toggle point 24 MSB
	{0x0101, 0x00, 0xff},	//toggle point 25 LSB
	{0x0102, 0x00, 0xff},	//toggle point 25 MSB
	{0x0103, 0x00, 0xff},	//toggle point 26 LSB
	{0x0104, 0x00, 0xff},	//toggle point 26 MSB
	{0x0105, 0x00, 0xff},	//toggle point 27 LSB
	{0x0106, 0x00, 0xff},	//toggle point 27 MSB
	{0x0107, 0x00, 0xff},	//toggle point 28 LSB
	{0x0108, 0x00, 0xff},	//toggle point 28 MSB
	{0x0109, 0x00, 0xff},	//toggle point 29 LSB
	{0x010a, 0x00, 0xff},	//toggle point 29 MSB
	{0x010b, 0x00, 0xff},	//toggle point 30 LSB	
	{0x010c, 0x00, 0xff},	//toggle point 30 MSB
	{0x010d, 0x00, 0xff},	//toggle point 31 LSB
	{0x010e, 0x00, 0xff},	//toggle point 31 MSB
	{0x010f, 0xff, 0xff},	//polarity setting of T1 1
	{0x0110, 0xff, 0xff},	//polarity setting of T1 2
	{0x0111, 0xff, 0xff},	//polarity setting of T1 3
	{0x0112, 0xff, 0xff},	//polarity setting of T1 4
	{0x0113, 0xff, 0xff},	//polarity setting of T2 1
	{0x0114, 0xff, 0xff},	//polarity setting of T2 2
	{0x0115, 0xff, 0xff},	//polarity setting of T2 3
	{0x0116, 0xff, 0xff},	//polarity setting of T2 4
	{0x0117, 0xf8, 0xff},	//polarity setting of P0 1
	{0x0118, 0x03, 0xff},	//polarity setting of P0 2
	{0x0119, 0x00, 0xff},	//polarity setting of P0 3
	{0x011a, 0xc4, 0xff},	//polarity setting of P0 4
	{0x011b, 0x00, 0xff},	//polarity setting of P1 1	
	{0x011c, 0x00, 0xff},	//polarity setting of P1 2
	{0x011d, 0x00, 0xff},	//polarity setting of P1 4
	{0x011e, 0x00, 0xff},	//polarity setting of P1 4
	{0x011f, 0xe0, 0xff},	//polarity setting of P2 1
	{0x0120, 0x00, 0xff},	//polarity setting of P2 2	
	{0x0121, 0x00, 0xff},	//polarity setting of P2 3
	{0x0122, 0x00, 0xff},	//polarity setting of P2 4
	{0x0123, 0x01, 0xff},	//polarity setting of P3 1
	{0x0124, 0x00, 0xff},	//polarity setting of P3 2
	{0x0125, 0x00, 0xff},	//polarity setting of P3 3
	{0x0126, 0x00, 0xff},	//polarity setting of P3 4
	{0x0127, 0x00, 0xff},	//polarity setting of P4 1
	{0x0128, 0x00, 0xff},	//polarity setting of P4 2
	{0x0129, 0x00, 0xff},	//polarity setting of P4 3
	{0x012a, 0x00, 0xff},	//polarity setting of P4 4
	{0x012b, 0x00, 0xff},	//polarity setting of P5 1	
	{0x012c, 0x00, 0xff},	//polarity setting of P5 2
	{0x012d, 0x00, 0xff},	//polarity setting of P5 3
	{0x012e, 0x00, 0xff},	//polarity setting of P5 4
	{0x012f, 0x00, 0xff},	//polarity setting of P6 1
	{0x0130, 0x00, 0xff},	//polarity setting of P6 2	
	{0x0131, 0x00, 0xff},	//polarity setting of P6 3
	{0x0132, 0x00, 0xff},	//polarity setting of P6 4
	{0x0133, 0x00, 0xff},	//polarity setting of P7 1
	{0x0134, 0x00, 0xff},	//polarity setting of P7 2
	{0x0135, 0x00, 0xff},	//polarity setting of P7 3
	{0x0136, 0x00, 0xff},	//polarity setting of P7 4
	{0x01b0, 0x00, 0xff},	//user access control
	{0x01b4, 0x10, 0xff},	//LDO2 control
	{0x01c0, 0x00, 0xff},	//USER_KEY2
};
#define AFE_DEF_CONFIG_REGS	(sizeof(afe_default_config)/sizeof(struct scan_reg_config))

const struct scan_reg_config cis_default_config[] = {
	{0x0000, 0x00FF, 0xffff},	//LIGHTSOURCE_CTRL
	{0x0008, 0x0056, 0xffff},	//T1_R
	{0x000C, 0x0056, 0xffff},	//T1_G
	{0x0010, 0x0056, 0xffff},	//T1_B
	{0x0014, 0x0056, 0xffff}, 	//T1_IR
	{0x0018, 0x0056, 0xffff}, 	//T1_IRT
	{0x001C, 0x0220, 0xffff}, 	//T1_UV
	{0x0020, 0x0056, 0xffff},	//T1_UVT
	{0x0024, 0x0220, 0xffff},	//T1_P
	{0x0028, 0x0220, 0xffff},	//T_SI_WHITE
	{0x002C, 0x0220, 0xffff},	//T_SI_IR
	{0x0030, 0x0220, 0xffff},	//T_SI_IRT
	{0x0034, 0x0640, 0xffff},	//T_SI_UV
	{0x0038, 0x0640, 0xffff},	//T_SI_UVT
	{0x003C, 0x0660, 0xffff},	//T_SI_P
	{0x0040, 0x0300, 0xffff},	//LED_Red_On_Time
	{0x0044, 0x0300, 0xffff},	//LED_Green_On_Time
	{0x0048, 0x0300, 0xffff},	//LED_Blue_On_Time
	{0x004c, 0x0300, 0xffff},	//LED_IR_On_Time
	{0x0050, 0x0300, 0xffff},	//LED_T_IR_On_Time
	{0x0054, 0x0100, 0xffff},	//LED_UV_On_Time
	{0x0058, 0x0100, 0xffff},	//LED_T_UV_On_Time
	{0x005c, 0x5ea, 0xffff},	//LED_UVP_On_Time
};
#define CIS_DEF_CONFIG_REGS	(sizeof(cis_default_config)/sizeof(struct scan_reg_config))

void check_scanner_set_sensor_default_config()
{
	struct scanunit_config config;
	
	/* setup all CISs to default configuration */
	config.regcount = CIS_DEF_CONFIG_REGS;
	config.regconfig = (struct scan_reg_config *)cis_default_config;
	scanunit_set_sensor_config(&checkscanner, checkscanner.hwinfo.sensor_a, &config);
}

void scanner_set_all_digitiser_default_config()
{
	struct scanunit_config config;

	/*setup all AFEs to default configuration*/ 
	config.regcount = AFE_DEF_CONFIG_REGS;
	config.regconfig = (struct scan_reg_config *)afe_default_config;
	scanunit_set_digitiser_config(&checkscanner, 4, &config);	
	scanner_afe_pll_dll_config(&checkscanner, 4);
}

void check_scanner_get_digitiser_default_config()
{
	struct scanunit_config  config;
	struct scan_reg_config afe_temp_config = {0x0000, 0x00, 0xff};
	int i;
	
	printf("check side A afe reg config:\r\n ");
	for(i=0; i<AFE_DEF_CONFIG_REGS; i++)
	{
		afe_temp_config.address = afe_default_config[i].address;
		config.regcount = 1;
		config.regconfig = &afe_temp_config;
		scanunit_get_digitiser_config(&checkscanner, checkscanner.hwinfo.sectinfo_a->digitiser_id, &config);
		if(afe_temp_config.value != afe_default_config[i].value)
			printf("afe reg 0x%04x: write= %02x, read= %02x\r\n", config.regconfig->address, afe_default_config[i].value, config.regconfig->value);
	}
}

void ocr_scanner_get_digitiser_default_config()
{
	struct scanunit_config  config;
	struct scan_reg_config afe_temp_config = {0x0000, 0x00, 0xff};
	int i;
	
	printf("ocr side A afe reg config:\r\n ");
	for(i=0; i<AFE_DEF_CONFIG_REGS; i++)
	{
		afe_temp_config.address = afe_default_config[i].address;
		config.regcount = 1;
		config.regconfig = &afe_temp_config;
		scanunit_get_digitiser_config(&checkscanner, checkscanner.hwinfo.sectinfo_a->digitiser_id, &config);
		if(afe_temp_config.value != afe_default_config[i].value)
			printf("afe reg 0x%04x: write= %02x, read= %02x\r\n", config.regconfig->address, afe_default_config[i].value, config.regconfig->value);
	}
	printf("ocr side B afe reg config:\r\n ");
	for(i=0; i<AFE_DEF_CONFIG_REGS; i++)
	{
		afe_temp_config.address = afe_default_config[i].address;
		config.regcount = 1;
		config.regconfig = &afe_temp_config;
		scanunit_get_digitiser_config(&checkscanner, checkscanner.hwinfo.sectinfo_b->digitiser_id, &config);
		if(afe_temp_config.value != afe_default_config[i].value)
			printf("afe reg 0x%04x: write= %02x, read= %02x\r\n", config.regconfig->address, afe_default_config[i].value, config.regconfig->value);
	} 
}

void check_scanner_init_setting()
{
	memset((void *)&check_paraconfig, 0 , sizeof(parameters_config_t));
	check_scanning_ctrl.hwresolution = FPGA_REG_CIS_DPI_300;
	check_scanning_ctrl.hwmode = FPGA_REG_CIS_SCANMODE_EN_RGB;
	check_paraconfig.config.mode = check_scanning_ctrl.scanmode;
	check_paraconfig.config.hwresolution = check_scanning_ctrl.hwresolution;
	check_scanner_set_sensor_config(check_scanning_ctrl.hwresolution, check_scanning_ctrl.scanmode, CHECK_SIDE_A);
	check_scanning_ctrl.lightsource.side_a= LIGHTSOURCE_ALL;
}

void check_scanner_set_sensor_config(unsigned char resolution, unsigned char mode, unsigned char side)
{
	unsigned char sideen;

	sideen = (side == CHECK_SIDE_A)? BIT(0): BIT(1);
	scanunit_mode_ctrl_t smode;
	smode.dpimode = resolution;
	smode.scanmode = mode;
	smode.sidemode = sideen;
	scanunit_set_scanning_mode(&checkscanner, smode);
}

int scanner_prepare_scanning(int unitid, unsigned char resolution, unsigned char mode, unsigned char *buffer, int scanlines, int flags)
{
	int lights = 10;
	if(unitid == CHECK_SCANUNIT)
	{
		lights = (mode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS) ? 6 : 10;
		check_scanning_ctrl.linedlen = lights*CHECK_SCANLINE_DATA_CNT;
		check_scanner_set_sensor_config(resolution, mode, CHECK_SIDE_A);
		check_scanner_set_sensor_config(resolution, mode, CHECK_SIDE_B);
		scanunit_turnoff_lights(&checkscanner);
		check_scanning_ctrl.scanmode = mode;
		check_scanning_ctrl.scanflag = flags;
		check_scanning_ctrl.buffer = buffer;
		check_scanning_ctrl.buffptr = buffer;
		check_scanning_ctrl.state = SCANNING_INIT;
		check_scanning_ctrl.linecount = 0;
		check_scanning_ctrl.scanlines = scanlines;
		printf("check scanner_prepare_scanning: data length per line = %d, expected scanning lines = %d.\n", check_scanning_ctrl.linedlen, check_scanning_ctrl.scanlines);
	}

	return 0;
}


int scanner_initialize()
{
	int rs;

	scanner_afe_registers_reset(&checkscanner, MAX_DIGITISER_NUM);
	scanner_set_all_digitiser_default_config();
	check_scanner_get_digitiser_default_config();

	//reset check scan unit controller 
	scanunit_reset(&checkscanner);

	rs = scanunit_get_hwinfo(&checkscanner, &checkscanner.hwinfo);
	if (rs != 0) {
		dbg_printf("Error getting check scanunit hardware information.\r\n");
		return -1;
	}  
	scanunit_turnoff_lights(&checkscanner);
	check_scanner_set_sensor_default_config();
	
	check_scanner_init_setting();
        scanner_cmos_output_config(&checkscanner, checkscanner.hwinfo.sectinfo_a->digitiser_id);
//	check_scanner_setup_ajustable_config(check_scanning_ctrl.hwresolution, check_scanning_ctrl.hwmode);

	return 0;
}

unsigned char check_scanner_ledmodetohwledmode(unsigned char ledmode)
{
	int i;
	for (i = 0; i < sizeof(ledmodetohwledmode_table)/sizeof(scanning_ledmode_t); i++) {
		if (ledmodetohwledmode_table[i].ledmode == ledmode) {
			return ledmodetohwledmode_table[i].hwledmode;
		}
	}
	return 0;
}

unsigned char check_scanner_modetospeedtype(int para)
{
	int i;
	for (i = 0; i < sizeof(check_configmode_to_para_table)/sizeof(config_match_t); i++) {
		if (check_configmode_to_para_table[i].para == para) {
			return check_configmode_to_para_table[i].speedtype;
		}
	}
	return 0;
}

unsigned char check_scanner_modetonvmid(int para)
{
	int i;
	for (i = 0; i < sizeof(check_configmode_to_para_table)/sizeof(config_match_t); i++) {
		if (check_configmode_to_para_table[i].para == para) {
			return check_configmode_to_para_table[i].nvm_id;
		}
	}
	return 0;
}

unsigned int check_scanner_configmode_to_nvmoffset(int para)
{
	int i;
	for (i = 0; i < sizeof(check_configmode_to_para_table)/sizeof(config_match_t); i++) {
		if (check_configmode_to_para_table[i].para == para) {
			return check_configmode_to_para_table[i].offset;
		}
	}
	return 0;
}

