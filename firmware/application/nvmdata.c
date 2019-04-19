#include "nvmapi.h"
#include "nvram.h"

#include "nvmdata.h"


#define TABLE_ENTRY(Id, Name, Info)	{.id = (Id), .name = (Name), .info = &(Info)}
#define NVM_DATA_ENTRY(name)		TABLE_ENTRY(NVM_DATA_ID(name), NVM_DATA_NAME(name), NVM_DATA_INFO(name))

#define NVM_BLOCK_COUNT(table)		(sizeof(table)/sizeof(nvram_block_t))


DECLARE_NVM_DATA(SCAN_DPI_300_ADC_A);
DECLARE_NVM_DATA(SCAN_DPI_300_ADC_B);
DECLARE_NVM_DATA(SCAN_DPI_600_ADC_A);
DECLARE_NVM_DATA(SCAN_DPI_600_ADC_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CIS_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CIS_B);
DECLARE_NVM_DATA(SCAN_DPI_600_CIS_A);
DECLARE_NVM_DATA(SCAN_DPI_600_CIS_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_GREYSCALE_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_DARK_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_DARK_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_RGB_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_IR_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA_IR_B);


/* nvmdata stored in AT24Cxx EEPROM */
static nvram_block_t eeprom_blocks[]=
{
	NVM_DATA_ENTRY(SCAN_DPI_300_ADC_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_ADC_B),
	NVM_DATA_ENTRY(SCAN_DPI_300_CIS_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_CIS_B),
	NVM_DATA_ENTRY(SCAN_DPI_600_CIS_A),
	NVM_DATA_ENTRY(SCAN_DPI_600_CIS_B),
};


/* nvmdata stored in SOC eNVM */
static nvram_block_t envm_blocks[]=
{
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_GREYSCALE_DARK_B),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_GREYSCALE_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_GREYSCALE_B),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_RGB_DARK_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_RGB_DARK_B),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_RGB_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_RGB_B),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_IR_A),
	NVM_DATA_ENTRY(SCAN_DPI_300_CALIBDATA_IR_B),
};


static nvram_control_t nvram_sections[]=
{
	{
		.dev_id = 0,
		.validflag = 0x55aa,
		.version = 0x0001,
		.nvmblock = envm_blocks,
		.blocks = NVM_BLOCK_COUNT(envm_blocks),
		.offset = 0x100000,
		.length = 0x7000,
	},
	{
		.dev_id = 1,
		.validflag = 0x55aa,
		.version = 0x0001,
		.nvmblock = eeprom_blocks,
		.blocks = NVM_BLOCK_COUNT(eeprom_blocks),
		.offset = 0,
		.length = 0x200,
	}
};

#define NVRAM_SECTION_COUNT	(sizeof(nvram_sections)/sizeof(nvram_control_t))


int nvmdata_initialize()
{
	int rs;
	rs = nvm_register(NVRAM_SECTION_COUNT, nvram_sections);
//	printf("NVRAM_SECTION_COUNT=%d,nvram_sections=%d\n\r",NVRAM_SECTION_COUNT,nvram_sections);
	rs = nvm_initialize(0);
	if (rs != 0) {
		nvm_setdefault(0);
	}
	rs = nvm_initialize(1);
	if (rs != 0) {
		nvm_setdefault(1);
	}
	return rs;
}
