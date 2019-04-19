
#include "FreeRTOS.h"
#include "task.h"

#ifdef __ICCARM__
#include <yfuns.h>
#endif
#include <stdio.h>
#include <string.h>

#include <stdint.h>

#include <stdlib.h>


#include "nvram.h"
#include "nvmdata.h"
#include "indicator.h"

#define usleep(a)		{uint32_t i; for(i=0; i<a; i++);}
extern struct nvram nvram_list[];
extern unsigned char scanner_modetonvmid(int para);
struct nvram *eepram = &nvram_list[1];

const uint8_t dpi_300_adc_default_a[] = {0x80, 0x80};
const uint8_t dpi_300_adc_default_b[] = {0x80, 0x80};
//				       color(R,    G,     B),   grey
const uint16_t dpi_300_cis_default_a[] = {0x0100, 0x0100, 0x0100, 0x0060};
const uint16_t dpi_300_cis_default_b[] = {0x0100, 0x0100, 0x0100, 0x0060};

DEFINE_NVM_DATA(SCAN_DPI_300_ADC_A, sizeof(dpi_300_adc_default_a), dpi_300_adc_default_a);
DEFINE_NVM_DATA(SCAN_DPI_300_ADC_B, sizeof(dpi_300_adc_default_b), dpi_300_adc_default_b);
DEFINE_NVM_DATA(SCAN_DPI_600_ADC_A, 2, NULL);
DEFINE_NVM_DATA(SCAN_DPI_600_ADC_B, 2, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CIS_A, sizeof(dpi_300_cis_default_a), dpi_300_cis_default_a);
DEFINE_NVM_DATA(SCAN_DPI_300_CIS_B, sizeof(dpi_300_cis_default_b), dpi_300_cis_default_b);
DEFINE_NVM_DATA(SCAN_DPI_600_CIS_A, 24, NULL);
DEFINE_NVM_DATA(SCAN_DPI_600_CIS_B, 24, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_A, CIS_PIXELS_300DPI+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_B, CIS_PIXELS_300DPI+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_A, CIS_PIXELS_300DPI+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_B, CIS_PIXELS_300DPI+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_DARK_A, CIS_PIXELS_300DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_DARK_B, CIS_PIXELS_300DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_A, CIS_PIXELS_300DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_B, CIS_PIXELS_300DPI*3+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_IR_A, CIS_PIXELS_300DPI+CALI_EXTRA_SIZE, NULL);
DEFINE_NVM_DATA(SCAN_DPI_300_CALIBDATA_IR_B, CIS_PIXELS_300DPI+CALI_EXTRA_SIZE, NULL);

