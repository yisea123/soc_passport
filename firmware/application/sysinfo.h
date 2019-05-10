#ifndef __SYSINFO_H__
#define __SYSINFO_H__

extern unsigned int sysinfo_get_uid_lo(void);
extern unsigned int sysinfo_get_uid_hi(void);
extern int get_log_data(char *fpname, unsigned char *pdata, int len);

#endif /* __SYSINFO_H__ */
