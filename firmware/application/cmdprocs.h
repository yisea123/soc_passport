/*
 * prototype definition of command functions 
 *
 * Copyright 2016 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __CMDPROCS_H_
#define __CMDPROCS_H_

#include <command.h>

extern int cmd_nop(comm_packet_head_t *cmdhead, void *data);
extern int cmd_undefined(comm_packet_head_t *cmdhead, void *data);

extern int cmd_reset(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_device_info(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_device_error(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_device_partlist(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_device_status(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_sensor_status(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_lifetime_info(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_statistics(comm_packet_head_t *cmdhead, void *data);
extern int cmd_clear_statistics(comm_packet_head_t *cmdhead, void *data); 
extern int cmd_upgrade_firmware(comm_packet_head_t *cmdhead, void *data);
extern int cmd_calibrate_sensor(comm_packet_head_t *cmdhead, void *data);

extern int cmd_move_media(comm_packet_head_t *cmdhead, void *data);
extern int cmd_stop_media(comm_packet_head_t *cmdhead, void *data);
extern int cmd_eject_front(comm_packet_head_t *cmdhead, void *data);
extern int cmd_eject_rear(comm_packet_head_t *cmdhead, void *data);
extern int cmd_return_front(comm_packet_head_t *cmdhead, void *data);
extern int cmd_media_alignment(comm_packet_head_t *cmdhead, void *data);
extern int cmd_enter_wait_alignment(comm_packet_head_t *cmdhead, void *data);
extern int cmd_open_aligner(comm_packet_head_t *cmdhead, void *data);
extern int cmd_close_aligner(comm_packet_head_t *cmdhead, void *data);
extern int cmd_start_scanning(comm_packet_head_t *cmdhead, void *data);
extern int cmd_stop_scanning(comm_packet_head_t *cmdhead, void *data);

extern int cmd_reset_scanunit(comm_packet_head_t *cmdhead, void *data);
extern int cmd_set_scan_mode(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_scan_feature(comm_packet_head_t *cmdhead, void *data);
extern int cmd_get_image_calib_data(comm_packet_head_t *cmdhead, void *data);
extern int cmd_set_image_calib_data(comm_packet_head_t *cmdhead, void *data);
extern int cmd_scanning_adjustment(comm_packet_head_t *cmdhead, void *data);
extern int cmd_scanning_calibration(comm_packet_head_t *cmdhead, void *data);
extern int cmd_scan_calibrate_data(comm_packet_head_t *cmdhead, void *data);
extern int cmd_dummy_scanning(comm_packet_head_t *cmdhead, void *data);

extern int cmd_reset_acceptor(comm_packet_head_t *cmdhead, void *data);
extern int cmd_accept_media(comm_packet_head_t *cmdhead, void *data);

extern int cmd_get_img_exposure_time(comm_packet_head_t *cmdhead, void *data);
extern int cmd_set_img_exposure_time(comm_packet_head_t *cmdhead, void *data);

extern int cmd_get_img_afe_gain(comm_packet_head_t *cmdhead, void *data);
extern int cmd_set_img_afe_gain(comm_packet_head_t *cmdhead, void *data);

extern int cmd_get_clocktime(comm_packet_head_t *cmdhead, void *data);
extern int cmd_set_clocktime(comm_packet_head_t *cmdhead, void *data);

extern int cmd_get_log(comm_packet_head_t *cmdhead, void *data);
#endif /* __CMDPROCS_H_ */

