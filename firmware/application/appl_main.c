/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

#include "version.h"
#include "usbdrv.h"

#include "m2sxxx.h"
#include "indicator.h"

extern int board_configure(void);
extern int create_tasks(void);
extern int nvmdata_initialize();


void ddr_memory_test()
{
	uint8_t *ptr = (uint8_t *)0xa0000000;

	memset(ptr, 0xff, 0x100000);
	memset(ptr, 0x55, 0x100000);
	memset(ptr+0x100000, 0xaa, 0x100000);
	memset(ptr, 0x00, 0x100000);
}


int main()
{
	board_configure();

	SYSREG->WDOG_CR = 0x0000;        /*Disable Watch-dog*/

	printf("\r\n Multiboxes Firmware V0.10\r\n");
	printf("\r\n");

	ddr_memory_test();

	nvmdata_initialize();

	usbd_install(NULL);

	create_tasks();

        /* Start the tasks and timer running. */
       vTaskStartScheduler();

	/*
	* If all is well, the scheduler will now be running, and the following line
	* will never be reached.  If the following line does execute, then there
	* was insufficient FreeRTOS heap memory available for the idle and/or timer
	* tasks to be created.  See the memory management section on the FreeRTOS
	* web site for more details.
	*/
	for (;;);
}
