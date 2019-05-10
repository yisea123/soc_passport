#ifndef __PAPERDRV_H__
#define __PAPERDRV_H__

#include "mechdrv.h"
#include "mechanism_ops.h"
#include <motor.h>
/*-------------------------------------------------------------
   
-------------------------------------------------------------*/ 
extern int fd_paperpath;

#define PAPER_SPEED_0		3125//2232	//from dts
#define PAPER_SPEED_1		2083//2232	//from dts
#define PAPER_SPEED_2		947//1237 	//from dts
#define PAPER_SPEED_3		506//675//827	//from dts
#define PAPER_SPEED_4		393	//from dts

#define PAPER_SPEED_ALIGN       	PAPER_SPEED_4    
#define PAPER_SPEED_LOAD_HIGH   	PAPER_SPEED_0    
#define PAPER_SPEED_LOAD_SCAN_6   	PAPER_SPEED_2    
#define PAPER_SPEED_LOAD_SCAN_10  	PAPER_SPEED_3
#define PAPER_SPEED_EJECT       	PAPER_SPEED_0  

#define ALIGN_SPEED		625 	//from dts
#define PATHSEL_SPEED		625	//from dts	NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH

#define PAPER_DIR_TO_FRONTGATE	MOTION_CLOCKWISE //MOTOR_DIR_BACKWARD
#define PAPER_DIR_TO_BACKEND	MOTION_COUNTERCLOCKWISE//MOTOR_DIR_FORWARD

typedef enum{
	PAPER_SCAN_6,
	PAPER_SCAN_10
}paper_scan_type_t;


/* paper path sensors masks .
   NOTE: same as dts.*/
#define	PPS_SEN_MSK		(unsigned short)0x1		 /* paperpresent_sensor */
#define	PPA_SEN00_MSK		(unsigned short)0x2		 /* paperalign_sensor00 */
#define	PPA_SEN01_MSK		(unsigned short)0x4		 /* paperalign_sensor01*/
#define	PPATH_SEN00_MSK		(unsigned short)0x8		 /* paperpath_sensor00*/
#define	PPATH_SEN01_MSK		(unsigned short)0x10		 /* paperpath_sensor01*/
#define	PPATH_SEN10_MSK		(unsigned short)0x20		 /* paperpath_sensor10*/
#define	PPATH_SEN11_MSK		(unsigned short)0x40		 /* paperpath_sensor11*/
#define	PPATH_SEN20_MSK		(unsigned short)0x80		 /* paperpath_sensor20*/
#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
#else
#define	PPATH_SEN21_MSK		(unsigned short)0x100		 /* paperpath_sensor21*/ /*del for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
//#define	PEJECT_SEN_MSK		(unsigned short)0x200		 /* papereject_sensor */
#endif
#define PCOVER_SEN_MSK		(unsigned short)0x400		 /* papercover_sensor*/ 

#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
#define PPATHSELPAP_SEN0_MSK	(unsigned short)0x800		/*pathselectpaper_sensor0.NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
#define PPATHSELPAP_SEN1_MSK	(unsigned short)0x1000		/*pathselectpaper_sensor1.NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
#define PATHSELRST_SEN_MSK	(unsigned short)0x2000		/*pathselectreset_sensor.NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
#endif

#define PPA_SEN_MSK	(PPA_SEN00_MSK|PPA_SEN01_MSK)
#define PPATH_SEN0X_MSK	(PPATH_SEN00_MSK|PPATH_SEN01_MSK)
#define PPATH_SEN1X_MSK	(PPATH_SEN10_MSK|PPATH_SEN11_MSK)

#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
#define PPATH_SEN2X_MSK	(PPATH_SEN20_MSK)	//modify for MECHUNIT_TYPE_ACCEPTOR_DUALPATH
#define PPATHSELPAP_SENX_MSK (PPATHSELPAP_SEN0_MSK|PPATHSELPAP_SEN1_MSK)
#define PPATH_SEN_ALL	(PPATH_SEN0X_MSK|PPATH_SEN1X_MSK|PPATH_SEN2X_MSK|PPATHSELPAP_SENX_MSK)
#else
#define PPATH_SEN2X_MSK	(PPATH_SEN20_MSK|PPATH_SEN21_MSK)
#define PPATH_SEN_ALL	(PPATH_SEN0X_MSK|PPATH_SEN1X_MSK|PPATH_SEN2X_MSK)
#endif


