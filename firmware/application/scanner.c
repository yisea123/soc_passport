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
extern const int digitisers_num;
int scanmode;
scanning_control_t scanning_ctrl;
parameters_config_t paraconfig;

const unsigned char check_dpitohwdpi[] = {0xff, FPGA_REG_CIS_DPI_200, FPGA_REG_CIS_DPI_300, 0xff, FPGA_REG_CIS_DPI_600, 0xff, 0xff, 0xff};
scanning_ledmode_t ledmodetohwledmode_table[] = {
	{SCANMODE_SINGLE_GREYSCALE,	FPGA_REG_CIS_SCANMODE_EN_GREYSCALE},
	{SCANMODE_SINGLE_COLOR, 	FPGA_REG_CIS_SCANMODE_EN_RGB},
	{SCANMODE_TEN_LIGHTSOURCE,	FPGA_REG_CIS_SCANMODE_TEN_LIGHTS},
	{SCANMODE_SIX_LIGHTSOURCE,	FPGA_REG_CIS_SCANMODE_SIX_LIGHTS},
	{SCANMODE_SINGLE_COLOR_IR_UV,	FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV},
};

config_match_t check_configmode_to_para_table[] = {
	{(CHECK_SCANUNIT<<28)|(SIDE_A<<24)|(CONFIG_ADC<<20)|(FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV<<4)|FPGA_REG_CIS_DPI_200, 	NVM_DATA_ID(SCAN_DPI_200_ADC_A), 	SCAN_TYPE_200DPI_SIX},
	{(CHECK_SCANUNIT<<28)|(SIDE_A<<24)|(CONFIG_CIS<<20)|(FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV<<4)|FPGA_REG_CIS_DPI_200,	NVM_DATA_ID(SCAN_DPI_200_CIS_A),	SCAN_TYPE_200DPI_SIX},
	{(CHECK_SCANUNIT<<28)|(SIDE_A<<24)|(CONFIG_CALI<<20)|(0xffff<<4)|FPGA_REG_CIS_DPI_200, 				NVM_DATA_ID(SCAN_DPI_200_CALIBDATA),	SCAN_TYPE_300DPI_COLOR},
};

/* wm8235 internal registers */ 
const struct scan_reg_config afe_default_config[] = {
	{0x0003, 0x00, 0xff},	//Setup Reg1
	{0x0004, 0x40, 0xff},	//Setup Reg2
	{0x0005, 0x1c, 0xff},	//Setup Reg3	
	{0x0006, 0x0a, 0xff},	//VRLC control
	{0x0007, 0xa0, 0xff},	//output control
	{0x0008, 0x00, 0xff},	//LVDS control
	{0x0009, 0x00, 0xff},	//LVDS clock patten
	{0x000a, 0x00, 0xff},	//flag control 1
	{0x000b, 0x00, 0xff},	//flag control 2
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
	{0x0019, 0x05, 0xff},	//clock monitor
	{0x001a, 0x00, 0xff},	//pll control 1
	{0x001b, 0x09, 0xff},	//pll control 2
	{0x001c, 0x08, 0xff},	//pll divider control 1
//	{0x001d, 0x28, 0xff},	//pll divider control 2
	{0x0023, 0x03, 0xff},	//cycle mode control
	{0x0024, 0x80, 0xff},	//DAC IN1
	{0x0025, 0x80, 0xff},	//DAC IN2
	{0x0026, 0x80, 0xff},	//DAC IN3
	{0x0027, 0x80, 0xff},	//DAC IN4
	{0x0028, 0x80, 0xff},	//DAC IN5
	{0x0029, 0x80, 0xff},	//DAC IN6
	{0x002a, 0x80, 0xff},	//DAC IN7
	{0x002b, 0x80, 0xff},	//DAC IN8
	{0x002c, 0x80, 0xff},	//DAC IN9
	{0x002d, 0x01, 0xff},	//AGAIN IN1
	{0x002e, 0x01, 0xff},	//AGAIN IN2
	{0x002f, 0x01, 0xff},	//AGAIN IN3
	{0x0030, 0x01, 0xff},	//AGAIN IN4
	{0x0031, 0x01, 0xff},	//AGAIN IN5
	{0x0032, 0x01, 0xff},	//AGAIN IN6
	{0x0033, 0x01, 0xff},	//AGAIN IN7
	{0x0034, 0x01, 0xff},	//AGAIN IN8
	{0x0035, 0x01, 0xff},	//AGAIN IN9
	{0x0082, 0x2d, 0xff},	//RSMP rise
	{0x0083, 0x35, 0xff},	//RSMP fall
	{0x0084, 0x1c, 0xff},	//VSMP rise
	{0x0085, 0x23, 0xff},	//VSMP fall
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
	{0x00a9, 0xff, 0xff},	//TG config10
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
};
#define AFE_DEF_CONFIG_REGS	(sizeof(afe_default_config)/sizeof(struct scan_reg_config))

