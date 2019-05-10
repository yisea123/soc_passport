#ifndef __SPV_STATES_H__
#define __SPV_STATES_H__


typedef void * (*spv_stat_fun)(void);


/* definition of supervisor state functions */
extern void * spv_state_init(void);
extern void * spv_state_idle(void);
extern void * spv_state_reset(void);
extern void * spv_state_error(void);
extern void * spv_state_scanning(void);
extern void * spv_state_loading(void);
extern void * spv_state_ready(void);
extern void * spv_state_return(void);
extern void * spv_state_calibrate(void);
extern void * spv_state_scanning_adjustment(void);

/* definition of supervisor synchronizing functions */
extern int spv_is_state(void * state);
extern int spv_change_state(void * state);
extern int spv_sync_reset();
extern int spv_sync_wait();
extern int spv_sync_done();



//-------------------------------------------------------------------------
#endif /* __SPV_STATES_H__ */
