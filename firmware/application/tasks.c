/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>


extern int console();
extern int task_parser(int);
extern int task_supervisor(int);

xSemaphoreHandle sem_ready_supervisor;


/* Priorities used by the various different tasks. */
#define SUPERVISOR_TASK_PRIORITY                (tskIDLE_PRIORITY + 2)
#define PARSER_TASK_PRIORITY                    (tskIDLE_PRIORITY + 2)
#define CONSOLE_TASK_PRIORITY                   (tskIDLE_PRIORITY + 2)

/* task stack size. */
#define SUPERVISOR_TASK_STACK_SIZE         	1000
#define PARSER_TASK_STACK_SIZE                  1000
#define CONSOLE_TASK_STACK_SIZE                 500


static void tasksync_initialize(void)
{
	vSemaphoreCreateBinary(sem_ready_supervisor);
}


void SupervisorTask(void * pvParameters)
{
	task_supervisor(0);
	for (;;);
}


void ParserTask(void * pvParameters)
{
	task_parser(0);
	for (;;);
}


void ConsoleTask(void * pvParameters)
{
	for (;;)
	console();
}


int create_tasks(void)
{
	portBASE_TYPE rs;

	tasksync_initialize();

	/* Create tasks. */
	rs = xTaskCreate(SupervisorTask, "Spv", SUPERVISOR_TASK_STACK_SIZE, NULL, SUPERVISOR_TASK_PRIORITY, NULL);
	if (rs != pdPASS)
		return -1;
	rs = xTaskCreate(ParserTask, "Parser", PARSER_TASK_STACK_SIZE, NULL, PARSER_TASK_PRIORITY, NULL);
	if (rs != pdPASS)
		return -1;
#ifndef NDEBUG
	rs = xTaskCreate(ConsoleTask, "Console", CONSOLE_TASK_STACK_SIZE, NULL, CONSOLE_TASK_PRIORITY, NULL);
	if (rs != pdPASS)
		return -1;
#endif
	return 0;
}


