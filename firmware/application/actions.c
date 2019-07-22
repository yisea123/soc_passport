/*
	actions
*/
#include "actions.h"
#include "steppermotor.h"
#include "photosensor.h"
#include "simplesensor.h"
#include "FreeRTOS.h"
#include "task.h"

extern struct steppermotor * steppermotor_list[];
extern struct simplesensor * simplesensor_list[];
extern const int simplesensor_num;
extern struct photosensor * photosensor_list[];
extern const int photosensor_num;

#define DEV_SCANNER_MOTOR		steppermotor_list[0]
#define DEV_SENSOR_FRONT		simplesensor_list[0]
#define DEV_SENSOR_REAR			simplesensor_list[1]
#define DEV_SENSOR_EXSIT		photosensor_list[0]

/* 齿数25，齿距2.032mm，步距角0.9，步距25*2.032/400 */
#define CIS_MM_TO_STEP(x)				(x*1000/127)
#define CIS_MOTION_DISTANCE				CIS_MM_TO_STEP(98)

void sensor_event_handler(struct simplesensor * sensor, sensor_event_t event, void * data)
{
	//struct steppermotor * motor;

	//motor = data;
	//steppermotor_stop(motor);		//gpio_simplesensor_set_event有问题，并没有把data传入，暂时不改等驱动改好后改过来
	steppermotor_stop(DEV_SCANNER_MOTOR);
}

int Actions_SensorAllSt(int * sensor_st)
{
	int st;
	*sensor_st = 0;
	if(simplesensor_status(DEV_SENSOR_FRONT, &st))
	{
		return -1;
	}
	if(st)
	{
		*sensor_st |= SENSOR_ST_FRONT;
	}
	if(simplesensor_status(DEV_SENSOR_REAR, &st))
	{
		return -1;
	}
	if(st)
	{
		*sensor_st |= SENSOR_ST_REAR;
	}
	if(photosensor_status(DEV_SENSOR_EXSIT, &st))
	{
		return -1;
	}
	if(st)
	{
		*sensor_st |= SENSOR_ST_EXSIT;
	}
	return 0;
}

int Actions_Move(unsigned char dir)
{
	int sensor_st = 0;
	struct steppermotor_config config;
	struct speed_info speed;
	unsigned int timeout = 120;
	int rs = 0;

	if(Actions_SensorAllSt(&sensor_st))
	{
		return -1;
	}
	if((sensor_st & SENSOR_ST_FRONT) && (sensor_st & SENSOR_ST_REAR))
	{
		return -1;
	}
	if(dir)		//forward
	{
		if(sensor_st & SENSOR_ST_FRONT)
		{
			return 0;
		}
		config.speedinfo = &speed;
		config.dir = MOTION_COUNTERCLOCKWISE;
		config.steps_to_run = CIS_MOTION_DISTANCE;
		config.num_speed = 1;
		config.speedinfo->speed = MOVE_SPEED;
		config.speedinfo->steps = CIS_MOTION_DISTANCE;
		config.speedinfo->nextspeed = NULL;
		steppermotor_set_config(DEV_SCANNER_MOTOR, &config);
		if(simplesensor_set_event(DEV_SENSOR_FRONT, SENSOR_EV_DETECTED, sensor_event_handler, DEV_SCANNER_MOTOR))
		{
			return -1;
		}
		
		rs = steppermotor_start(DEV_SCANNER_MOTOR);
	}
	else		//backward
	{
		if(sensor_st & SENSOR_ST_REAR)
		{
			return 0;
		}
		config.speedinfo = &speed;
		config.dir = MOTION_CLOCKWISE;
		config.steps_to_run = CIS_MOTION_DISTANCE;
		config.num_speed = 1;
		config.speedinfo->speed = MOVE_SPEED;
		config.speedinfo->steps = CIS_MOTION_DISTANCE;
		config.speedinfo->nextspeed = NULL;
		steppermotor_set_config(DEV_SCANNER_MOTOR, &config);
		if(simplesensor_set_event(DEV_SENSOR_REAR, SENSOR_EV_DETECTED, sensor_event_handler, DEV_SCANNER_MOTOR))
		{
			return -1;
		}
		
		rs = steppermotor_start(DEV_SCANNER_MOTOR);
	}
	while((steppermotor_status(DEV_SCANNER_MOTOR) != 0) && (timeout))
	{
		vTaskDelay(10/ portTICK_RATE_MS);
		timeout--;
	}

	return rs;
}
