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
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"


struct scancalib_data_buf buf;

struct scan_reg_config check_afe_offset_config[] = {
	{AFE_REG_DAC_IN1, 0x00, 0xff},		//DAC IN1
	{AFE_REG_DAC_IN2, 0x00, 0xff},		//DAC IN2
	{AFE_REG_DAC_IN3, 0x00, 0xff},		//DAC IN3
	{AFE_REG_DAC_IN4, 0x00, 0xff},		//DAC IN4
	{AFE_REG_DAC_IN5, 0x00, 0xff},		//DAC IN5
	{AFE_REG_DAC_IN6, 0x00, 0xff},		//DAC IN6
	{AFE_REG_DAC_IN7, 0x00, 0xff},		//DAC IN7
	{AFE_REG_DAC_IN8, 0x00, 0xff},		//DAC IN8
	{AFE_REG_DAC_IN9, 0x00, 0xff},		//DAC IN9
};
#define AFE_OFFSET_CONFIG_REGS	(sizeof(check_afe_offset_config)/sizeof(struct scan_reg_config))


int max_value_get(int *distrdata, int size)
{
	int i;		

	for(i = size-1; i > 0; i--)
	{
		if(distrdata[i] != 0)
			return i;
	}
	return i;
}


int set_cis_ontime(struct scanunit *scanner, int id, struct scan_reg_config *light_config, int regcount)
{
	struct scanunit_config config;
	config.regconfig = light_config;
	config.regcount = regcount;
	scanunit_set_sensor_config(scanner, id, &config);
	return 0;
}


void scanner_set_cis_lightsource(struct scanunit *scanner, int cisid, unsigned short lightsource)
{
	struct scanunit_config config;
	struct scan_reg_config regconfig;


	regconfig.address = REG_ADDR_LIGHTSOURCE;
	regconfig.mask = REG_MASK_LIGHTSOURCE;
	regconfig.value = lightsource;

	config.regcount = 1;
	config.regconfig = &regconfig;
	scanunit_set_sensor_config(scanner, cisid, &config);
}



int scanner_fill_image_data(unsigned char *rawdata, int totalpixels, int linepixels, int start_pixel, int width, unsigned int pattern)
{
	int i, j, lines;
	unsigned int *ptr = (unsigned int *)rawdata;

	if (rawdata==NULL || totalpixels<0 || totalpixels<linepixels ||
		start_pixel<0 || start_pixel >linepixels || width<0 || width > linepixels)
		return -1;

	lines = totalpixels/linepixels;
	for (i=0; i<lines; i++){
		for (j=0; j<width; j++)
		{
			unsigned int pdata = ptr[start_pixel + j];
			pdata = (pdata & 0xff000000)|(pattern & 0x00ffffff);
			ptr[start_pixel + j] = pdata;
		}
		ptr += linepixels;
	}
	return 0;
}


int scanner_write_config_to_nvm(int setting, unsigned char *config, int len)
{
	unsigned int id;
	int rs;

	id = check_scanner_modetonvmid(setting);
	printf("scanner_write_config_to_nvm:id = %d, setting=%x\r\n", id, setting);

	if(id != -1) {
		rs = nvm_write(id, config, len);
		if (rs != -1) {
			return 0;
		}
	}
	return -1;
}

int scanner_read_config_from_nvm(int setting, unsigned char *config, int len)
{
	unsigned int id;
	int rs;

	id = check_scanner_modetonvmid(setting);
	printf("scanner_read_config_from_nvm:id = %d, setting=%x\r\n", id, setting);
	if(id != -1) {
		rs = nvm_read(id, config, len);
		if (rs != -1) {
			return 0;
		}
	}
	return -1;
}


void scanner_afe_setvalue(struct scanunit *scanner, int device, int addr, unsigned char value)
{
	struct scanunit_config  config;
	struct scan_reg_config afe_temp_config = {0x0000, 0x00, 0xff};

	afe_temp_config.address = addr;
	afe_temp_config.value = value;
	config.regcount = 1;
	config.regconfig = &afe_temp_config;
	scanunit_set_digitiser_config(scanner, device, &config);
}


