#ifndef __PAPERPATH_H__
#define __PAPERPATH_H__

//#include "../paperpath_unit.h"
//#include "../mechlib.h"
#include <mechlib.h>
#include <motor.h>

extern int fd_paperpath;

#define DISTANCE_PPS_TO_PPA			1780	// 17.8mm  paperpresent_sensor<-->paperalign_sensor
#define DISTANCE_PPA_TO_ALIGNER			(50*10)	// 0.5mm  paperalign_sensor<-->aligner
#if 0
#define DISTANCE_PPA_TO_PPATHSEN0		4850	// paperalign_sensor<-->paperpath_sensor0
#define DISTANCE_PPATHSEN0_TO_PPATHSEN1		7200	// paperpath_sensor0<-->paperpath_sensor1
#define DISTANCE_PPATHSEN1_TO_PPATHSEN2  	5740	// paperpath_sensor1<-->paperpath_sensor2
#else
#define DISTANCE_PPA_TO_PPATHSEN0		5180	// paperalign_sensor<-->paperpath_sensor0
#define DISTANCE_PPATHSEN0_TO_PPATHSEN1		5180	// paperpath_sensor0<-->paperpath_sensor1
#define DISTANCE_PPATHSEN1_TO_PPATHSEN2  	6040	// paperpath_sensor1<-->paperpath_sensor2
#endif
#define DISTANCE_PPATHSEN1_TO_CISBASE		2985	//paperpath_sensor1<-->CIS Base 28.85mm
#define DISTANCE_PPAHSEN2_TO_REARROLLER		 830
//#define DISTANCE_EJECT_MIN			(DISTANCE_PPATHSEN1_TO_PPATHSEN2-DISTANCE_PPATHSEN1_TO_CISBASE+DISTANCE_PPAHSEN2_TO_REARROLLER+500)//1320 	//(1320*5)// 13.2mm  paperpath_sensor2<-->papereject_sensor
#define DISTANCE_EJECT_MIN			(DISTANCE_PPATHSEN1_TO_PPATHSEN2-DISTANCE_PPATHSEN1_TO_CISBASE+DISTANCE_PPAHSEN2_TO_REARROLLER+500)//1200 	// paperpath_sensor2-->acceptor box 110mmï½ž115mm
#define DISTANCE_PPATHSEN2_TO_ACCEPTOR		9900	// paperpath_sensor2<-->acceptorpath_sensor	89mm

#define DISTANCE_PPA_TO_PPATHSEN2	(DISTANCE_PPA_TO_PPATHSEN0+DISTANCE_PPATHSEN0_TO_PPATHSEN1+DISTANCE_PPATHSEN1_TO_PPATHSEN2)

#define CHECK_MAX_LENGTH	24000//22500  //225mm
#define CHECK_MIN_LENGTH	17000	//170mm

//#define RATIO_PAPERPATH		66	// Ã¿²½ÐÐ³Ì0.65975mm
//#define RATIO_PAPERPATH		(2540/200)//13	// 200 full steps pass 25.4mm
#define RATIO_PAPERPATH    200/2540    	// 200 full steps pass 25.4mm
//#define RATIO_PAPERPATH		(2465/200)// trial value

#define STEP_ALIGN_MOVE		33 //41
#define STEP_PATHSEL_MOVE	20	/*NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/

#define ALIGN_DIR_CLOSE		MOTION_COUNTERCLOCKWISE
#define ALIGN_DIR_OPEN		MOTION_CLOCKWISE

/*NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
#define PATHSEL_DIR_0		MOTION_COUNTERCLOCKWISE
#define PATHSEL_DIR_1		MOTION_CLOCKWISE

typedef struct {
	#if 1
	mechunit_drv_status_t drv_status;
	#else
	paper_drv_status_t drv_status;
	#endif
	long 	path_status;	        /* paper path sensors states	*/	
	unsigned char	cover_status;		/* cover status	. 0â€”â€”opened/1â€”â€”closed		*/
	unsigned char   aligner_status;
#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
	unsigned char 	pathselrst_status;	/*NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
#endif
}paper_status_t;

#define ALIGNER_CLOSE     	0x00
#define ALIGNER_OPEN      	0x01


#endif
