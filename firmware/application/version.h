#ifndef __VERSION_H__
#define __VERSION_H__


#define bcd2bin(x)    (((x) & 0x0f) + ((x) >> 4) * 10)
#define bin2bcd(x)    ((((x) / 10) << 4) + (x) % 10)

/* hardware version numbers */
#define HW_MAJOR_VERSION	01
#define HW_MINOR_VERSION	02

/* firmware version numbers */
#define FW_MAJOR_VERSION	01
#define FW_MINOR_VERSION	03

/* version number in 16-bits BCD code */
#define HW_VERSION_BCD	(((unsigned short)(bin2bcd(HW_MAJOR_VERSION))<<8) + bin2bcd(HW_MINOR_VERSION))
#define FW_VERSION_BCD	(((unsigned short)(bin2bcd(FW_MAJOR_VERSION))<<8) + bin2bcd(FW_MINOR_VERSION))


#define version_major(v)	bcd2bin((v) >> 8)
#define version_minor(v)	bcd2bin((v) & 0xff)

/* version number in 4-bytes ASCII string */
#define VERSION_TXT	()

extern int version_check_result(void);
extern void image_version_get(void);
#endif /* __VERSION_H__ */