void scanner_afe_getvalue(struct scanunit *scanner, int device, int addr, unsigned char *value)
{
	struct scanunit_config  config;
	struct scan_reg_config afe_temp_config = {0x0000, 0x00, 0xff};

	afe_temp_config.address = addr;
	config.regcount = 1;
	config.regconfig = &afe_temp_config;
	scanunit_get_digitiser_config(scanner, device, &config);
	*value = afe_temp_config.value;
}


void scanner_afe_update_lbits(struct scanunit *scanner, int device, int addr, unsigned char mask, unsigned char value)
{
	unsigned char temp = 0;

	scanner_afe_getvalue(scanner, device, addr, &temp);
	temp = (temp & ~mask)|(value & mask);//value = mask
	scanner_afe_setvalue(scanner, device, addr, temp);
}


int scanner_set_afe_offset(struct scanunit *scanner, int device, unsigned int offset)
{
	struct scanunit_config config;
	int i;

	for(i=0; i<AFE_OFFSET_CONFIG_REGS; i++)
		check_afe_offset_config[i].value = offset;

	config.regconfig = check_afe_offset_config;
	config.regcount = AFE_OFFSET_CONFIG_REGS;
	scanunit_set_digitiser_config(scanner, device, &config);

	return 0;
}


int scanner_set_afe_gainmsb_config(struct scanunit *scanner, int device, unsigned int gainmsb)
{
	struct scanunit_config config;

	struct scan_reg_config afe_gainmsb_config[9] = {
		{AFE_REG_AGAIN_IN1, 0x00, 0xff},		//PGA Gain AGAIN IN1
		{AFE_REG_AGAIN_IN2, 0x00, 0xff},		//PGA Gain AGAIN IN2
		{AFE_REG_AGAIN_IN3, 0x00, 0xff},		//PGA Gain AGAIN IN3
		{AFE_REG_AGAIN_IN4, 0x00, 0xff},		//PGA Gain AGAIN IN4
		{AFE_REG_AGAIN_IN5, 0x00, 0xff},		//PGA Gain AGAIN IN5
		{AFE_REG_AGAIN_IN6, 0x00, 0xff},		//PGA Gain AGAIN IN6
		{AFE_REG_AGAIN_IN7, 0x00, 0xff},		//PGA Gain AGAIN IN7
		{AFE_REG_AGAIN_IN8, 0x00, 0xff},		//PGA Gain AGAIN IN8
		{AFE_REG_AGAIN_IN9, 0x00, 0xff},		//PGA Gain AGAIN IN9
	};

	afe_gainmsb_config[0].value = gainmsb & 0x1f;
	afe_gainmsb_config[1].value = gainmsb & 0x1f;
	afe_gainmsb_config[2].value = gainmsb & 0x1f;
	afe_gainmsb_config[3].value = gainmsb & 0x1f;
	afe_gainmsb_config[4].value = gainmsb & 0x1f;
	afe_gainmsb_config[5].value = gainmsb & 0x1f;
	afe_gainmsb_config[6].value = gainmsb & 0x1f;
	afe_gainmsb_config[7].value = gainmsb & 0x1f;
	afe_gainmsb_config[8].value = gainmsb & 0x1f;

	config.regconfig = (struct scan_reg_config *)afe_gainmsb_config;
	config.regcount = (sizeof(afe_gainmsb_config)/sizeof(struct scan_reg_config));
	scanunit_set_digitiser_config(scanner, device, &config);
	return 0;
}

int scanner_get_afe_gainmsb_config(struct scanunit *scanner, int device, unsigned int *gainmsb)
{
	struct scanunit_config config;
	struct scan_reg_config afe_gainmsb_config[1] = {
		{AFE_REG_AGAIN_IN1, 0x00, 0xff},		//PGA Gain AGAIN IN
	};

	config.regconfig = (struct scan_reg_config *)afe_gainmsb_config;
	config.regcount = 1;
	scanunit_get_digitiser_config(scanner, device, &config);

	*gainmsb = afe_gainmsb_config[0].value;
	return 0;
}

