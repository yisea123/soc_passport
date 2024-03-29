#ifndef __SCANCALIB_H__
#define __SCANCALIB_H__

#include <stdbool.h>
#include "scanning.h"
#include "scanunit.h"

#define BIT(x) 	(1<<(x))
#define LSB_8(x)  ((x)&0xff)
#define MSB_7(x)  (((x)&0x7f00)>>8)
#define MSB_8(x)  (((x)&0xff00)>>8)

#define REG_ADDR_LIGHTSOURCE		0x00
#define REG_MASK_LIGHTSOURCE		0xffff
//afe registers address
#define AFE_REG_SOFT_RESET		0x0
#define AFE_REG_SETUP_REG1		0x3
#define AFE_REG_OUTPUT_CTRL		0x7
#define AFE_REG_CMOS_DRV_CTRL1		0xD
#define AFE_REG_PG_CONFIG		0x14
#define AFE_REG_PG_CODE_LSB		0x15
#define AFE_REG_PG_CODE_MSB		0x16
#define AFE_REG_PG_WIDTH1		0x17
#define AFE_REG_PG_WIDTH2		0x18
#define AFE_REG_CLK_MONITOR		0x19
#define AFE_REG_PLL_CTRL1		0x1a
#define AFE_REG_PLL_CTRL2		0x1b
#define AFE_REG_PLL_DIVIDER_CTRL1	0x1c
#define AFE_REG_DAC_IN1			0x24
#define AFE_REG_DAC_IN2			0x25
#define AFE_REG_DAC_IN3			0x26
#define AFE_REG_DAC_IN4			0x27
#define AFE_REG_DAC_IN5			0x28
#define AFE_REG_DAC_IN6			0x29
#define AFE_REG_DAC_IN7			0x2a
#define AFE_REG_DAC_IN8			0x2b
#define AFE_REG_DAC_IN9			0x2c
#define AFE_REG_AGAIN_IN1		0x2d
#define AFE_REG_AGAIN_IN2		0x2e
#define AFE_REG_AGAIN_IN3		0x2f
#define AFE_REG_AGAIN_IN4		0x30
#define AFE_REG_AGAIN_IN5		0x31
#define AFE_REG_AGAIN_IN6		0x32
#define AFE_REG_AGAIN_IN7		0x33
#define AFE_REG_AGAIN_IN8		0x34
#define AFE_REG_AGAIN_IN9		0x35
#define AFE_REG_DLL_CFG1		0x80
#define AFE_REG_DLL_CFG2		0x81
#define AFE_REG_RSMP_RISE		0x82
#define AFE_REG_RSMP_FALL		0x83
#define AFE_REG_VSMP_RISE		0x84
#define AFE_REG_VSMP_FALL		0x85
#define AFE_REG_TG_CFG1			0xa0
#define AFE_REG_TG_CFG2                 0xa1
#define AFE_REG_TG_CFG3			0xa2
#define AFE_REG_TG_CFG4                 0xa3
#define AFE_REG_TG_CFG5			0xa4
#define AFE_REG_TG_CFG21                0xb4



#define AFE_03_PD		BIT(0)
#define AFE_03_PDMD		BIT(1)
#define AFE_07_OUTSYNC		BIT(4)
#define AFE_07_CMOSMODE_EN	BIT(5)
#define AFE_07_OUTPD		BIT(6)
#define AFE_07_OE_OP		BIT(7)
#define AFE_0D_DRV_CTRL		BIT(3)
#define AFE_0D_OPDRV_MASK	(0x7)
#define AFE_14_PGINV_INV	BIT(4)
#define AFE_14_SEL_PGZ		BIT(3)
#define AFE_14_SEL_PGY		BIT(2)
#define AFE_14_SEL_PGX		BIT(1)
#define AFE_14_PGEN		BIT(0)
#define AFE_14_PGINV_SHIFT	(4)
#define AFE_14_PGPAT_SHIFT	(5)
#define AFE_14_PGPAT_MASK	(0x60)
#define AFE_19_MONCLK_RSMP	(0x4)
#define AFE_19_MONCLK_VSMP	(0x5)
#define AFE_19_MONCLK_ACLK	(0x6)
#define AFE_19_MONCLK_OCLK	(0x7)
#define AFE_19_MONCLK_MASK	(0x7)
#define AFE_1C_PLLEXDIV_SEL_SHIFT (4)
#define AFE_1C_PLLEXDIV_SEL_MASK (0x70)
#define AFE_A0_TG_EN		BIT(0)
#define AFE_A0_TGMD_SLAVE	0
#define AFE_A0_TGMD_MASTER	BIT(1)
#define AFE_A0_TGMD_MASK	BIT(1)
#define AFE_B4_SEL_FLAG_FLAGPIX	0
#define AFE_B4_SEL_FLAG_PO0	(0x8)
#define AFE_B4_SEL_FLAG_PO1	(0x9)
#define AFE_B4_SEL_FLAG_PO2	(0xA)
#define AFE_B4_SEL_FLAG_PO3	(0xB)
#define AFE_B4_SEL_FLAG_PO4	(0xC)
#define AFE_B4_SEL_FLAG_PO5	(0xD)
#define AFE_B4_SEL_FLAG_PO6	(0xE)
#define AFE_B4_SEL_FLAG_PO7	(0xF)
#define AFE_B4_SEL_FLAG_MASK	(0xf)


