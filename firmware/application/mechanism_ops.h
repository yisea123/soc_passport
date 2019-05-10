#ifndef __MECHANISM_OPS_H__
#define __MECHANISM_OPS_H__

#include "mechlib.h"
//#include "mechdrv.h"

#define MECHDRVLIB_API

/*

*/
//#define PPS_DRV_THRESHHOLD	(unsigned short)3200	//3200*0.61=1952mv
#define PPS_DRV_THRESHHOLD	(unsigned short)2000	//2000*0.61=1220mv

#define PPS_MIN_INPUT   (unsigned short)300//64
//#define	PPS_THR_DEFAULT	(unsigned short)0x800 	//2048
#define	PPS_THR_DEFAULT	(unsigned short)500//256//(unsigned short)512//(unsigned short)1024	//just for debug by hl 2016.11.30

#define SEN_CALIBRATED_NONE 0
#define SEN_CALIBRATED_DONE 1
#define SEN_CALIBRATED_ERR  2
#define SEN_CALIBRATED_ZEROERR  3

#define SEN_CAL_BLANK   0
#define SEN_CAL_WITHOUT_MEDIA   1
#define SEN_CAL_WITH_MEDIA  2

#define SEN_CAL_STEP    5
typedef struct{
    unsigned int sen_mask;
    unsigned short scope_min;
    unsigned short scope_max;
    unsigned short scope_low;
    unsigned short scope_high;
    unsigned short scope_tmp;
    unsigned char  calibrate_dir;
    unsigned char  val_result[500]; 
}sen_calibrate_t;
//------------------------------------------------------------
typedef struct{
	unsigned int sen_mask;
	char sen_open_status;		//1——opened; 0——closed
}mech_sen_status_t;

//-----------------------------------------------------------
typedef struct{
	unsigned int sen_mask;
	unsigned int sen_pos_index;
}sen_mask_pos_t;

typedef struct{
	//int *fd;
	struct mechanism_dev_t	*pmechanism_dev;
	unsigned long eflag;
	int oflag;
	unsigned long sigio_flag;
	//mech_drv_t *pmech_drv;
	void (*sigio_handle)(int);
	void (*synchro_handle)(void);
}mech_sigio_t;

typedef struct{
	//int *fd;
	struct mechanism_dev_t	*pmechanism_dev;
	mechunit_feature_t mech_feature;

	mech_sen_status_t	*mech_sen_status;//[MECHUNIT_SENSOR_NUM_MAX];

	mech_unit_sen_config_t *punit_sen_config; 
	char sen_config_inited;

	mech_unit_sen_feature_t *punit_sen_feature; 
	char sen_feature_inited;

	mech_unit_motor_feature_t *punit_motor_feature;
	char motor_feature_inited;

	mech_unit_sen_raw_input_t *punit_sen_raw_input;

	//void * 	uint_status;
	mech_control_t unit_control;
	mech_sigio_t	*pmech_sigio;

	sen_mask_pos_t *sen_mask_pos;
	unsigned char sen_pos_num;

//	mech_unit_info_t unit_motor_info;
//	mech_unit_info_t unit_sensor_info;
}mech_drv_t;

