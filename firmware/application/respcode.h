#ifndef __RESPCODE_H__
#define	__RESPCODE_H__

typedef enum {
	RESN_CLASS_NONE = 0x00,	
	RESN_CLASS_APP_COMMON = 0x01, 
	RESN_CLASS_APP_CUSTOM,
	RESN_CLASS_SCAN_APP_LIB,
	RESN_CLASS_SCAN_DRIVER,
	RESN_CLASS_MECH_DRIVER_MOV,
	RESN_CLASS_MECH_APP_LIB,
	RESN_CLASS_MECH_DRIVER_OTHER,
	RESN_CLASS_KERNEL 
}resn_class_code_t;

//==========================================
#define RESN_OFFSET	0x00
//-----RESN_CLASS_APP_COMMON-----
#define RESN_OFFSET_APP_COMMON	(RESN_OFFSET)
#define RESN_LENGTH_APP_COMMON	0xff
#define RESN_MASK_APP_COMMON	0xff
#define is_resn_app_common(x)		(((x>RESN_OFFSET_APP_COMMON)&&(x<=(RESN_OFFSET_APP_COMMON+RESN_LENGTH_APP_COMMON)))?1:0)

//-----RESN_CLASS_APP_CUSTOM-----
#define RESN_OFFSET_APP_CUSTOM	(RESN_OFFSET+0x100)
#define RESN_LENGTH_APP_CUSTOM	0x100
#define RESN_MASK_APP_CUSTOM	0x100
#define is_resn_app_custom(x)		(((x>=RESN_OFFSET_APP_CUSTOM)&&(x<(RESN_OFFSET_APP_CUSTOM+RESN_LENGTH_APP_CUSTOM)))?1:0)


//-----RESN_CLASS_SCAN_APP_LIB-----
#define RESN_OFFSET_SCAN_APP_LIB	(RESN_OFFSET+0x1000)
#define RESN_LENGTH_SCAN_APP_LIB	0x100
#define RESN_MASK_SCAN_APP_LIB	0x10ff

//-----RESN_CLASS_SCAN_DRIVER-----
#define RESN_OFFSET_SCAN_DRIVER_LIB	(RESN_OFFSET+0x1100)
#define RESN_LENGTH_SCAN_DRIVER_LIB	0xf00
#define RESN_MASK_SCAN_DRIVER_LIB	0x11ff

//-----RESN_CLASS_MECH_DRIVER_MOV-----
#define RESN_OFFSET_MECH_DRIVER_MOV	(RESN_OFFSET+0x2000)
#define RESN_LENGTH_MECH_DRIVER_MOV	0x1000
#define RESN_MASK_MECH_DRIVER_MOV	0x2fff
#define is_resn_mech_driver_mov(x)		(((x>=RESN_OFFSET_MECH_DRIVER_MOV)&&(x<(RESN_OFFSET_MECH_DRIVER_MOV+RESN_LENGTH_MECH_DRIVER_MOV)))?1:0)

//-----RESN_CLASS_MECH_APP_LIB-----
#define RESN_OFFSET_MECH_APP_LIB	(RESN_OFFSET+0x3000)
#define RESN_LENGTH_MECH_APP_LIB	0x100
#define RESN_MASK_MECH_APP_LIB		0x30ff
#define is_resn_mech_app_lib(x)		(((x>=RESN_OFFSET_MECH_APP_LIB)&&(x<(RESN_OFFSET_MECH_APP_LIB+RESN_LENGTH_MECH_APP_LIB)))?1:0)

//-----RESN_CLASS_MECH_DRIVER_OTHER-----
#define RESN_OFFSET_MECH_DRIVER_OTHER	(RESN_OFFSET+0x3100)
#define RESN_LENGTH_MECH_DRIVER_OTHER	0xf00
#define RESN_MASK_MECH_DRIVER_OTHER	0x31ff
#define is_resn_mech_driver_other(x)	(((x>=RESN_OFFSET_MECH_DRIVER_OTHER)&&(x<(RESN_OFFSET_MECH_DRIVER_OTHER+RESN_LENGTH_MECH_DRIVER_OTHER)))?1:0)

