#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <string.h>

#define LOGGING

#ifdef DEBUG
#define dbg_printf(format, ...)		fprintf(stdout, format, ##__VA_ARGS__)
#else
#define dbg_printf(format, ...)
#endif


#ifdef LOGGING
#define log_err(format, ...)		syslog(LOG_ERR, format, ##__VA_ARGS__)
#define log_warning(format, ...)	syslog(LOG_WARNING, format, ##__VA_ARGS__)
#define log_notice(format, ...)		syslog(LOG_NOTICE, format, ##__VA_ARGS__)
#define log_info(format, ...)		syslog(LOG_INFO, format, ##__VA_ARGS__)
#define log_debug(format, ...)		syslog(LOG_DEBUG, format, ##__VA_ARGS__)
#else
#define log_err(format, ...)
#define log_warning(format, ...)
#define log_notice(format, ...)
#define log_info(format, ...)
#define log_debug(format, ...)
#endif


#ifdef LOGGING
static inline void log_data(void *data, size_t count, size_t limit)
{
	char buffer[128];
	unsigned char *dptr;

	dptr = data;
	if (limit > 0 && count > limit)
		count = limit;
	while (count > 0)
	{
		int len = 0;
		char *ptr = buffer;
		while ((len+3) < sizeof(buffer) && count > 0)
		{
			int n = sprintf(ptr, "%02X ", *dptr++);
			if (n < 0)
				return;
			ptr += n;
			len += n;
			--count;
		}
//		syslog(LOG_INFO, "%s", buffer);
	}
}
#else
static inline void log_data(void *data, size_t count, size_t limit)
{
}
#endif


#endif /* __DEBUG_H__ */