typedef struct{
	mech_drv_t	*mech_drv;
	/*-------------------------------------------------------------
		mech_feature_get
 
        description:
		get the motor_num & sensor_num of the pmech_drv
		note: Must executed before sensor/motor operations at initialize state
	return:
		0: OK
		other:error
	-------------------------------------------------------------*/
	int (*mech_feature_get)(mech_drv_t *);	//

	/*-------------------------------------------------------------
		mech_sen_feature_get
 
        description:
		note: Must executed before other sensor operations
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
	int (*mech_sen_feature_get)(mech_drv_t *pmech_drv);	//

	/*-------------------------------------------------------------
		mech_sen_config_init
 
        description:
		note：Must executed at the initial stage
	return:
		0: OK
		other:error
	-------------------------------------------------------------*/
	int (*mech_sen_config_init)(mech_drv_t *);	//

	/*-------------------------------------------------------------
		mech_sen_config_load
 
	description: 
	return:
		0: OK
		other:error
	-------------------------------------------------------------*/
	int (*mech_sen_config_load)(mech_drv_t *); //

	/*------------------------------------------------ 	
        mech_sen_raw_input_get
	return: 0: OK 
		other: error
	------------------------------------------------ */
	int (* sen_raw_input_get)(mech_drv_t *, char );
	/*------------------------------------------------ 	
        mech_sen_val_get
        return: 0: OK
        	other: error
	------------------------------------------------ */	
	int (* sen_val_get)(mech_drv_t *pmech_drv, unsigned int sen_mask, unsigned long *val);
	/*------------------------------------------------ 	
        sen_raw_print_to_file
        description:
        	print the sensors' raw value of mech unit into files with name "/tmp/sensor+mech_name+sensor_index.txt"
		drv_val is 0, 5, 10,......250
        	in[1]~in[10] are the raw values according sample order, and in[0] is the average of in[1]~in[10].
	param:
        	flag：0——without paper
		      1——with paper
        return:
        	0: OK
        	other: error
	------------------------------------------------ */	
	int (*sen_raw_print_to_file)(mech_drv_t *pmech_drv, char flag);
	/*-------------------------------------------------------------
	sensor_is_enabled
	return:0——OK
	other——err
	-------------------------------------------------------------*/ 
	int (*sen_is_enabled)(mech_drv_t *pmech_drv, unsigned int sen_mask, char *penabled);
	/*-------------------------------------------------------------
        sen_pwm_set
	return: 0: OK
	        other: error
	-------------------------------------------------------------*/
	int (*sen_pwm_set)(mech_drv_t *pmech_drv,unsigned int mask, unsigned long pps_drv_value);

	/*-------------------------------------------------------------
		sen_drv_ref_set
	return:
		0: OK
		other:error
	-------------------------------------------------------------*/
	int (*sen_drv_ref_set)(mech_drv_t *pmech_drv, unsigned int mask, unsigned long pps_ref_value, unsigned long pps_drv_value);

	/*-------------------------------------------------------------
        sensors_enable
	description:
	    enable/disable specified sensors
        return:
		0: OK
	        other: error
	-------------------------------------------------------------*/ 
	int (* sensors_enable)(mech_drv_t *pmech_drv, unsigned int sen_masks, unsigned char enable);

	/*-------------------------------------------------------------
        sensor_enable_all
	description:
	    enable/disable all sensors
        return:
		0: OK
	        other: error
	-------------------------------------------------------------*/ 
	int (* sensor_enable_all)(mech_drv_t *pmech_drv, unsigned char enable);

	/*-------------------------------------------------------------
	mech_status_get 
	description: 
		showflag:1——print status info on the console 
	-------------------------------------------------------------*/
	int (*mech_status_get)(mech_drv_t *pmech_drv, char showflag);
	/*------------------------------------------------ 	
        sen_info_get
        return:
        	0: OK
        	other: error
	------------------------------------------------ */
	int (*sen_info_get)(mech_drv_t *pmech_drv);
	/*------------------------------------------------ 	
        motor_get_info
        return:
        	0: OK
        	other: error
	------------------------------------------------ */
	//int (*motor_get_info)(mech_drv_t *pmech_drv,mech_unit_info_t *p_mech_unit_info);
	int (*motor_get_info)(mech_drv_t *pmech_drv);
}mech_operation_t;

extern mech_operation_t mech_operation;
//================================================================
/*-------------------------------------------------------------
		mech_feature_get
 
        description:
		get the motor_num & sensor_num of the pmech_drv
		note: Must executed before sensor/motor operations at initialize state
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_feature_get(mech_drv_t *pmech_drv);

/*-------------------------------------------------------------
		mech_sen_feature_get
 
        description:
		note: Must executed before other sensor operations
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_sen_feature_get(mech_drv_t *pmech_drv);

/*-------------------------------------------------------------
		mech_sen_config_init
 
        description:
		note：Must executed at the initial stage
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_sen_config_init(mech_drv_t *pmech_drv);

/*-------------------------------------------------------------
		mech_init
 
        description:
		note：Must executed at the initial stage
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_init(mech_drv_t *pmech_drv);
//=============================================================
/*-------------------------------------------------------------
		mech_sen_config_set
 
	description: 
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_sen_config_set(mech_drv_t *pmech_drv, sen_config_t *p_sen_config);

/*-------------------------------------------------------------
		mech_sen_drv_ref_set
 
	description: 
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_sen_drv_ref_set(mech_drv_t *pmech_drv, unsigned int mask, unsigned long pps_ref_value, unsigned long pps_drv_value);

/*-------------------------------------------------------------
        mech_sen_pwm_set
	return:
		0: OK
	        other: error
-------------------------------------------------------------*/
extern int mech_sen_pwm_set(mech_drv_t *pmech_drv,unsigned int mask, unsigned long pps_drv_value);

/*-------------------------------------------------------------
		mech_sen_config_load
 
	description: 
	return:
		0: OK
		other:error
-------------------------------------------------------------*/
extern int mech_sen_config_load(mech_drv_t *pmech_drv);

