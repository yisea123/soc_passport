#ifndef __MISC_H__
#define __MISC_H__

extern int misc_set_skew_to_file(int value);
extern int misc_read_skew_from_file(void);
extern int misc_read_eim_bclk_feq(void);
extern void misc_paper_skew_init(void);
extern void misc_upgrade_firmware(void);

#endif /* __MISC_H__ */
