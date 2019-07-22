/*
	Console Command
*/

//#include "console.h"
#include "console_cmd.h"
#include "steppermotor.h"
#include "photosensor.h"
#include "simplesensor.h"
#include "actions.h"

extern struct steppermotor * steppermotor_list[];
extern struct simplesensor * simplesensor_list[];
extern const int simplesensor_num;
extern struct photosensor * photosensor_list[];
extern const int photosensor_num;

void sensor_front_event_handler(struct simplesensor * sensor, sensor_event_t event, void * data)
{
	struct steppermotor * motor;

	motor = data;
	//steppermotor_stop(motor);		//gpio_simplesensor_set_event有问题，并没有把data传入，暂时不改等驱动改好后改过来
	steppermotor_stop(steppermotor_list[0]);
}

int motor1_run(unsigned char dir, int steps)
{
	struct steppermotor * motor;
	struct steppermotor_config config;
    struct speed_info speed;
	int rs = 0, st = 0;

    config.speedinfo = &speed;
	motor = steppermotor_list[0];
	if(!dir)
	{
		config.dir = MOTION_CLOCKWISE;
	}
	else
	{
		config.dir = MOTION_COUNTERCLOCKWISE;
	}
	config.steps_to_run = steps;
    config.num_speed = 1;
    config.speedinfo->speed = MOVE_SPEED;
    config.speedinfo->steps = steps;
    config.speedinfo->nextspeed = NULL;
	steppermotor_set_config(motor, &config);
	if(simplesensor_set_event(simplesensor_list[0], SENSOR_EV_DETECTED, sensor_front_event_handler, motor))
	{
		return -1;
	}
	if(simplesensor_set_event(simplesensor_list[1], SENSOR_EV_DETECTED, sensor_front_event_handler, motor))
	{
		return -1;
	}
	rs = steppermotor_start(motor);

	return rs;
}

int sensor_test(void)
{
	struct simplesensor * sensor;
	struct photosensor * p_sensor;
	int i, value;
	for(i = 0; i < simplesensor_num; i++)
	{
		sensor = simplesensor_list[i];
		if(simplesensor_status(sensor, &value))
			return -1;
		printf("simplesensor sensor %d, = %d.\r\n", i, value);
	}
	for(i = 0; i < photosensor_num; i++)
	{
		p_sensor = photosensor_list[i];
		if(photosensor_status(p_sensor, &value))
			return -1;
		printf("photosensor sensor %d, = %d.\r\n", i, value);
	}
	return 0;
}