//-----RESN_CLASS_KERNEL----- 
#define RESN_OFFSET_KERNEL	(RESN_OFFSET+0x8000)
#define RESN_LENGTH_KERNEL	0x1000
#define RESN_MASK_KERNEL	0x8fff
#define is_kernel_err(x)	(((x>0)&&(x<RESN_LENGTH_KERNEL))?1:0)
#define is_resn_kernel(x)	(((x>RESN_OFFSET_KERNEL)&&(x<(RESN_OFFSET_KERNEL+RESN_LENGTH_KERNEL)))?1:0)


//==========================================
//-----RESN_CLASS_APP_COMMON-----
#define RESN_APP_COMMON_CODE(x)		(RESN_OFFSET_APP_COMMON+x)
#define RESN_APP_COMMON_INVALID_PARAMETER	RESN_APP_COMMON_CODE(0x01)
#define RESN_APP_COMMON_MISSING_DATA		RESN_APP_COMMON_CODE(0x02)
#define RESN_APP_COMMON_UNDEFINED_COMMAND	RESN_APP_COMMON_CODE(0x03)
#define RESN_APP_COMMON_DATA_ERROR		RESN_APP_COMMON_CODE(0x04)
#define RESN_APP_COMMON_SOFTWARE_ERROR		RESN_APP_COMMON_CODE(0x10)
#define RESN_APP_COMMON_READ_DATA_ERROR		RESN_APP_COMMON_CODE(0x11)
#define RESN_APP_COMMON_WRITE_DATA_ERROR	RESN_APP_COMMON_CODE(0x12)
#define RESN_APP_COMMON_FILE_NOT_EXIST		RESN_APP_COMMON_CODE(0x13)
#define RESN_APP_COMMON_MEM_ALLOC_ERROR		RESN_APP_COMMON_CODE(0x14)
#define RESN_APP_COMMON_MEDIA_ERROR		RESN_APP_COMMON_CODE(0x40)
#define RESN_APP_COMMON_PRINTING_ERROR		RESN_APP_COMMON_CODE(0x80)
#define RESN_APP_COMMON_SCANNING_ERROR		RESN_APP_COMMON_CODE(0xc0)
#define RESN_APP_COMMON_ACCEPTING_ERROR		RESN_APP_COMMON_CODE(0xe0)
//-----RESN_CLASS_APP_CUSTOM-----

//-----RESN_CLASS_SCAN_APP_LIB-----
//-----RESN_CLASS_SCAN_DRIVER-----
//-----RESN_CLASS_MECH_DRIVER_MOV-----
#define RESN_MECH_DRIVER_MOV_CODE(x)		(RESN_OFFSET_MECH_DRIVER_MOV+x)

//-----RESN_CLASS_MECH_APP_LIB-----
#define RESN_MECH_APP_CODE(x)				(RESN_OFFSET_MECH_APP_LIB+x)
#define RESN_MECH_APP_SENSOR_NOT_INIT			RESN_MECH_APP_CODE(0x01)	//0x3001
#define RESN_MECH_APP_MOTOR_NOT_INIT			RESN_MECH_APP_CODE(0x02)
#define RESN_MECH_APP_INVALID_MOTOR_SPEED		RESN_MECH_APP_CODE(0x03)
#define RESN_MECH_APP_INVALID_PARAM			RESN_MECH_APP_CODE(0x04)	
#define RESN_MECH_APP_SIG_INIT_ERR			RESN_MECH_APP_CODE(0x05)
#define RESN_MECH_APP_CAL_WITHPAPER_MIN			RESN_MECH_APP_CODE(0x06)
#define RESN_MECH_APP_CAL_WITHPAPER_MAX			RESN_MECH_APP_CODE(0x07)
#define RESN_MECH_APP_CAL_WITHOUTPAPER_MIN		RESN_MECH_APP_CODE(0x08)
#define RESN_MECH_APP_CAL_WITHOUTPAPER_MAX		RESN_MECH_APP_CODE(0x09)
#define RESN_MECH_APP_CALIBRATE_BLACK			RESN_MECH_APP_CODE(0x0a)
#define RESN_MECH_APP_FILE_OPEN_ERR			RESN_MECH_APP_CODE(0x0b)
#define RESN_MECH_APP_FILE_READ_ERR			RESN_MECH_APP_CODE(0x0c)
#define RESN_MECH_APP_CAL_LOAD_ERR			RESN_MECH_APP_CODE(0x0d)
#define RESN_MECH_APP_CAL_WITHPAPER_50ERR		RESN_MECH_APP_CODE(0x0e)
#define RESN_MECH_APP_CAL_WITHPAPER_75ERR		RESN_MECH_APP_CODE(0x0f)
#define RESN_MECH_APP_CAL_WITHPAPER_THRERR		RESN_MECH_APP_CODE(0x10)
#define RESN_MECH_APP_CAL_WITHPAPER_CHANGELITTLE	RESN_MECH_APP_CODE(0x11)