//#define PAPERPATH_PAPER_SEN_MSK	(PPS_SEN_MSK|PPA_SEN_MSK|PPATH_SEN_ALL|PEJECT_SEN_MSK)
#define PAPERPATH_PAPER_SEN_MSK	(PPS_SEN_MSK|PPA_SEN_MSK|PPATH_SEN_ALL)

#define PPA_SEN_POS		1
#define PPATH_SEN0X_POS		2
#define PPATH_SEN1X_POS		3
#define PPATH_SEN2X_POS		4
#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
#define PATHSELRST_SEN_POS	5
#endif

/* paper path motors mask
   NOTE: same as dts.*/
#define PAP_MOTOR               	(unsigned short)0x1		 /* paper_motor*/
#define ALIGN_MOTOR             	(unsigned short)0x2		 /* alignment_motor*/
#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
#define PATHSEL_MOTOR             	(unsigned short)0x8		 /* pathselect_motor.NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH*/
#endif

extern mech_drv_t paper_mech_drv;
extern mech_operation_t paper_operation;
#if 0
/*-------------------------------------------------------------
	paper_status_get
-------------------------------------------------------------*/
extern void paper_status_get(char showflag);


/*-------------------------------------------------------------
	paper_get_sen_raw_input
-------------------------------------------------------------*/
extern int paper_get_sen_raw_input(char showflag);
#endif

/*========================================================================================================================
	paperpath base API
========================================================================================================================*/
/*-------------------------------------------------------------
        paper_init
	note：Must executed at the initial stage
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_init(void);

/*========================================================================================================================
	paperpath moving API
========================================================================================================================*/ 
/*-------------------------------------------------------------
        paper_align
 
        description:synchro type motor move
		moving paper to aligner
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paper_align(void);

/*-------------------------------------------------------------
		paper_load
 
        description:synchro type motor move.load paper to specified sensors position and then stop or scan with moving specified length.
		sen_mask:	PPATH_SEN0X_MSK
		        	PPATH_SEN1X_MSK
        			PPATH_SEN2X_MSK
        	needscan:	0——stop at the specified sensors position.
        			1——when paper arrive the specified sensors position，start to scan with moving specified length
        	scanlength:	valid when needscan=1. (unit:0.01mm)
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paper_load(unsigned int sen_mask, unsigned char needscan, unsigned short scanlength);


/*-------------------------------------------------------------
		paper_scan
 
        description:synchro type motor move
	        paper move from aligner sensors，and stop before PPATH_SEN2X_MSK position.
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_scan(paper_scan_type_t paper_scan_type);

/*-------------------------------------------------------------
		paper_eject_rear
 
        description:synchro type motor move
		paper eject to rear acceptor path
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_eject_rear(void);

/*-------------------------------------------------------------
		paper_park_gate
 
        description:synchro type motor move
	        paper head move to front aligner position and park
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_park_gate(void);

/*-------------------------------------------------------------
		paper_eject_gate
 
        description:synchro type motor move
	        paper eject to front gate
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_eject_gate(void);

/*-------------------------------------------------------------
		paper_park_path
 
        description:synchro type motor move
	        paper tail move to front aligner position and park
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_park_path(void);

/*-------------------------------------------------------------
		paper_step
 
	description:synchro type motor move 
		steps_time——full steps for stepermotor/time for dcmotor
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_step(unsigned char  dir, unsigned short speed, unsigned long steps_time);

/*-------------------------------------------------------------
        paper_stop
 
	description:
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	paper_stop(void);

/*-------------------------------------------------------------
        aligner_open_close
 
        description:
        	openflag：	0——close
			       1——open	
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int aligner_open_close(unsigned char openflag);

/*-------------------------------------------------------------
        paper_skew_set
 
	description:
        	
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paper_skew_set(int skew_steps);

#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
/*-------------------------------------------------------------
        paperpath_path_select
 
	description:
        	paper_path：PAPER_PATH_0 / PAPER_PATH_1
		NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_path_select(paper_path_t paper_path);

/*-------------------------------------------------------------
        paperpath_path_reset
 
	description:
		NOTE:just define for MECHUNIT_TYPE_ACCEPTOR_DUALPATH
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_path_reset(void);
#endif
/*========================================================================================================================
	paperpath status API
========================================================================================================================*/ 
/*-------------------------------------------------------------
        paperpath_get_formlength
        return:
        	0——OK
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int paperpath_get_formlength(long * pformlength);

/*-------------------------------------------------------------
        paperpath_is_coveropen
        return:
        	0——close
        	1——open
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int paperpath_is_coveropen(void);

/*-------------------------------------------------------------
        paperpath_is_coveropen
        return:
        	0——close
        	1——open
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int paparpath_aligner_is_open(void);

/*-------------------------------------------------------------
        paperpath_paper_is_at_gate
        return:
        	0——no paper at front gate 
        	1——paper at front gate
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_gate(void);

/*-------------------------------------------------------------
        paperpath_paper_is_at_aligner
        return:
        	0——no paper at aligner 
        	1——paper at aligner
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_aligner(void);

/*-------------------------------------------------------------
        paperpath_paper_is_at_aligner_either
        return:
        	0——no paper at aligner either(PPA_SEN00 || PPA_SEN01)
        	1——paper at aligner either(PPA_SEN00 || PPA_SEN01)
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_aligner_either(void);

/*-------------------------------------------------------------
        paperpath_paper_is_at_sen0x
        return:
        	0——no paper at sen0x(PPATH_SEN00 & PPATH_SEN01)
        	1——paper at sen0x(PPATH_SEN00 & PPATH_SEN01)
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_sen0x(void);

/*-------------------------------------------------------------
        paperpath_paper_is_at_sen1x
        return:
        	0——no paper at sen1x(PPATH_SEN10 & PPATH_SEN11)
        	1——paper at sen1x(PPATH_SEN10 & PPATH_SEN11)
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_sen1x(void);

/*-------------------------------------------------------------
        paperpath_paper_is_at_sen2x
        return:
        	0——no paper at sen2x(PPATH_SEN20|PPATH_SEN21)
        	1——paper at sen2x(PPATH_SEN20|PPATH_SEN21)
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_sen2x(void);

#if 0
/*-------------------------------------------------------------
        paperpath_paper_is_at_eject
        return:
        	0——no paper at eject(PEJECT_SEN_MSK)
        	1——paper at eject(PEJECT_SEN_MSK)
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_at_eject(void);
#endif

/*-------------------------------------------------------------
        paperpath_paper_is_inpath
        return:
        	0——has no paper 
        	1——has paper 
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_paper_is_inpath(void);

/*-------------------------------------------------------------
	paperpath_scan_can_started
        description:
        	service for paper moving accompanied with scanning.
        	such as paper_scan()、paper_load()
	return:
	    0——can not start scan 
	    1——can start scan 
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_scan_can_started(void);

/*-------------------------------------------------------------
	paperpath_reset_scantriger_status
-------------------------------------------------------------*/
extern MECHDRVLIB_API void paperpath_reset_scantriger_status(void);