struct scan_reg_config check_cis_default_config[] = {
	{0x0000, 0x00FF, 0xffff},	//LIGHTSOURCE_CTRL
	{0x0008, 0x0056, 0xffff},	//T1_R
	{0x000C, 0x0056, 0xffff},	//T1_G
	{0x0010, 0x0056, 0xffff},	//T1_B
	{0x0014, 0x0056, 0xffff}, 	//T1_IR
	{0x0018, 0x0056, 0xffff}, 	//T1_IRT
	{0x001C, 0x0220, 0xffff}, 	//T1_UV
	{0x0020, 0x0056, 0xffff},	//T1_UVT
	{0x0024, 0x0220, 0xffff},	//T1_P
	{0x0028, 0x0400, 0xffff},	//T_SI_WHITE
	{0x002C, 0x0600, 0xffff},	//T_SI_IR
	{0x0030, 0x0adc, 0xffff},	//T_SI_IRT
	{0x0034, 0x1e45, 0xffff},	//T_SI_UV
	{0x0038, 0x0adc, 0xffff},	//T_SI_UVT
	{0x003C, 0x0adc, 0xffff},	//T_SI_P
	{0x0040, 0x0300, 0xffff},	//LED_Red_On_Time
	{0x0044, 0x0300, 0xffff},	//LED_Green_On_Time
	{0x0048, 0x0300, 0xffff},	//LED_Blue_On_Time
	{0x004c, 0x0300, 0xffff},	//LED_IR_On_Time
	{0x0050, 0x0300, 0xffff},	//LED_T_IR_On_Time
	{0x0054, 0x0300, 0xffff},	//LED_UV_On_Time
	{0x0058, 0x0300, 0xffff},	//LED_T_UV_On_Time
	{0x005c, 0x0adc, 0xffff},	//LED_UVP_On_Time
	{0x0078, 0x0008, 0xffff},	//shift
};

void scanner_set_sensor_default_config(struct scanunit *scanner, struct scan_reg_config *default_config)
{
	struct scanunit_config config;
	/* setup all CISs to default configuration */
	config.regcount = sizeof(check_cis_default_config)/sizeof(struct scan_reg_config);
	config.regconfig = default_config;
	scanunit_set_sensor_config(scanner, scanner->hwinfo.sensor_a, &config);
}


void scanner_set_all_digitiser_default_config(struct scanunit *scanner)
{
	struct scanunit_config config;

	/*setup all AFEs to default configuration*/ 
	config.regcount = AFE_DEF_CONFIG_REGS;
	config.regconfig = (struct scan_reg_config *)afe_default_config;
	scanunit_set_digitiser_config(scanner, 4, &config);
	scanner_afe_pll_dll_config(scanner, 4);
}

void scanner_get_digitiser_default_config(struct scanunit *scanner)
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
		scanunit_get_digitiser_config(scanner, SIDE_A, &config);
		if(afe_temp_config.value != afe_default_config[i].value)
			printf("afe reg 0x%04x: write= %02x, read= %02x\r\n", config.regconfig->address, afe_default_config[i].value, config.regconfig->value);
	}

	if(scanner->hwinfo.sides > 1)
	{
		printf("side B afe reg config:\r\n ");
		for(i=0; i<AFE_DEF_CONFIG_REGS; i++)
		{
			afe_temp_config.address = afe_default_config[i].address;
			config.regcount = 1;
			config.regconfig = &afe_temp_config;
			scanunit_get_digitiser_config(scanner, SIDE_B, &config);
			if(afe_temp_config.value != afe_default_config[i].value)
				printf("afe reg 0x%04x: write= %02x, read= %02x\r\n", config.regconfig->address, afe_default_config[i].value, config.regconfig->value);
		}
	}
}

void scanner_init_setting(struct scanunit *scanner)
{
	memset((void *)&paraconfig, 0 , sizeof(parameters_config_t));
	scanning_ctrl.hwresolution = FPGA_REG_CIS_DPI_200;
	scanning_ctrl.hwmode = FPGA_REG_CIS_SCANMODE_EN_RGB_IR_UV;

	paraconfig.config.unitid = CHECK_SCANUNIT;
	paraconfig.config.mode = scanning_ctrl.scanmode;
	paraconfig.config.hwresolution = scanning_ctrl.hwresolution;
	scanner_set_sensor_config(scanner, scanning_ctrl.hwresolution, scanning_ctrl.scanmode, SIDE_A);
	scanning_ctrl.lightsource.side_a= LIGHTSOURCE_ALL;
}

void scanner_set_sensor_config(struct scanunit *scanner, unsigned char resolution, unsigned char mode, unsigned char side)
{
	unsigned short sideen;
	scanunit_mode_ctrl_t smode;

	sideen = (side == SIDE_A)? FPGA_REG_CIS_SIDE_EN_A: FPGA_REG_CIS_SIDE_EN_B;
	smode.dpimode = resolution;
	if((mode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS)||(mode == FPGA_REG_CIS_SCANMODE_TEN_LIGHTS))
		smode.scanmode = mode;
	else
		smode.ledmode = mode;
	smode.sidemode = sideen;
	scanunit_set_scanning_mode(scanner, smode);
}