config_match_t configmode_to_para_table[] = {
	{(SIDE_A<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_EN_RGB<<4)|FPGA_REG_CIS_DPI_300, 		NVM_DATA_ID(SCAN_DPI_300_ADC_A), 		ADC_COLOR_OFFSET, 	SCAN_TYPE_300DPI_COLOR},
	{(SIDE_B<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_EN_RGB<<4)|FPGA_REG_CIS_DPI_300, 		NVM_DATA_ID(SCAN_DPI_300_ADC_B), 		ADC_COLOR_OFFSET, 	SCAN_TYPE_300DPI_COLOR},
	{(SIDE_A<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_EN_GREYSCALE<<4)|FPGA_REG_CIS_DPI_300,	NVM_DATA_ID(SCAN_DPI_300_ADC_A), 		ADC_GREYSCALE_OFFSET, 	SCAN_TYPE_300DPI_GRAY},
	{(SIDE_B<<16)|(CONFIG_ADC<<12)|(FPGA_REG_CIS_SCANMODE_EN_GREYSCALE<<4)|FPGA_REG_CIS_DPI_300,	NVM_DATA_ID(SCAN_DPI_300_ADC_B), 		ADC_GREYSCALE_OFFSET, 	SCAN_TYPE_300DPI_GRAY},
	{(SIDE_A<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_EN_RGB<<4)|FPGA_REG_CIS_DPI_300, 		NVM_DATA_ID(SCAN_DPI_300_CIS_A),		CIS_COLOR_OFFSET, 	SCAN_TYPE_300DPI_COLOR},
	{(SIDE_B<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_EN_RGB<<4)|FPGA_REG_CIS_DPI_300, 		NVM_DATA_ID(SCAN_DPI_300_CIS_B),		CIS_GREYSCALE_OFFSET, 	SCAN_TYPE_300DPI_COLOR},
	{(SIDE_A<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_EN_GREYSCALE<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CIS_A),		CIS_GREYSCALE_OFFSET,	SCAN_TYPE_300DPI_GRAY},
	{(SIDE_B<<16)|(CONFIG_CIS<<12)|(FPGA_REG_CIS_SCANMODE_EN_GREYSCALE<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CIS_B),		CIS_GREYSCALE_OFFSET, 	SCAN_TYPE_300DPI_GRAY},
	{(SIDE_A<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_GREYSCALE_DARK<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_A),0,			SCAN_TYPE_300DPI_GRAY},
	{(SIDE_B<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_GREYSCALE_DARK<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_B),0,			SCAN_TYPE_300DPI_GRAY},
	{(SIDE_A<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_EN_GREYSCALE<<4)|FPGA_REG_CIS_DPI_300,	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_GREYSCALE_A),0,			SCAN_TYPE_300DPI_GRAY},
	{(SIDE_B<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_EN_GREYSCALE<<4)|FPGA_REG_CIS_DPI_300,	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_GREYSCALE_B),0,			SCAN_TYPE_300DPI_GRAY},
	{(SIDE_A<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_RGB_DARK<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_RGB_DARK_A),	0,			SCAN_TYPE_300DPI_COLOR},
	{(SIDE_B<<16)|(CONFIG_CALI<<12)|(FAKE_CIS_SCANMODE_EN_RGB_DARK<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_RGB_DARK_B), 0,			SCAN_TYPE_300DPI_COLOR},
	{(SIDE_A<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_EN_RGB<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_RGB_A),	0,			SCAN_TYPE_300DPI_COLOR},
	{(SIDE_B<<16)|(CONFIG_CALI<<12)|(FPGA_REG_CIS_SCANMODE_EN_RGB<<4)|FPGA_REG_CIS_DPI_300, 	NVM_DATA_ID(SCAN_DPI_300_CALIBDATA_RGB_B), 	0,			SCAN_TYPE_300DPI_COLOR},
};

unsigned char scanner_modetonvmid(int para)
{
	int i;
	for (i = 0; i < sizeof(configmode_to_para_table)/sizeof(config_match_t); i++) {
		if (configmode_to_para_table[i].para == para) {
			return configmode_to_para_table[i].nvm_id;
		}
	}
	return 0;
}

int nvram_write_test()
{
	char *buffer = "111111111111111111111111111111111";

	nvram_write(&nvram_list[1], buffer, 0, 0x21);

}


int nvram_read_test()
{
	char buf[256] = {0};
	int i = 0;

	nvram_read(&nvram_list[1], buf, 0, 0x21);

	for (i =0;i<33; i++) {
		printf("buf[%d] = %d \n\r",i,buf[i]);
	}
}

int nvm_write_test()
{
	uint8_t *buffer ;
        int i = 0;
	int count =10;
	buffer = malloc(count);
	for (i=0;i<2;i++) {
		buffer[i] = rand();
	}
	nvm_write(configmode_to_para_table[0].nvm_id, buffer, sizeof(dpi_300_adc_default_a));
	printf("write size is %d \n\r", sizeof(dpi_300_adc_default_a));
	for (i =0;i<sizeof(dpi_300_adc_default_a); i++) {
		printf("write data is  %d \n\r",buffer[i]);
	}

}

int nvm_read_test()
{
	char buf[256] = {0};
	int i = 0,rs = 0;

	nvm_read(configmode_to_para_table[0].nvm_id,buf,sizeof(dpi_300_adc_default_a));
	rs = sizeof(dpi_300_adc_default_a);
	printf("read size is %d \n\r", sizeof(dpi_300_adc_default_a));
	for (i =0;i<rs; i++) {
		printf("buf[%d] = %d \n\r",i,buf[i]);
	}

}

int id_test(int val)
{
	int id;
	id = scanner_modetonvmid(val);
	printf("seting id is %x------>%d\n\r",val,id);
}