#define RESN_MECH_APP_CUSTOM_OFFSET		RESN_MECH_APP_CODE(0x50)

//-----RESN_CLASS_MECH_DRIVER_OTHER-----
#define RESN_MECH_DRIVER_OTHER_CODE(x)			(RESN_OFFSET_MECH_DRIVER_OTHER+x)
		
#define RESN_MECH_NO_ERROR  	 	0		// 
#define RESN_MECH_ERR_IVALID_CMD			RESN_MECH_DRIVER_OTHER_CODE(1)	//0x3101:
//#define RESN_MECH_ERR_SENSOR_ERR			RESN_MECH_DRIVER_OTHER_CODE(2)	//0x3102:
#define RESN_MECH_ERR_MOTOR_GETDATA			RESN_MECH_DRIVER_OTHER_CODE(3)	//0x3103:
#define RESN_MECH_ERR_SENSOR_GETDATA			RESN_MECH_DRIVER_OTHER_CODE(4)	//0x3104:

#define RESN_MECH_ERR_MOTOR_MOVE_SET_CONFIG_ERR		RESN_MECH_DRIVER_OTHER_CODE(5)	//0x3105:motor moving params(speed & other params) failed download to FPGA.Followed with MOTOR_STOP_BY_ABNORMAL. 
//steppermotor_set_config/dcmotor_set_config failed 
#define RESN_MECH_ERR_MOTOR_MOVE_SET_SENMASK_ERR	RESN_MECH_DRIVER_OTHER_CODE(6)	//0x3106:steppermotor_set_sensor_sel_mask failed
#define RESN_MECH_ERR_MOTOR_MOVE_SET_TRIGGER_NEXT	RESN_MECH_DRIVER_OTHER_CODE(7)	//0x3107:steppermotor_set_trigger_next/sensor_set_trigger_next failed
#define RESN_MECH_ERR_MOTOR_MOVE_START_ERR		RESN_MECH_DRIVER_OTHER_CODE(8)	//0x3108steppermotor_start/dcmotor_start failed

#define RESN_MECH_ERR_MOTOR_WAIT_TRIGER_TIMEOUT   	RESN_MECH_DRIVER_OTHER_CODE(9)	//0x3109:wait for trigger interrupt timeout.Followed with MOTOR_STOP_BY_ABNORMAL. 
#define RESN_MECH_ERR_MOTOR_WAIT_STOP_TIMEOUT     	RESN_MECH_DRIVER_OTHER_CODE(0x0a)	//0x310a:wait for stop interrupt timeout.Followed with MOTOR_STOP_BY_ABNORMAL. 
#define RESN_MECH_ERR_MOTOR_INT_INVALID           	RESN_MECH_DRIVER_OTHER_CODE(0x0b)	//0x310b:interrupt info is invalid.Followed with MOTOR_STOP_BY_ABNORMAL. 
#define RESN_MECH_ERR_MOTOR_SENSOR_CONFIG_ERR		RESN_MECH_DRIVER_OTHER_CODE(0x0c)	//0x310c:sensor params about motor moving(triger infomation etc.) failed download to FPGA.Followed with MOTOR_STOP_BY_ABNORMAL.
#define RESN_MECH_ERR_MOTOR_HW_ERR			RESN_MECH_DRIVER_OTHER_CODE(0x0d)	//0x310d:motor hardware driver error.Followed with MOTOR_STOP_BY_ABNORMAL.
#define RESN_MECH_ERR_MOTOR_BUSY			RESN_MECH_DRIVER_OTHER_CODE(0x0e)	//0x310e:
#define RESN_MECH_ERR_MOTOR_TOO_MORE_TRIGERINT		RESN_MECH_DRIVER_OTHER_CODE(0x0f)	//0x310f:

//-----RESN_CLASS_KERNEL----- 
#define RESN_KERNEL_CODE(x)			(RESN_OFFSET_KERNEL+x)

#endif
