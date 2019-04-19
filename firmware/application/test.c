
#include "FreeRTOS.h"
#include "task.h"

#ifdef __ICCARM__
#include <yfuns.h>
#endif
#include <stdio.h>
#include <string.h>

#include "m2sxxx.h"

#include "nvram.h"

extern int board_configure(void);
extern int nvmdata_initialize();


int main()
{

	int rs = 0;

	board_configure();

	SYSREG->WDOG_CR = 0x0000;        /*Disable Watch-dog*/

	printf("\r\nNvram test is running\r\n");
	printf("\r\n");
        /*nvmdata ≥ı ºªØ*/
	nvmdata_initialize(); 


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
