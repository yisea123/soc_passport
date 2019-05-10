#ifndef __TASKSYNC_H__
#define __TASKSYNC_H__

#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle sem_ready_supervisor;
extern xSemaphoreHandle sem_ready_scanner;

#define tasksync_ready_supervisor()		xSemaphoreGive(sem_ready_supervisor)
#define tasksync_ready_scanner()		xSemaphoreGive(sem_ready_scanner)

#define tasksync_waitready_supervisor()		xSemaphoreTake(sem_ready_supervisor, 0)
#define tasksync_waitready_scanner()		xSemaphoreTake(sem_ready_scanner, 0)

#endif /* __TASKSYNC_H__ */