void scanner_set_afe_vsmpup_config(struct scanunit *scanner, int device, unsigned int vsmpup, unsigned int vsmpdown)
{
	struct scanunit_config config;
	struct scan_reg_config afe_vsmpup_config[2] = {
		{AFE_REG_VSMP_RISE, 0x0f, 0xff},		//VSMPUP
		{AFE_REG_VSMP_FALL, 0x17, 0xff},		//VSMPDOWN
	};

	afe_vsmpup_config[0].value = vsmpup;
	afe_vsmpup_config[1].value = vsmpdown;

	config.regconfig = (struct scan_reg_config *)afe_vsmpup_config; 
	config.regcount = 2;
	scanunit_set_digitiser_config(scanner, device, &config);
	scanner_afe_system_reset(scanner, device);
}

void scanner_set_afe_siginac_config(struct scanunit *scanner, int device, unsigned int siginac, unsigned int sigacti)
{
	struct scanunit_config config;
	struct scan_reg_config afe_siginac_config[2] = {
		{0x0008, 0x00, 0xff},		//inactive_pixel
		{0x000c, 0x00, 0xff},		//active_pixel
	};

	afe_siginac_config[0].value = siginac;
	afe_siginac_config[1].value = sigacti;

	config.regconfig = (struct scan_reg_config *)afe_siginac_config; 
	config.regcount = 2;
	scanunit_set_digitiser_config(scanner, device, &config);
	scanner_afe_system_reset(scanner, device);
}


void scanner_set_afe_clock_monitor_source(struct scanunit *scanner, int device, unsigned int monclk)
{
	scanner_afe_update_lbits(scanner, device, AFE_REG_CLK_MONITOR, AFE_19_MONCLK_MASK, monclk);
	scanner_afe_system_reset(scanner, device);
}


void scanner_afe_registers_reset(struct scanunit *scanner, int device)
{
	scanner_afe_setvalue(scanner, device, AFE_REG_SOFT_RESET, 0x34);
}


void scanner_afe_system_reset(struct scanunit *scanner, int device)
{
	scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PD, AFE_03_PD);
	scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PDMD, AFE_03_PDMD);

	vTaskDelay(1/portTICK_RATE_MS);//delay 1ms
	scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PD, 0);
	scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PDMD, 0);

}

void scanner_afe_pll_dll_config(struct scanunit *scanner, int device)
{
	scanner_afe_setvalue(scanner, device, AFE_REG_PLL_CTRL1, scanner->afe_info.pllctrl1);
	scanner_afe_setvalue(scanner, device, AFE_REG_PLL_CTRL2, scanner->afe_info.pllctrl2);
	scanner_afe_update_lbits(scanner, device, AFE_REG_PLL_CTRL1, AFE_1C_PLLEXDIV_SEL_MASK, (scanner->afe_info.pll_exdiv_sel)<<AFE_1C_PLLEXDIV_SEL_SHIFT);
 	scanner_afe_setvalue(scanner, device, AFE_REG_DLL_CFG1, scanner->afe_info.dllconfig1);
	scanner_afe_setvalue(scanner, device, AFE_REG_DLL_CFG2, scanner->afe_info.dllconfig2);
	//after set pll need reset as follows
	scanner_afe_system_reset(scanner, device);
}

void scanner_afe_power_ctrl(struct scanunit *scanner, int device, int on_off)
{
	unsigned char setupreg1_val;
	if(on_off == 0)
	{
		scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PD, AFE_03_PD);
		scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PDMD, AFE_03_PDMD);

	}else
	{
		scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PD, 0);
		scanner_afe_update_lbits(scanner, device, AFE_REG_SETUP_REG1, AFE_03_PDMD, 0);
	}
}