int nvm_rw_test()
{
	uint8_t *buffer1 ;
	uint8_t *buffer2 ;
	uint8_t *buffer3 ;
	uint8_t *buffer4 ;

	uint8_t buf1[10] = {0};
	uint8_t buf2[10] = {0};
	uint8_t buf3[10] = {0};
	uint8_t buf4[10] = {0};
	int count1,count2, i,j,k,w,z;

	count1 = sizeof(dpi_300_adc_default_a);
	count2 = sizeof(dpi_300_cis_default_a);
	buffer1 = malloc(count1);
	buffer2 = malloc(count1);
	buffer3 = malloc(count2);
	buffer4 = malloc(count2);
	for (z=0;z<1000;z++) {
		for (i = 0; i < count1; i++) {
			buffer1[i] = rand();

		}
		for (j = 0; j < count1; j++) {
			buffer2[j] = rand();

		}
		for (k = 0; k < count2; k++) {
			buffer3[k] = rand();

		}
		for (w = 0; w < count2; w++) {
			buffer4[w] = rand();

		}

//		for (i = 0; i < count1; i++) {
//			printf("creat buffer1[%d]=%d\n\r",i,buffer1[i]);
//		}
//		printf("\r\n");
//		for (j = 0; j < count1; j++) {
//			printf("creat buffer2[%d]=%d\n\r",j,buffer2[j]);
//		}
//		printf("\r\n");
//		for (k = 0; k < count2; k++) {
//			printf("creat buffer3[%d]=%d\n\r",k,buffer3[k]);
//		}
//		printf("\r\n");
//		for (w = 0; w < count2; w++) {
//			printf("creat buffer4[%d]=%d\n\r",w,buffer4[w]);
//		}
//        	printf("\r\n");

		nvm_write(configmode_to_para_table[0].nvm_id, buffer1, count1);
		nvm_write(configmode_to_para_table[1].nvm_id, buffer2, count1);
		nvm_write(configmode_to_para_table[4].nvm_id, buffer3, count2);
		nvm_write(configmode_to_para_table[5].nvm_id, buffer4, count2);

		usleep(500000);
		nvm_read(configmode_to_para_table[0].nvm_id,buf1,count1);
		nvm_read(configmode_to_para_table[1].nvm_id,buf2,count1);
		nvm_read(configmode_to_para_table[4].nvm_id,buf3,count2);
		nvm_read(configmode_to_para_table[5].nvm_id,buf4,count2);

//		for (i = 0; i < count1; i++) {
//			printf("read buf1[%d]=%d\n\r",i,buf1[i]);
//		}
//		printf("\r\n");
//		for (j = 0; j < count1; j++) {
//			printf("read buf2[%d]=%d\n\r",j,buf2[j]);
//		}
//		printf("\r\n");
//		for (k = 0; k < count2; k++) {
//			printf("read buf3[%d]=%d\n\r",k,buf3[k]);
//		}
//		printf("\r\n");
//		for (w = 0; w < count2; w++) {
//			printf("read buf4[%d]=%d\n\r",w,buf4[w]);
//		}
//		printf("\r\n");

		for (i = 0; i < count1; i++) {
			if (buffer1[i] != buf1[i]) {
				printf("buffer1[%d]=%d <----> buf1[%d]=%d\n\r",i,buffer1[i],i,buf1[i]);
				indicator_flash_set(INDICATOR_0,2);
			}
		}
		for (i = 0; i < count1; i++) {
			if (buffer2[i] != buf2[i]) {
				printf("buffer2[%d]=%d <----> buf2[%d]=%d\n\r",i,buffer2[i],i,buf2[i]);
				indicator_flash_set(INDICATOR_0,3);
			}
		}
		for (k = 0; k < count2; k++) {
			if (buffer3[k] != buf3[k]) {
				printf("buffer3[%d]=%d <----> buf3[%d]=%d\n\r",k,buffer3[k],k,buf3[k]);
				indicator_flash_set(INDICATOR_0,4);
			}
		}
		for (k = 0; k < count2; k++) {
			if (buffer4[k] != buf4[k]) {
				printf("buffer4[%d]=%d <----> buf4[%d]=%d\n\r",k,buffer4[k],k,buf4[k]);
				indicator_flash_set(INDICATOR_0,5);
			}
		}
	}
	printf("nvram write read test total number is %d\n\r",z);
	return 0;
}
