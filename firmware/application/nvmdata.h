#ifndef __NVMDATA_H__
#define __NVMDATA_H__

#include "nvmapi.h"


/* define unique nvram data ID */
enum {
	NVM_DATA_ID(SCAN_DPI_200_ADC_A),
	NVM_DATA_ID(SCAN_DPI_200_ADC_B),
	NVM_DATA_ID(SCAN_DPI_200_CIS_A),
	NVM_DATA_ID(SCAN_DPI_200_CIS_B),
	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_DARK_A),
	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_DARK_B),
	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_A),
	NVM_DATA_ID(SCAN_DPI_200_CALIBDATA_COLOR_B),

};


#endif /* __NVMDATA_H__ */