void scanner_cmos_output_config(struct scanunit *scanner, int device)
{
	if(scanner->afe_info.outdataen == 0)
	{
		//select cmosmode
		scanner_afe_update_lbits(scanner, device, AFE_REG_OUTPUT_CTRL, AFE_07_CMOSMODE_EN, AFE_07_CMOSMODE_EN);
		if(scanner->afe_info.flag_en)
		{
			//output drv control
			scanner_afe_update_lbits(scanner, device, AFE_REG_CMOS_DRV_CTRL1, AFE_0D_DRV_CTRL, 0);
			scanner_afe_update_lbits(scanner, device, AFE_REG_CMOS_DRV_CTRL1, AFE_0D_OPDRV_MASK, 5); //5mA
			if(scanner->afe_info.master_mode_en)
			{
				//select TG mode
				scanner_afe_update_lbits(scanner, device, AFE_REG_TG_CFG1, AFE_A0_TGMD_MASK, AFE_A0_TGMD_MASTER); //tg master
				//set line length
				scanner_afe_setvalue(scanner, device, AFE_REG_TG_CFG2, LSB_8(scanner->afe_info.linelength));
				scanner_afe_setvalue(scanner, device, AFE_REG_TG_CFG3, MSB_7(scanner->afe_info.linelength));
			}else
				scanner_afe_update_lbits(scanner, device, AFE_REG_TG_CFG1, AFE_A0_TGMD_MASK, AFE_A0_TGMD_SLAVE); //tg slaver
			//TG enable
			scanner_afe_update_lbits(scanner, device, AFE_REG_TG_CFG1, AFE_A0_TG_EN, AFE_A0_TG_EN);
			//set flag signal selection
			scanner_afe_update_lbits(scanner, device, AFE_REG_TG_CFG21, AFE_B4_SEL_FLAG_MASK, scanner->afe_info.flagsig_sel);
			if(scanner->afe_info.flagsig_sel < 8)
			{
				//set flagpix length
				scanner_afe_setvalue(scanner, device, AFE_REG_TG_CFG4, LSB_8(scanner->afe_info.flagpixlen));
				scanner_afe_setvalue(scanner, device, AFE_REG_TG_CFG5, MSB_7(scanner->afe_info.flagpixlen));
			}
		
		}
		//enable outputpin
		scanner_afe_update_lbits(scanner, device, AFE_REG_OUTPUT_CTRL, AFE_07_OE_OP, AFE_07_OE_OP);

		//enable data output
		scanner_afe_update_lbits(scanner, device, AFE_REG_OUTPUT_CTRL, AFE_07_OUTPD, scanner->afe_info.outdataen);
	}else   
		//disable data output
		scanner_afe_update_lbits(scanner, device, AFE_REG_OUTPUT_CTRL, AFE_07_OUTPD, scanner->afe_info.outdataen);
                
	scanner_afe_pll_dll_config(scanner, device);	
}


void scanner_afe_test_pattern_generator_config(struct scanunit *scanner, int device, struct afe_test_pattern_generator para)
{
 	scanner_afe_update_lbits(scanner, device, AFE_REG_PG_CONFIG, AFE_14_PGPAT_MASK, (para.output_type<<AFE_14_PGPAT_SHIFT));
	scanner_afe_update_lbits(scanner, device, AFE_REG_PG_CONFIG, AFE_14_PGINV_INV, (para.output_invert<<AFE_14_PGINV_SHIFT));
	scanner_afe_update_lbits(scanner, device, AFE_REG_PG_CONFIG, AFE_14_SEL_PGZ, AFE_14_SEL_PGZ);
	scanner_afe_update_lbits(scanner, device, AFE_REG_PG_CONFIG, AFE_14_SEL_PGY, AFE_14_SEL_PGY);
	scanner_afe_update_lbits(scanner, device, AFE_REG_PG_CONFIG, AFE_14_SEL_PGX, AFE_14_SEL_PGX);
	scanner_afe_update_lbits(scanner, device, AFE_REG_PG_CONFIG, AFE_14_PGEN, AFE_14_PGEN);

	scanner_afe_setvalue(scanner, device, AFE_REG_PG_CODE_LSB, LSB_8(para.pgcode));
	scanner_afe_setvalue(scanner, device, AFE_REG_PG_CODE_MSB, MSB_8(para.pgcode));
	scanner_afe_setvalue(scanner, device, AFE_REG_PG_WIDTH1, para.pgwidth1);
	scanner_afe_setvalue(scanner, device, AFE_REG_PG_WIDTH2, para.pgwidth2);}