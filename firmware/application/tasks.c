/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>


extern int console();


/* Priorities used by the various different tasks. */

#define CONSOLE_TASK_PRIORITY                   (tskIDLE_PRIORITY + 2)

/* task stack size. */

#define CONSOLE_TASK_STACK_SIZE                 500



void ConsoleTask(void * pvParameters)
{
	for (;;)
		console();
}


int create_tasks(void)
{
	portBASE_TYPE rs;

//	tasksync_initialize();

	/* Create tasks. */

	rs = xTaskCreate(ConsoleTask, "Console", CONSOLE_TASK_STACK_SIZE, NULL, CONSOLE_TASK_PRIORITY, NULL);
	if (rs != pdPASS)
		return -1;
	return 0;
}