int scanner_prepare_scanning(struct scanunit *scanner, unsigned char hwresolution, unsigned char hwmode, int scanlines, int flags)
{
	int lights = 3;

	lights = (hwmode == FPGA_REG_CIS_SCANMODE_SIX_LIGHTS) ? 6 : 10;
	scanning_ctrl.linedlen = lights*scanner->hwinfo.sectinfo_a->lend*(scanner->hwinfo.colors + scanner->hwinfo.colors/3);
	if(scanner->hwinfo.sides == 1)
		scanner_set_sensor_config(scanner, hwresolution, hwmode, SIDE_A);
	else
	{
		scanner_set_sensor_config(scanner, hwresolution, hwmode, SIDE_A);
		scanner_set_sensor_config(scanner, hwresolution, hwmode, SIDE_B);
	}
	scanunit_turnoff_lights(scanner);
	scanning_ctrl.scanmode = hwmode;
	scanning_ctrl.scanflag = flags;
	scanning_ctrl.state = SCANNING_INIT;
	scanning_ctrl.linecount = 0;
	scanning_ctrl.scanlines = scanlines;
	printf("scanner_prepare_scanning: data length per line = %d, expected scanning lines = %d.\n", scanning_ctrl.linedlen, scanning_ctrl.scanlines);

	return 0;
}


int scanner_initialize()
{
	int rs;

	scanner_afe_registers_reset(&checkscanner, 4);
	scanner_set_all_digitiser_default_config(&checkscanner);
	scanner_get_digitiser_default_config(&checkscanner);

	//reset check scan unit controller 
	scanunit_reset(&checkscanner);

	rs = scanunit_get_hwinfo(&checkscanner, &checkscanner.hwinfo);
	if (rs != 0) {
		dbg_printf("Error getting check scanunit hardware information.\r\n");
		return -1;
	}  
	scanunit_turnoff_lights(&checkscanner);
	scanner_set_sensor_default_config(&checkscanner, check_cis_default_config);
	
	scanner_init_setting(&checkscanner);
	scanner_afe_power_ctrl(&checkscanner, SIDE_A, 0);
	check_scanner_setup_ajustable_config(scanning_ctrl.hwresolution, scanning_ctrl.hwmode);

	return 0;
}


int scanner_start_scanning(struct scanunit *scanner, int sidetype)
{
	if(sidetype == IAM_A_PAGE)
	{
		scanner_afe_power_ctrl(scanner, SIDE_A, 1);
		scanner->afe_info.outdataen = 0;
		scanner_cmos_output_config(scanner, SIDE_A);
	}
	else if(sidetype == IAM_B_PAGE)
	{
		scanner_afe_power_ctrl(scanner, SIDE_B, 1);
		scanner->afe_info.outdataen = 0;
		scanner_cmos_output_config(scanner, SIDE_B);
	}
	else if(sidetype == IAM_AB_PAGE)
	{
		scanner_afe_power_ctrl(scanner, SIDE_A, 1);
		scanner_afe_power_ctrl(scanner, SIDE_B, 1);
		scanner->afe_info.outdataen = 0;
		scanner_cmos_output_config(scanner, SIDE_A);
		scanner_cmos_output_config(scanner, SIDE_B);
	}
	else
		return -1;

	scanunit_start_scanning(scanner);
	return 0;
}


int scanner_stop_scanning(struct scanunit *scanner, int sidetype)
{
	scanunit_stop_scanning(scanner);
	if(sidetype == IAM_A_PAGE)
	{
		scanner->afe_info.outdataen = AFE_07_OUTPD;
		scanner_cmos_output_config(scanner, SIDE_A);
		scanner_afe_power_ctrl(scanner, SIDE_A, 0);
	}
	else if(sidetype == IAM_B_PAGE)
	{
		scanner->afe_info.outdataen = AFE_07_OUTPD;
		scanner_cmos_output_config(scanner, SIDE_B);
		scanner_afe_power_ctrl(scanner, SIDE_B, 0);
	}
	else if(sidetype == IAM_AB_PAGE)
	{
		scanner->afe_info.outdataen = AFE_07_OUTPD;
		scanner_cmos_output_config(scanner, SIDE_A);
		scanner_cmos_output_config(scanner, SIDE_B);
		scanner_afe_power_ctrl(scanner, SIDE_A, 0);
		scanner_afe_power_ctrl(scanner, SIDE_B, 0);
	}else
		return -1;

	return 0;
}


unsigned int check_scanner_ledmodetohwledmode(unsigned char ledmode)
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

unsigned int scanner_get_wraddr(struct scanunit *scanner)
{
	int wraddr = 0;
	wraddr = scanunit_get_wr_addr(scanner);
	return wraddr;
}
