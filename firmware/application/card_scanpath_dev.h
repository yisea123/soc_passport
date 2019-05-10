#ifndef __CARD_SCANPATH_DEV_H__
#define __CARD_SCANPATH_DEV_H__

#include <mechlib.h>
#include <motor.h>

#define DISTANCE_CD_TO_CS0	2150	//21.5mm
#define DISTANCE_FROLLER_TO_CS0	1400	//14mm

#ifdef IC61S_4M
#define DISTANCE_CS0_TO_CS1	3100	//31mm
#define DISTANCE_CS1_TO_CS2	3150	//31.5mm
#endif
#ifdef IC61S_10M
#define DISTANCE_CS0_TO_CS1	3100	//31mm
#define DISTANCE_CS1_TO_CS2	3150	//31.5mm
#endif

#ifdef IC62S_4M
#define DISTANCE_CS0_TO_CS1	5600	//56mm
#define DISTANCE_CS0_TO_CISEND	4000	//40mm
#endif
#define IDCARD_LENGTH		8560	//85.6mm	

#ifdef IC62S_4M
#define RATIO_PAPERPATH    24/334   	// 24 full steps(1 circle) pass 3.34mm
#endif
#ifdef IC61S_4M
#define RATIO_PAPERPATH    48/813   	// 48 full steps(1 circle) pass 8.13mm
#endif
#ifdef IC61S_10M
#define RATIO_PAPERPATH    48/813   	// 48 full steps(1 circle) pass 8.13mm
#endif


typedef struct {
	mechunit_drv_status_t drv_status;	
	unsigned long cardpath;
}cardpath_status_t;


#endif