/*------------------------------------------------ 	
        mech_sen_inited
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_sen_inited(mech_drv_t *pmech_drv);

/*------------------------------------------------ 	
        mech_motor_inited
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_motor_inited(mech_drv_t *pmech_drv);

/*------------------------------------------------ 	
        mech_sen_get_info
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_sen_get_info(mech_drv_t *pmech_drv);

/*------------------------------------------------ 	
        mech_motor_get_info
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_motor_get_info(mech_drv_t *pmech_drv);
#if 0

/*------------------------------------------------ 	
        mech_ioctl
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_ioctl(mech_drv_t *pmech_drv, int request, void *arg);
#endif
/*------------------------------------------------ 	
        mech_status_get
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_status_get(mech_drv_t *pmech_drv, char showflag);

/*-------------------------------------------------------------
        mech_sensors_enable
	description:
	    enable/disable specified sensors
        return:
		0: OK
	        other: error
-------------------------------------------------------------*/ 
extern int mech_sensors_enable(mech_drv_t *pmech_drv, unsigned int sen_masks, unsigned char enable);

/*-------------------------------------------------------------
        mech_sensor_enable_all
	description:
	    enable/disable all sensors
        return:
		0: OK
	        other: error
-------------------------------------------------------------*/ 
extern int mech_sensor_enable_all(mech_drv_t *pmech_drv, unsigned char enable);

/*-------------------------------------------------------------
    mech_sen_is_enabled
    return:0——OK
	other——err
-------------------------------------------------------------*/ 
extern int mech_sen_is_enabled(mech_drv_t *pmech_drv, unsigned int sen_mask, char *penabled);

/*------------------------------------------------ 	
        mech_sen_raw_input_get
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_sen_raw_input_get(mech_drv_t *pmech_drv, char showflag);
	
/*------------------------------------------------ 	
        mech_sen_val_get
        return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_sen_val_get(mech_drv_t *pmech_drv, unsigned int sen_mask, unsigned long *val);

/*------------------------------------------------ 	
        mech_sen_raw_print_to_file
        description:
        	print the sensors' raw value of mech unit into files with name "/tmp/sensor+mech_name+sensor_index.txt"
		drv_val is 0, 5, 10,......250
        	in[1]~in[10] are the raw values according sample order, and in[0] is the average of in[1]~in[10].
	param:
        	flag：0——without paper
		      1——with paper
        return:
        	0: OK
        	other: error
------------------------------------------------ */	
extern int mech_sen_raw_print_to_file(mech_drv_t *pmech_drv, char flag);

/*============================================================
        sensors calibrate API
        note:
	mech_sen_calibrate_reset() will clear calibrate status of all sensors of cardpath.
        for counter type sensors, calibrate without paper.
        for reflection type sensors, calibrate with paper.
============================================================*/ 
/*-------------------------------------------------------------
        mech_sen_calibrate_reset
 
	description:
        	clear calibrate status of all sensors of cardpath.
        return:
	        none
-------------------------------------------------------------*/
extern void mech_sen_calibrate_reset(mech_drv_t *pmech_drv);

/*------------------------------------------------ 	
        mech_sen_calibrate_save
        description:
        	save  pps_drv_value to "/tmp/sensor_cal_+mech_name.txt"
	return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_sen_calibrate_save(mech_drv_t *pmech_drv);

/*------------------------------------------------ 	
        mech_sen_calibrate_load
        description:
        	get  pps_drv_value from "/tmp/sensor_cal_+mech_name.txt"
	return:
        	0: OK
        	other: error
------------------------------------------------ */
extern int mech_sen_calibrate_load(mech_drv_t *pmech_drv);

extern int mech_sen_calibrate_black_test(mech_drv_t *pmech_drv, unsigned int sen_mask, unsigned int sen_masks);
extern int mech_sen_calibrate_get_withoutmedia_curve(mech_drv_t *pmech_drv, unsigned char index, unsigned int sen_mask);
extern int mech_sen_calibrate_get_withmedia_curve(mech_drv_t *pmech_drv, unsigned char index, unsigned int sen_mask);
extern int mech_sen_calibrate_get_val(mech_drv_t *pmech_drv, unsigned char index, unsigned int sen_mask);

extern int mech_match_sen_calibrate_withoutpaper(mech_drv_t *pmech_drv, unsigned char index, unsigned int sen_mask);
extern int mech_reflective_sen_calibrate_withpaper(mech_drv_t *pmech_drv, unsigned char index, unsigned int sen_mask);
//---------------------------------------------------
#if 0
/*------------------------------------------------ 
	mech_synchro_ctrl
        return:
		0: OK
		other:error
------------------------------------------------*/
extern int mech_synchro_ctrl(mech_sigio_t *pmech_sigio, void *pctrl);
#else
#if 0
/*------------------------------------------------ 
	mech_ctrl
		return:
		0: OK
		other:error
------------------------------------------------*/
extern int mech_ctrl(struct mechanism_dev_t *pmechanism_dev, void *pctrl);
#endif		
#endif

