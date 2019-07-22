/*
	actions
*/

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#define SENSOR_ST_FRONT		(1<<0)
#define SENSOR_ST_REAR		(1<<1)
#define SENSOR_ST_EXSIT		(1<<2)

#define MOVE_SPEED_1				1200
#define MOVE_SPEED_2				740 //764
#define MOVE_SPEED				MOVE_SPEED_2

int Actions_SensorAllSt(int * sensor_st);
int Actions_Move(unsigned char dir);

#endif
