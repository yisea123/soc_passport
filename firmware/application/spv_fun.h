#ifndef __SPV_FUN_H__
#define __SPV_FUN_H__


enum {
	DO_RESET,
	DO_MEDIA_EJECT_GATE,
	DO_MEDIA_EJECT_REAR,
	DO_MEDIA_RETURN_GATE,
	DO_MEDIA_MOVE,
};

extern int spv_action;



extern int spv_do_reset(int mode);
extern int spv_do_sensor_calibration(int unit);
extern int spv_do_scanning(int mode, int timeout);
extern int spv_do_scanning_end(void);
extern int spv_do_scan_calibration(int mode, int scanlines);
extern int spv_do_media_alignment(int timeout);
extern int spv_go_media_alignment(void);
extern int spv_do_media_return(int mode);
extern int spv_do_accept_media(void);
extern int spv_do_scanning_adjustment(void);

#endif /* __SPV_FUN_H__ */