/*------------------------------------------------ 
	mech_sigio_get
        return:
		0: OK
		other:error
------------------------------------------------*/
extern int mech_sigio_get(mech_drv_t *pmech_drv, mech_sigio_t *pmech_sigio);


//---------------------------------------------------
/* 
	NOTE： 
        1.after this triger，do not  triger interrupt , and move continue util total steps done：
        	to_trigger_steps——0，
        	sen_mask——0
        2.after this triger，stop.Disregard the total steps & sensor status.
        	sen_mask——0
		motor_trigger_stop_flag——MOTOR_TRIGGER_STOP
*/ 
static inline void motor_triger_phase_build(motor_trigger_phase_t * p,  unsigned long  to_trigger_steps	, unsigned int sen_mask, 
	unsigned char motor_sen_flag, unsigned short motor_triger_flag) 
{ 
	p->to_trigger_steps = to_trigger_steps; 
	p->sen_mask = sen_mask; 
	p->motor_sen_flag = motor_sen_flag; 
	//(unsigned short)(p->motor_triger_flag) = motor_triger_flag; 
	p->motor_triger_flag.motor_trigger_stop_flag = (motor_triger_flag & MOTOR_TRIGER_STOP_FLAG_MASK)?1:0;
	p->motor_triger_flag.motor_trigger_condition_flag = (motor_triger_flag & MOTOR_TRIGER_SENSOR_FLAG_MASK)?1:0;
	p->motor_triger_flag.motor_sensor_stop_flag = (motor_triger_flag & MOTOR_SENSOR_STOP_FLAG_MASK)?1:0;
	p->motor_triger_flag.motor_sensor_continue_mode = (motor_triger_flag & MOTOR_SENSOR_CONTINUE_MODE_MASK)?1:0;
	p->motor_triger_flag.motor_sensor_stop_mode = (motor_triger_flag & MOTOR_SENSOR_STOP_MODE_MASK)?1:0;
	p->sen_pos_index = 0;

}

static inline void motor_triger_phase_senposindex_fill(mech_drv_t *pmech_drv, motor_mov_t *pmotor_mov) 
{
	char i, j;

	//printf("motor_triger_phase_senposindex_fill0: sen_pos_num=%d\n", pmech_drv->sen_pos_num);

	for (i=0; i < pmotor_mov->trigger_phase_num; i++) {
		for (j=0; j < pmech_drv->sen_pos_num; j++) {
			if (pmotor_mov->motor_trigger_phase[i].sen_mask == pmech_drv->sen_mask_pos[j].sen_mask) {
				pmotor_mov->motor_trigger_phase[i].sen_pos_index = pmech_drv->sen_mask_pos[j].sen_pos_index;
		//		printf("motor_triger_phase_senposindex_fill1:sen_mask=%x sen_pos_index=%x\n", pmotor_mov->motor_trigger_phase[i].sen_mask, pmotor_mov->motor_trigger_phase[i].sen_pos_index);
				break;
			}
		}
		if(j == pmech_drv->sen_pos_num)
		{
			pmotor_mov->motor_trigger_phase[i].sen_pos_index = 0;
			//printf("motor_triger_phase_senposindex_fill2:sen_mask=%x sen_pos_index=%x\n", pmotor_mov->motor_trigger_phase[i].sen_mask, pmotor_mov->motor_trigger_phase[i].sen_pos_index);
		}

		//printf("motor_triger_phase_senposindex_fill3:sen_mask=%x sen_pos_index=%x\n", pmotor_mov->motor_trigger_phase[i].sen_mask, pmotor_mov->motor_trigger_phase[i].sen_pos_index);
	}
}

extern void motor_mov_init(motor_mov_t *pmotor_mov);
extern unsigned int motor_step_adjust(unsigned int steps, mech_unit_motor_feature_t *punit_motor_feature, unsigned short motor_mask);
extern int motor_speed_adjust(unsigned char speed_phase_num, motor_speed_phase_t  *motor_speed_phase, mech_unit_motor_feature_t *punit_motor_feature, unsigned short motor_mask);
#if 0
extern long mechunit_ioctl( struct mechanism_dev_t *mech_dev, unsigned int ioctrl_cmd, unsigned long arg);
#endif

#define err_info_init()	0
#define  err_info_fill(err_class_code,  err)  0
#define err_info_show()	0
#endif 
