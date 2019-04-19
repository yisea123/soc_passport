/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "mss_gpio.h"
#include "indicator.h"


#include <stdio.h>
#include <string.h>

#define usleep(a)		{uint32_t i; for(i=0; i<a; i++);}
#define ON                      0
#define OFF                     1

void write_led_attrib(int id, int val)
{   
    switch (id) 
    {
    case INDICATOR_0:
	MSS_GPIO_set_output(MSS_GPIO_2, val);
        break;
    }  
}

void indicator_set(int id, int state)
{
    switch (state) 
    {
    case INDICATOR_ON:
        write_led_attrib(id, ON);
        break;
    case INDICATOR_OFF:
        write_led_attrib(id, OFF);
        break;
    case INDICATOR_BLINK_FAST:
        for (;;) {
            write_led_attrib(id, ON);
            usleep(100000);
            write_led_attrib(id, OFF);
            usleep(100000);
        }
        break;
    case INDICATOR_BLINK_SLOW:
        for (;;) {
            write_led_attrib(id, ON);
            usleep(500000);
            write_led_attrib(id, OFF);
            usleep(500000);
        }
        break;
    }
}

void indicator_flash_set(int id, int num)
{
    int i;

    while (1) {
        write_led_attrib(id, OFF);
        usleep(2000000);
        for (i = 0; i < num; i++) {
            usleep(500000);
            write_led_attrib(id, ON);
            usleep(500000);
            write_led_attrib(id, OFF);
        }
    }
}
