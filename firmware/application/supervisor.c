#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "card_scanpath_drv.h"
#include "card_scanpath_dev.h"

#include "media.h"
#include "spv_states.h"
#include "scanner.h"
#include "spv_fun.h"
#include "tasksync.h"
#include "mechdev.h"
#include "command.h"
#include "gpiokey.h"
#include "indicator.h"

#define LOGGING
#include "debug.h"

extern int scanner_initialize();
extern struct gpiokey gpiokey_list[];

static void *spv_state, *new_spv_state;

xSemaphoreHandle sem_spv = NULL, sem_spv_done = NULL;

int media_state = MEDIA_UNAVAILABLE;
volatile int spv_status = 0;
struct gpiokey *key_force_eject = &gpiokey_list[0];


int get_key(void)
{

    if (gpiokey_is_active(key_force_eject)) {
            vTaskDelay(3000/portTICK_RATE_MS);
            if (gpiokey_is_active(key_force_eject))
            {  
                 return gpiokey_is_active(key_force_eject);
                 
            }
            else{
                return 0;
            }
    }
     return 0;       
}

void mechanism_initialize()
{
//	cardpath_init();
}


int task_supervisor(int arg)
{
	vSemaphoreCreateBinary(sem_spv);
	vSemaphoreCreateBinary(sem_spv_done);
	if (sem_spv == NULL || sem_spv_done == NULL)
//		log_err("Failed creating semaphore in task_supervisor\r\n");

//	mechanism_initialize();
	scanner_initialize();

	new_spv_state = NULL;
//	spv_state = (void *)spv_state_init;
//	while (spv_state)
//	{
//		spv_state = (*((spv_stat_fun)spv_state))();
//	}
//	log_notice("task supervisor exit!\n");
	return 0;
}


void spv_update_media_status(void)
{
	if (cardpath_is_in_path())
		media_state = 0;
	else if (cardpath_is_at_front())
		media_state = MEDIA_AT_FRONT;
	else
		media_state = MEDIA_UNAVAILABLE;
}


unsigned short spv_get_sensor_status(void)
{
	unsigned short gstatus = 0, rstatus = 0;
	gstatus = cardpath_sensor_status();
/*	if(gstatus & CARDPATH_CD_SENSOR_MASK)
		rstatus |= IAM_FRONT_SENSOR;
	if(gstatus & CARDPATH_CS0_SENSOR_MASK)
		rstatus |= IAM_BEHIND_SENSOR;
#ifdef IC61S
	if(gstatus & CARDPATH_CS1_SENSOR_MASK)
		rstatus |= IAM_BEHIND_SPACE;
	
	if(gstatus & CARDPATH_CS2_SENSOR_MASK)
		rstatus |= IAM_BEHIND_END;
#endif
#ifdef IC62S_4M
	if(gstatus & CARDPATH_CS1_SENSOR_MASK)
		rstatus |= IAM_BEHIND_END;
#endif
	if(gstatus & CARDPATH_COVER_SENSOR_MASK)
		rstatus |= IAM_COVER_OPEN;

	if (cardpath_is_in_path())
		indicator_turn_on(INDICATOR_0);
	else
		indicator_turn_off(INDICATOR_0); */

	return rstatus;
}


void * spv_wait_order()
{
	uint32_t timeout = 200/portTICK_RATE_MS;	// timeout = 200 microsecond

	return (xSemaphoreTake(sem_spv, timeout) == pdFALSE) ? NULL : new_spv_state;
}


int spv_change_state(void * state)
{
	new_spv_state = state;
	xSemaphoreGive(sem_spv);
	return 0;
}


int spv_is_state(void * state)
{
	return (spv_state == state);
}


int spv_sync_reset(void)
{
	uint32_t timeout = 1/portTICK_RATE_MS;	// timeout = 1 microsecond
	xSemaphoreTake(sem_spv_done, timeout);	// make sure synchronous semaphore is reset to 0
	return 0;
}


int spv_sync_wait(void)
{
	xSemaphoreTake(sem_spv_done, 0);
	return 0;
}

int spv_sync_done(void)
{
	xSemaphoreGive(sem_spv_done);
	return 0;
}


void * spv_state_init(void)
{
	void * nextstate;

//	log_notice("Spv entering INIT state!\r\n");

	// system initialization sequence

	nextstate = spv_state_reset();	// perform power-on reset

	tasksync_ready_supervisor();

	return nextstate;
}


void * spv_state_idle(void)
{
	void * nextstate;
	int rs;
//	log_notice("Spv entering IDLE state!\r\n");

	while (1) {
		nextstate = spv_wait_order();
		if (nextstate)
			return nextstate;

		if (cardpath_is_coveropen())
			return (void *)spv_state_error;

		if (cardpath_is_in_path())
			indicator_turn_on(INDICATOR_0);
		else
			indicator_turn_off(INDICATOR_0);

		rs = get_key();
		if(rs > 0)
		{
			rs = cardpath_eject_gate();
			if (rs == -1) {
				nextstate = (void *)spv_state_error;
			}
		}
	}
	return NULL;
}


void * spv_state_error(void)
{
	int cnt = 0;

//	log_notice("Spv entering ERROR state!\r\n");
	while (1) {
		void * nextstate;
		nextstate = spv_wait_order();
		if (nextstate == (void *)spv_state_reset)
				return nextstate;

		if (cardpath_is_coveropen())
                {
			cnt = 0;
                        indicator_turn_on(INDICATOR_0);
                }
		else if (++cnt == 2)
		{
			if ((cardpath_is_in_path())||(cardpath_is_at_cd()))
				cardpath_eject_gate();
                                indicator_turn_off(INDICATOR_0);
			return (void *)spv_state_idle;
		}
	}
	return NULL;
}