#if defined(MX6Q_VOUCHER_SCANNER_DUAL_RECYCLE_BOXES)
/*-------------------------------------------------------------
	paperpath_mech_type_is_acceptor_singlepath
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_mech_type_is_acceptor_singlepath(void);

/*-------------------------------------------------------------
	paperpath_mech_type_is_acceptor_dualpath
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_mech_type_is_acceptor_dualpath(void);

/*-------------------------------------------------------------
        paperpath_path_is_reset
        return:
        	0——no reset
        	1——reset 
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paperpath_path_is_reset(void);
#endif
/*============================================================
        paperpath sensors calibrate API
        note:
	paper_sen_calibrate_reset will clear calibrate status of all sensors of paperpath.
        for counter type sensors, calibrate without paper.
        for reflection type sensors, calibrate with paper.
============================================================*/ 
/*-------------------------------------------------------------
        paper_sen_calibrate_reset
 
	description:
        	clear calibrate status of all sensors of paperpath.
        return:
	        none
-------------------------------------------------------------*/
extern MECHDRVLIB_API void paper_sen_calibrate_reset(void);
#if 0
/*-------------------------------------------------------------
        paper_sen_calibrate_withpaper
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paper_sen_calibrate_withpaper(void);

/*-------------------------------------------------------------
        paper_sen_calibrate_withoutpaper
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paper_sen_calibrate_withoutpaper(void);
#endif
#if 0
/*-------------------------------------------------------------
        paper_sen_calibrate
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int paper_sen_calibrate(void);
#endif

extern MECHDRVLIB_API int paper_match_sen_calibrate_withoutpaper(void);
#endif
