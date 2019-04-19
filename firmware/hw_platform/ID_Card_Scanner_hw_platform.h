#ifndef ID_Card_Scanner_HW_PLATFORM_H_
#define ID_Card_Scanner_HW_PLATFORM_H_
/*****************************************************************************
*
*Created by Microsemi SmartDesign  Wed Jul 18 11:31:45 2018
*
*Memory map specification for peripherals in ID_Card_Scanner
*/

/*-----------------------------------------------------------------------------
* CM3 subsystem memory map
* Master(s) for this subsystem: CM3 FABRIC2MSSFIC2 
*---------------------------------------------------------------------------*/
#define CARD_SCANNER_MSS_0              0x40020800U
#define CARD_SCANNER_0_BIF_1            0x30000000U
#define CARD_SCANNER_0_BIF_2            0x30100000U
#define CARD_SCANNER_0_BIF_3            0x30200000U
#define CARD_SCANNER_0_BIF_4            0x30300000U
#define CARD_SCANNER_0_BIF_5            0x30400000U
#define CARD_SCANNER_0_BIF_6            0x30500000U
#define COREPWM_CHIP_0                  0x30E00000U
#define COREGPIO_CHIP_0                 0x30F00000U


/*-----------------------------------------------------------------------------
* RAM_2_MDDR_module_0 subsystem memory map
* Master(s) for this subsystem: RAM_2_MDDR_module_0 
*---------------------------------------------------------------------------*/
#define CARD_SCANNER_MSS_0_MDDR_DDR_AXI_SLAVE 0x00000000U


#endif /* ID_Card_Scanner_HW_PLATFORM_H_*/
