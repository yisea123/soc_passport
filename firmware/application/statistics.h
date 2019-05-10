#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <information.h>
extern dev_statistics_t   dev_statistics; 

#define STATISTICS_ACCEPT_CNT_TMP	0x00
#define STATISTICS_SCAN_CNT_TMP		0x01
#define STATISTICS_ERR_CNT_TMP		0x02
#define STATISTICS_CIS_ONTIME_TMP	0x03

#define STATISTICS_ACCEPT_CNT      	(DEV_STATISTICS_NVM_INDEX+STATISTICS_ACCEPT_CNT_TMP)
#define STATISTICS_SCAN_CNT      	(DEV_STATISTICS_NVM_INDEX+STATISTICS_SCAN_CNT_TMP)
#define STATISTICS_ERR_CNT		(DEV_STATISTICS_NVM_INDEX+STATISTICS_ERR_CNT_TMP)
#define STATISTICS_CIS_ONTIME		(DEV_STATISTICS_NVM_INDEX+STATISTICS_CIS_ONTIME_TMP)

#define STATISTICS_PWON_CNT		(STATISTICS_CIS_ONTIME+1)

extern int statistics_initialize(void);
extern int statistics_save(void);
extern int statistics_set(int index, unsigned int val);
extern int statistics_add(int index, unsigned int val);
extern int statistics_inc(int index);
extern int statistics_clear(int index);

#endif
