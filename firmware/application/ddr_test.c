#include <stdio.h>
#include <string.h>

#include <stdint.h>

#include <stdlib.h>

void ddr_memory_test()
{
	uint8_t *ptr = (uint8_t *)0xa0000000;

	memset(ptr, 0xff, 0x100000);
	memset(ptr, 0x55, 0x100000);
	memset(ptr+0x100000, 0xaa, 0x100000);
//	memset(ptr, 0x00, 0x100000);
}