struct scancalib_data_buf
{	 
	unsigned char sectionbuf[2*1024*1024]; 	//scan calibrate raw data buf after section
	unsigned char buffer[2*1024*1024]; 	//needed calibrate data buf
	unsigned char calibuf[2*1024*1024];	//store calibrated result
	int average[256];			//store average result
	unsigned char config[4*1024];
	int distr[256];
	int distr_red[256];
	int distr_green[256];
	int distr_blue[256];
};

struct scanner_adjustable_parameters_config{
	unsigned char config_tpye;
	int side;
	unsigned int hwresolution;
	unsigned int hwmode;
};

typedef struct scancalib_lightaddr_s
{
	unsigned int para;		// scancalib mode
	unsigned int lightaddr;		// scancalib lightsource addr
} scancalib_lightaddr_t;

typedef struct scancalib_lightsource_config_s
{
	unsigned lightsource : 8;	// scancalib lightsource
	unsigned sideid : 4;		// scancalib sideid
} scanner_lightsource_config_t;

typedef union {
	unsigned int para;			// scancalib lightsource parameter
	scanner_lightsource_config_t config;	// scancalib lightsource config
}scancalib_paramenter_t; 

struct afe_test_pattern_generator
{
	unsigned char output_type; 	//select pattern generator output
	unsigned char output_invert;	//invert pattern generator output
	unsigned short pgcode; 		//pgcode of pattern generator
	unsigned char pgwidth1;		//pgwidth1 of pattern generator
	unsigned char pgwidth2;		//pgwidth2 of pattern generator
};

extern int max_value_get(int *distrdata, int size);
extern int set_cis_ontime(struct scanunit *scanner, int id, struct scan_reg_config *light_config, int regcount);
extern void scanner_set_cis_lightsource(struct scanunit *scanner, int side, unsigned short lightsource);
extern int scanner_fill_image_data(unsigned char *rawdata, int totalpixels, int linepixels, int start_pixel, int width, unsigned int pattern);
extern int scanner_set_afe_offset(struct scanunit *scanner, int device, unsigned int offset);
extern void scanner_set_afe_siginac_config(struct scanunit *scanner, int device, unsigned int siginac, unsigned int sigacti);
extern int scanner_set_afe_gainmsb_config(struct scanunit *scanner, int device, unsigned int gainmsb);
extern int scanner_get_afe_gainmsb_config(struct scanunit *scanner, int device, unsigned int *gainmsb);
extern void scanner_set_afe_vsmpup_config(struct scanunit *scanner, int device, unsigned int vsmpup, unsigned int vsmpdown);
extern void scanner_afe_setvalue(struct scanunit *scanner, int device, int addr, unsigned char value);
extern void scanner_afe_getvalue(struct scanunit *scanner, int device, int addr, unsigned char *value);
extern void scanner_afe_registers_reset(struct scanunit *scanner, int device);
extern void scanner_afe_system_reset(struct scanunit *scanner, int device);
extern	void scanner_afe_pll_dll_config(struct scanunit *scanner, int device);
extern void scanner_set_afe_clock_monitor_source(struct scanunit *scanner, int device, unsigned int monclk);
extern void scanner_afe_power_ctrl(struct scanunit *scanner, int device, int on_off);
extern int scanner_write_config_to_nvm(int setting, unsigned char *config, int len);
extern int scanner_read_config_from_nvm(int setting, unsigned char *config, int len);
extern void scanner_cmos_output_config(struct scanunit *scanner, int device);
extern void scanner_afe_test_pattern_generator_config(struct scanunit *scanner, int device, struct afe_test_pattern_generator para);

extern void check_scanner_set_led_ontime(int side, unsigned int value, int lightsource);
extern void check_scanner_set_all_led_ontime(int side);
extern int check_scanner_adjust_afe_offset(int hwresolution, int hwmode, int side);
extern int check_scanner_adjust_cis_led_brightness(int hwresolution, unsigned int hwmode, int side, unsigned int light_sel);
extern int check_scanner_setup_ajustable_config(int hwresolution, int hwmode);
extern int check_scanner_load_ajustable_config(int hwresolution, int hwmode);
extern int check_scanner_save_ajustable_config(int hwresolution, int hwmode, int para_type, int side);
extern int check_scanner_read_calidata(unsigned char *pdata, int *len, unsigned char unitid, unsigned char resolution);
extern int check_scanner_write_calidata(unsigned char *pdata, int len, unsigned char unitid, unsigned char resolution);
extern void check_scanner_reset_cisled_config(unsigned int flag);
extern int check_scanner_calibrate_adjustable_paramters(void);
extern int check_get_current_brightness_distr(struct scanunit *scanner, int hwresolution, int scanmode, int lights, int side, int lightsource, int *red_distr, int *green_distr, int *blue_distr, int flags);
extern int check_scanner_set_exposure_time(scanning_exposure_time_setting_t *setting, int exposuretime);
extern int check_scanner_get_exposure_time(scanning_exposure_time_setting_t *setting, int *exposuretime);

#endif /* __SCANCALIB_H__ */
