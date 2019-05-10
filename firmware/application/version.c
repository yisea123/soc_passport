#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "debug.h"
#include "gpiokey.h"
#include "fpgadrv.h"
#include "mss_sys_services.h"
#include "mss_comblk.h"

#define FIRMVARE_IC61S_4M	0x00
#define FIRMVARE_IC61S_10M	0x01
#define FIRMVARE_IC62S		0x02
#define FIRMVARE_IC61S_UV	0x03

#ifdef IC61S_4M
#define FIRMVARE_TPYE	FIRMVARE_IC61S_4M
#endif

#ifdef IC61S_10M
#define FIRMVARE_TPYE	FIRMVARE_IC61S_10M
#endif

#ifdef IC62S
#define FIRMVARE_TPYE	FIRMVARE_IC62S
#endif

#ifdef IC61S_UV
#define FIRMVARE_TPYE	FIRMVARE_IC61S_UV
#endif

extern struct gpiokey gpiokey_list[];
struct gpiokey *version_config_1 = &gpiokey_list[1];
struct gpiokey *version_config_2 = &gpiokey_list[2];
struct gpiokey *version_config_3 = &gpiokey_list[3];

int get_board_type()
{
	int status = 0;
	if (gpiokey_is_active(version_config_1))
		status 	|= 0x1;
	if (gpiokey_is_active(version_config_2))
		status 	|= 0x2;
	if (gpiokey_is_active(version_config_3))
		status 	|= 0x4;
	return status;
}

int version_check_result(void)
{
	int boardtype = 0, fpgatype = 0;

	boardtype = get_board_type();
	fpgatype = fpga_get_product_type();
	if(boardtype != FIRMVARE_TPYE)
		return -1;
	else if(fpgatype != FIRMVARE_TPYE)
		return -2;
	else
		return 0;
}

//---------------------------------------------
uint8_t version[2], version_code;
void itoa(uint32_t value, char *str, int len)
{
  	int i;
	char ch;
	
	for(i=0; i<len*2; i++)
	{
		ch = (value >> (i*4))&0x0f;
		ch = (ch>9)?(ch+7+0x30):(ch+0x30);
		str[len*2-1-i] = ch;
	}
	str[len*2] = 0;
}
void image_version_get(void)
{
  	uint8_t  ret=0;
	char version_str[10];
	
	MSS_SYS_init(MSS_SYS_NO_EVENT_HANDLER);

	ret = MSS_SYS_get_design_version(version);
	if(ret == MSS_SYS_SUCCESS)
	{
	  version_code = (version[1]<<8)|version[0];
	  itoa(version_code, version_str, 2);
	  printf("\n\rversion: 0x%x", version_code);
  
	}
	else
	  printf("\n\rversion get error!\n\r");
}