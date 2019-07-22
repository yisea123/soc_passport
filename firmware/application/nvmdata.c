#include "nvmapi.h"
#include "nvram.h"

#include "nvmdata.h"


#define TABLE_ENTRY(Id, Name, Info)	{.id = (Id), .name = (Name), .info = &(Info)}
#define NVM_DATA_ENTRY(name)		TABLE_ENTRY(NVM_DATA_ID(name), NVM_DATA_NAME(name), NVM_DATA_INFO(name))

#define NVM_BLOCK_COUNT(table)		(sizeof(table)/sizeof(nvram_block_t))

DECLARE_NVM_DATA(SCAN_DPI_200_ADC_A);
DECLARE_NVM_DATA(SCAN_DPI_200_ADC_B);
DECLARE_NVM_DATA(SCAN_DPI_300_ADC_A);
DECLARE_NVM_DATA(SCAN_DPI_300_ADC_B);
DECLARE_NVM_DATA(SCAN_DPI_600_ADC_A);
DECLARE_NVM_DATA(SCAN_DPI_600_ADC_B);
DECLARE_NVM_DATA(SCAN_DPI_200_CIS_A);
DECLARE_NVM_DATA(SCAN_DPI_200_CIS_B);
DECLARE_NVM_DATA(SCAN_DPI_300_CIS_A);
DECLARE_NVM_DATA(SCAN_DPI_300_CIS_B);
DECLARE_NVM_DATA(SCAN_DPI_600_CIS_A);
DECLARE_NVM_DATA(SCAN_DPI_600_CIS_B);
DECLARE_NVM_DATA(SCAN_DPI_200_CALIBDATA);
DECLARE_NVM_DATA(SCAN_DPI_300_CALIBDATA);
DECLARE_NVM_DATA(SCAN_DPI_600_CALIBDATA);
DECLARE_NVM_DATA(SCAN_DPI_1200_CALIBDATA);


/* nvmdata stored in AT24Cxx EEPROM */
static nvram_block_t eeprom_blocks[]=
{
	NVM_DATA_ENTRY(SCAN_DPI_200_ADC_A),
	NVM_DATA_ENTRY(SCAN_DPI_200_CIS_A),
};


/* nvmdata stored in SOC eNVM */
static nvram_block_t envm_blocks[]=
{
	NVM_DATA_ENTRY(SCAN_DPI_200_CALIBDATA),
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
		.length = 0x10000,
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
