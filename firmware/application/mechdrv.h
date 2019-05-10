#ifndef __MECHDRV_H__
#define __MECHDRV_H__

#include <information.h>
#include <mechlib.h>
#define MECHDRVLIB_API
/*================================================================================

================================================================================*/
/*-------------------------------------------------
	sensor_get_info
-------------------------------------------------*/
extern mech_unit_info_t * sensor_get_info(void);

/*-------------------------------------------------
	motor_get_info
-------------------------------------------------*/
extern mech_unit_info_t *  motor_get_info(void);

extern mech_sen_raw_input_t *sensor_get_input_raw(void);
extern mech_sen_logic_input_t *sensor_get_input_logic(void);
extern mech_sen_cal_status_t *sensor_get_cal_status(void);
#endif
