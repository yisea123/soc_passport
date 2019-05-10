#ifndef __CARD_SCANPATH_DRV_H__
#define __CARD_SCANPATH_DRV_H__

#include "mechanism_ops.h"
#include "motor.h"
/*-------------------------------------------------------------
   
-------------------------------------------------------------*/ 

#define CARDPATH_SPEED_COLOR		520		//from ramp.c
#define CARDPATH_SPEED_GRAY		781		//from ramp.c
#define CARDPATH_SPEED_MOVECARD		800		//from ramp.c

#define CARDPATH_DIR_TO_GATE	MOTION_CLOCKWISE 
#define CARDPATH_DIR_TO_REAR	MOTION_COUNTERCLOCKWISE  

#define CARRIER_POS_SEN_POS		1

extern mech_drv_t cardpath_mech_drv;
extern mech_operation_t cardpath_operation;


typedef enum{
	SCAN_TYPE_200DPI_SIX,
	SCAN_TYPE_200DPI_TEN,	
	SCAN_TYPE_300DPI_GRAY,
	SCAN_TYPE_300DPI_COLOR
}move_scantype_t;

/*========================================================================================================================
	cardpath base API
========================================================================================================================*/
/*-------------------------------------------------------------
        cardpath_init
	note£ºMust executed at the initial stage
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	cardpath_init(void);

/*========================================================================================================================
	cardpath moving API
========================================================================================================================*/ 

/*-------------------------------------------------------------
		cardpath_step
	 
        description:synchro type motor move
        	dir--CARDPATH_DIR_TO_GATE/CARDPATH_DIR_TO_REAR
			speed--CARDPATH_SPEED_COLOR/CARDPATH_SPEED_GRAY/CARDPATH_SPEED_MOVECARD
		steps_time--full steps for stepermotor
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	cardpath_step(unsigned char dir, unsigned short speed, unsigned long steps_time);

extern MECHDRVLIB_API int cardpath_cardscan(move_scantype_t move_scantype);
extern MECHDRVLIB_API int cardpath_cardintake(void);
extern MECHDRVLIB_API int cardpath_eject_gate(void);
extern MECHDRVLIB_API int cardpath_return_gate(void);
extern MECHDRVLIB_API int cardpath_park_gate(void);
/*-------------------------------------------------------------
        cardpath_stop
 
	description:
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern MECHDRVLIB_API int	cardpath_stop(void);

/*========================================================================================================================
	cardpath status API
========================================================================================================================*/ 
/*-------------------------------------------------------------
        cardpath_is_coveropen
        return:
        	0-not reset
        	1-reset
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int cardpath_is_coveropen(void);

/*-------------------------------------------------------------
        cardpath_is_at_cd
        return:
        	0-not at CARDPATH_CD_SENSOR_MASK
        	1-at CARDPATH_CD_SENSOR_MASK
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int cardpath_is_at_cd(void);

/*-------------------------------------------------------------
        cardpath_is_at_front
        return:
        	0-not at (CARDPATH_CD_SENSOR_MASK|CARDPATH_CS0_SENSOR_MASK)
        	1-at (CARDPATH_CD_SENSOR_MASK|CARDPATH_CS0_SENSOR_MASK)
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int cardpath_is_at_front(void);

/*-------------------------------------------------------------
        cardpath_is_at_rear
        return:
        	0-not at CARDPATH_CS2_SENSOR_MASK
        	1-at CARDPATH_CS2_SENSOR_MASK
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int cardpath_is_at_rear(void);

/*-------------------------------------------------------------
        cardpath_is_in_path
        return:
        	0-not in path
        	1-in path
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int cardpath_is_in_path(void);

/*-------------------------------------------------------------
        cardpath_sensor_status
        return:
        	0-not in path
        	1-in path
-------------------------------------------------------------*/ 
extern MECHDRVLIB_API int cardpath_sensor_status(void);
/*============================================================
        cardpath sensors calibrate API
        note:
	cardpath_sen_calibrate_reset will clear calibrate status of all sensors of cardpath.
        for counter type sensors, calibrate without paper.
        for reflection type sensors, calibrate with paper.
============================================================*/ 
/*-------------------------------------------------------------
        cardpath_sen_calibrate_reset
 
	description:
        	clear calibrate status of all sensors of cardpath.
        return:
	        none
-------------------------------------------------------------*/
extern MECHDRVLIB_API void cardpath_sen_calibrate_reset(void);

extern MECHDRVLIB_API int cardpath_match_sen_calibrate_withoutpaper(void);
#endif

