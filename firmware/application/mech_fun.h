#ifndef __MECH_FUN_H__
#define __MECH_FUN_H__

#define MAX_CMD_LEN 512

extern int com_mach_help(char *para);
extern int com_mach_quit(char *para);
extern int com_sen_feature_get(char *para);
extern int com_sen_config_init(char *para);
extern int com_sen_config_load(char *para);
extern int com_sen_mask_show(char *para);
extern int com_sen_pwm_set(char *para);
extern int com_sen_val_get(char *para);
extern int com_sen_enable(char *para);
extern int com_sen_enable_all(char *para);
extern int com_sen_set_pwm_all(char *para);
extern int com_status_get(char *para);
extern int com_sen_raw_get(char *para);
extern int com_mech_sen_raw_print_to_file(char *para);
extern int com_paper_init(char *para);
extern int com_paper_scan(char *para);
extern int com_paper_step(char *para);
extern int com_paper_align(char *para);
extern int com_paper_eject_rear(char *para);
extern int com_paper_park_gate(char *para);
extern int com_paper_stop(char *para);
extern int com_paper_load(char *para);
extern int com_paper_eject_gate(char *para);
extern int com_align_open_close(char *para);
extern int com_accept_init(char *para);
extern int com_acceptor_pusher_reset(char *para);
extern int com_acceptor_pusher_push(char *para);
extern int com_acceptor_pusher_stop(char *para);
extern int com_acceptor_pusher_push_reset(char *para);
extern int com_paper_sen_calibrate_reset(char *para);
extern int com_paper_sen_calibrate_withpaper(char *para);
extern int com_paper_sen_calibrate_withoutpaper(char *para);
extern int com_paper_sen_calibrate(char *para);
extern int com_acceptor_sen_calibrate_withpaper(char *para);
extern int com_acceptor_sen_calibrate_withoutpaper(char *para);
extern int com_acceptor_sen_calibrate_reset(char *para);
extern int com_acceptor_sen_calibrate(char *para);
extern int com_fpga_motor_skew_set(char *para);
extern int com_paper_scantriger_status_get(char *para);
extern int com_demo_park_cycle(char *para);
extern int com_demo_noeject(char *para);
extern int com_demo_cycle(char *para);
extern int com_paper_scan_multithread(char *para);
extern int com_demo_calibrate(char *para);
extern int com_mech_sen_test_bias(char *para);
extern int com_mech_sen_test_effect_open_close(char *para);
extern int com_mech_sen_test_effect_zero(char *para);
extern int com_mech_sen_test_effect_close_advance(char *para);
#endif
