#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "scanunit.h"
#include "scanning.h"

extern struct scanunit checkscanner;
extern struct scanunit ocrscanner;

#define CHECK_MAX_SCAN_LINES		3000
#define CHECK_MAX_SCAN_LIGHTSOURCES	10
#define CHECK_LIGHT_SOURCE_NUM 	7
#define CHECK_SCANLINE_PIXELS	1008
#define CHECK_SCANLINE_DATA_CNT	(CHECK_SCANLINE_PIXELS*4)
#define CHECK_SCANLINE_SIZE	CHECK_LIGHT_SOURCE_NUM*CHECK_SCANLINE_DATA_CNT

#define OCR_MAX_SCAN_LINES		3000
#define OCR_MAX_SCAN_LIGHTSOURCES	16
#define OCR_LIGHT_SOURCE_NUM 	7
#define OCR_SCANLINE_PIXELS	1008
#define OCR_SCANLINE_DATA_CNT	(OCR_SCANLINE_PIXELS)
#define OCR_SCANLINE_SIZE	OCR_LIGHT_SOURCE_NUM*OCR_SCANLINE_DATA_CNT

#define FAKE_CIS_SCANMODE_EN_SIX_LIGHTS_DARK 0xFF
#define FAKE_CIS_SCANMODE_EN_TEN_LIGHTS_DARK 0xFE
#define FAKE_CIS_SCANMODE_EN_GREYSCALE_DARK 0xFD
#define FAKE_CIS_SCANMODE_EN_RGB_DARK 0xFC

enum {
	SCANNING_INIT,
	SCANNING_WORKING,
	SCANNING_STOPPING,
	SCANNING_END,
};


// data structure of scanning control block
typedef struct scanning_control_s
{
	int mode;				// scanning mode
	int resolution;				// scanning resolution (DPI)
	int hwmode;				// hardware scanning mode
	int hwresolution;			// hardware scanning resolution (DPI)
	int scanmode;				// 6 or 10 scanmode	
	int state;				// scanning state
	int scanflag;				// scanning control flags
	int lightsource_flag;			// scanning set lightsource flags
	scanning_lightsource_t lightsource;	// scanning lightsource flags
	int scanlines;				// expected scanning lines
	volatile int linecount;			// scanning lines count
	unsigned char *buffer;			// scanning buffer
	unsigned char *buffptr;			// scanning buffer pointer
	unsigned int linedlen;			// scanning line data length
	unsigned int param_page;		// scanning page side
	unsigned int param_balace;              // scanning balace type(black or white)
} scanning_control_t;


/* bit definition of scanning control flag */
#define SCANNING_FLAG_LIGHTS_ON		(0x0001)
#define SCANNING_FLAG_START_AT_SENSOR	(0x0002)
#define SCANNING_FLAG_STOP_AT_SENSOR	(0x0004)
#define SCANNING_FLAG_CALIBRATE		(0x0008)

typedef struct scanning_ledmode_s
{
	unsigned int ledmode;		// scanning ledmode
	unsigned int hwledmode;	// scanning hardware ledmode
} scanning_ledmode_t;

typedef struct scanning_speedmode_s
{
	unsigned int scanmode;		// scanning mode
	unsigned int speedmode;		// scanning paper speedmode
} scanning_speedmode_t;

typedef struct config_match_s
{
	int para;			// config para
	unsigned int nvm_id;		// nvm id number
	unsigned int offset;		// offset of nvm's first address
	unsigned int speedtype;		// speedtype
} config_match_t;

typedef struct scanner_para_config_s
{
	unsigned hwresolution : 4;	// scanning resolution (DPI)
	unsigned mode : 8;		// scanning mode
	unsigned config_tpye : 4;	// scanning config para tpye
	unsigned sideid : 4;		// scanning cisid	
} scanner_para_config_t;

typedef union {
	unsigned int para;		// scansetting parameter
	scanner_para_config_t config;// scanning_setting
}parameters_config_t;

/* bit definition of scanning control flag */
#define SCANNING_FLAG_LIGHTS_ON		(0x0001)
#define SCANNING_FLAG_START_AT_SENSOR	(0x0002)
#define SCANNING_FLAG_STOP_AT_SENSOR	(0x0004)
#define SCANNING_FLAG_CALIBRATE		(0x0008)
#define SCANNING_FLAG_SECOND_CALIBRATE	(0x0010)

#define CONFIG_ADC 0
#define CONFIG_CIS 1
#define CONFIG_CALI 2
#define CHECK_SIDE_A 0
#define CHECK_SIDE_B 1
#define OCR_SIDE_A 0
#define OCR_SIDE_B 1

enum {
	ADC_COLOR_OFFSET = 0,
	ADC_GREYSCALE_OFFSET = 1,
	CIS_COLOR_OFFSET = 0,
	CIS_GREYSCALE_OFFSET = 6,
};

extern const unsigned char check_dpitohwdpi[];
extern scanning_ledmode_t check_ledmodetohwledmode[];
extern scanning_control_t check_scanning_ctrl;
extern unsigned char *scanbuffer;
extern parameters_config_t check_paraconfig;

extern int scanner_initialize();
extern void scanner_set_all_digitiser_default_config(void);

extern void check_scanner_set_sensor_default_config(void);
extern void check_scanner_get_digitiser_default_config(void);
extern unsigned char check_scanner_ledmodetohwledmode(unsigned char ledmode);
extern unsigned char check_scanner_modetospeedtype(int para);
extern unsigned char check_scanner_modetonvmid(int para);
extern unsigned int check_scanner_configmode_to_nvmoffset(int para);
extern void check_scanner_set_sensor_config(unsigned char hwresolution, unsigned char scanmode, unsigned char side);
#endif /* __SCANNER_H__ */
