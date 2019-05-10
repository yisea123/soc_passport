#ifndef __CMD_CARDPATH_TESTPROCS_H__
#define __CMD_CARDPATH_TESTPROCS_H__

#include "command.h"

extern int cmd_test_sen_feature_get(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_sen_enable(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_sen_disable(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_mech_status_get(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_card_step(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_card_scan(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_card_eject(comm_packet_head_t *cmdhead, void *data);
extern int cmd_test_card_intake(comm_packet_head_t *cmdhead, void *data);

#endif
