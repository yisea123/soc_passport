#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

#include "config.h"
#include "debug.h"

#define RESET_CONFIG_FILE_NAME "/home/config/reset_mode.dat"
device_config_t device_config;

int save_reset_config(int mode)
{
	int rs,fp;
	unsigned char value;
	value = mode;

	fp = open(RESET_CONFIG_FILE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fp)
		rs = write(fp, (void *)&value, sizeof(value)); 
	else
		return -1;

        close(fp);
	return 0;
}


int read_reset_config(void)
{
	int rs,fp,mode;
	unsigned char value;

	mode = RESET_EJECT_MEDIA;

	rs = access(RESET_CONFIG_FILE_NAME, F_OK);
	if (!rs) {
		fp = open(RESET_CONFIG_FILE_NAME,O_RDONLY);
		if (fp)
			rs = read(fp, (void *)&value, sizeof(value));
		if (rs == 1)
			mode = value;
	}

        close(fp);
	return mode;
}
