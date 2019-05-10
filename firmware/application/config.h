#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <command.h>

#define RESET_NO_ACTION		RESET_MODE0
#define RESET_RETURN_MEDIA	RESET_MODE1
#define RESET_EJECT_MEDIA	RESET_MODE2
#define RESET_APCCEPT_MEDIA	RESET_MODE3

typedef struct {
	unsigned char   reset_mode;
}device_config_t;

extern device_config_t device_config;

extern int save_reset_config(int mode);
extern int read_reset_config(void);
#endif /* __CONFIG_H__ */
