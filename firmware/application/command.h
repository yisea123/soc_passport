#ifndef __COMMAND_H__
#define __COMMAND_H__


#define DEVICE_ID	(0x02020202)	// (always be 0x02020202)

#define MAX_PACKET_LEN	(1024 * 1024 * 4)
#define MAX_DATA_LEN	(MAX_PACKET_LEN - sizeof(comm_packet_head_t))


#pragma pack(1)		// set one-byte alignment

// data structure of command/response packet
typedef struct comm_packet_head_s
{
	unsigned int deviceid;			// device ID
	unsigned char ptype;			// packet type: command (C); response (P/N)
	unsigned short command;			// command code
	unsigned char reserved;			// reserved for 4-bytes alignment
	union {
	unsigned short para;			// command parameter
	unsigned short status;			// response status
	};
	unsigned char pid;			// packet ID (0~255)
	unsigned char pflag;			// packet flag
	unsigned int datalength;		// data length (SHOULD be ZERO if no data)
} comm_packet_head_t;

/* motion direction definition */
typedef enum {
	MOTION_FORWARD,
	MOTION_BACKWARD,
	MOTION_UP,
	MOTION_DOWN,
	MOTION_LEFT,
	MOTION_RIGHT,
} motion_direction;

/* motion value type definition */
typedef enum {
	MOTION_VALUE_MOTOR_FULLSTEP,		// value in full steps (stepper motor)
	MOTION_VALUE_DISP_MM,			// value in millimeters of displacement
	MOTION_VALUE_DISP_INCH,			// value in inches of displacement
} motion_value_type;


// data structure of media motion configuration
typedef struct motion_config_s
{
	unsigned char direction;		// direction of media motion
	unsigned char unit;			// definition of 'value'
	short speed_index;			// speed index setting (if speed_index = -1, speed is defined by 'speed')
	unsigned int speed;			// speed value setting in MMS (Millimeters per Second), or SPS (full Steps per Second)
	unsigned int value;			// motion displacement value (unit is defined by 'type')
} motion_config_t;


// parameter definition of command CMD_GET_LOG (get device log)
typedef struct para_device_log_s
{
	unsigned char type;			// device log type
	unsigned char index;			// device log data index number
} para_device_log_t;


typedef struct para_reset_s
{
	unsigned mode: 4 ;			// reset mode
	unsigned save_flag: 1;			// save flag
} para_reset_t;

#pragma pack()		// restore default alignment


/* packet type */
enum packet_type
{
	PACKET_COMMAND  = 'C',			// Command
	PACKET_RESPONSE_SUCCESS	= 'P',		// Positive Response
	PACKET_RESPONSE_ERROR	= 'N'		// Negative Response
};


//扫描模式
//bit7	bit6  bit5	bit4  bit3	bit2  bit1	bit0
//保留	灰度  彩色	 红    绿	 蓝   红外	紫外
enum _IAM_SCAN_MODE
{
	IAM_GREY_MODE			= 0x40,			//灰度模式
	IAM_COLOR_MODE			= 0x20,			//彩色模式
	IAM_RED_MODE			= 0x10,			//红色模式
	IAM_GREEN_MODE			= 0x08,			//绿色模式
	IAM_BLUE_MODE			= 0x04,			//蓝色模式
	IAM_INFRARED_MODE		= 0x02,			//红外模式
	IAM_ULTRAVIOLET_MODE	= 0x01			//紫外模式
};

/* packet flag definition */
#define PACKET_FLAG_XMIT_END		0x01

/* macro for command word definition */
#define CMD_TO_HEX(b1, b2)		(unsigned short)(((b2)<<8)+(b1))
#define CMD_BYTE1(cmd)			(unsigned char)(cmd)
#define CMD_BYTE2(cmd)			(unsigned char)(cmd>>8)


/* command definitions */
//1) 设备控制[d*]
#define CMD_RESET			CMD_TO_HEX('d', 'R')		//设备复位
#define CMD_GET_DEVICE_INFO		CMD_TO_HEX('d', 'i')		//获取设备基本信息
#define CMD_GET_DEVICE_STATUS		CMD_TO_HEX('d', 's')		//获取设备状态
#define CMD_GET_DEVICE_ERROR		CMD_TO_HEX('d', 'e')		//获取设备错误信息
#define CMD_GET_DEVICE_PARTLIST		CMD_TO_HEX('d', 'p')		//获取设备部件列表
#define CMD_UPGRADE_FW			CMD_TO_HEX('d', 'u')		//固件在线升级
#define CMD_CALIBRATE_SENSOR		CMD_TO_HEX('d', 'c')		//位置传感器校准
#define CMD_GET_SENSOR_STATUS		CMD_TO_HEX('d', 'd')		//查询所有传感器
#define CMD_GET_LIFETIME_INFO		CMD_TO_HEX('d', 'l')		//获取部件使用寿命信息
#define CMD_GET_LOG			CMD_TO_HEX('d', 'L') 		//获取设备日志数据
#define CMD_GET_STATISTICS		CMD_TO_HEX('d', 'S')		//获取部件统计信息
#define CMD_CLEAR_STATISTICS		CMD_TO_HEX('d', 'C')		//清除部件统计信息							//
#define CMD_GET_CLOCKTIME		CMD_TO_HEX('d', 't') 		//获取设备日历时钟
#define CMD_SET_CLOCKTIME		CMD_TO_HEX('d', 'T') 		//设置设备日历时钟 

//2) 介质控制命令[m*]
#define CMD_MOVE_MEDIA			CMD_TO_HEX('m', 'm')		//移动介质
#define CMD_STOP_MEDIA			CMD_TO_HEX('m', 'p')		//停止介质移动
#define CMD_EJECT_REAR			CMD_TO_HEX('m', 'r')		//向后端甩出介质
#define CMD_RETURN_REAR			CMD_TO_HEX('m', 'e')		//介质回退到后端
#define CMD_EJECT_FRONT			CMD_TO_HEX('m', 'f')		//向前端甩出介质
#define CMD_RETURN_FRONT		CMD_TO_HEX('m', 'n')		//介质回退到前门
#define CMD_MEDIA_ALIGNMENT		CMD_TO_HEX('m', 'a')		//对齐介质
#define CMD_ENTER_WAITALIGNMENT		CMD_TO_HEX('m', 'A')		//进入等待介质对齐模式
#define CMD_OPEN_ALIGNER		CMD_TO_HEX('m', 'o')		//打开对齐挡爪
#define CMD_CLOSE_ALIGNER		CMD_TO_HEX('m', 'c')		//关闭对齐挡爪
#define CMD_GET_MEDIA_LENGTH		CMD_TO_HEX('m', 'l')		//获得介质长度

//3) 收票机构控制命令[a*]
#define CMD_RESET_ACCEPTOR		CMD_TO_HEX('a', 'R')		//收票机构复位
#define CMD_ACCEPT_MEDIA		CMD_TO_HEX('a', 'i')		//推票进票箱
#define CMD_SELECT_ACCEPTOR		CMD_TO_HEX('a', 's')		//选择收票箱

//4) 图像扫描控制命令[s*]
#define CMD_RESET_SCANUNIT		CMD_TO_HEX('s', 'R')		//扫描单元复位
#define CMD_SET_SCANMODE		CMD_TO_HEX('s', 'm')		//设置扫描方式
#define CMD_GET_SCAN_FEATURE		CMD_TO_HEX('s', 'f')		//获取扫描单元特性信息
#define CMD_GET_IMG_CALIB_DATA		CMD_TO_HEX('s', 'r')		//获取图像校准数据
#define CMD_SET_IMG_CALIB_DATA		CMD_TO_HEX('s', 'w')		//设置图像校准数据
#define CMD_START_SCANNING		CMD_TO_HEX('s', 's')		//进纸并扫描
#define CMD_SCAN_CALIB_DATA		CMD_TO_HEX('s', 'c')		//进纸并扫描图像校准数据
#define CMD_SCANNING_CALIBRATION	CMD_TO_HEX('s', 'C')		//进纸并扫描及生成图像校准数据
#define CMD_SCANNING_ADJUSTMENT		CMD_TO_HEX('s', 'j')		//调整扫描暗电平/亮电平
#define CMD_STOP_SCANNING		CMD_TO_HEX('s', 'p')		//停止扫描
#define CMD_DUMMY_SCANNING		CMD_TO_HEX('s', 'd')		//模拟扫描
#define CMD_GET_IMG_EXPOSURE_TIME	CMD_TO_HEX('s', 'x')		//获取图像扫描曝光时间数据 
#define CMD_SET_IMG_EXPOSURE_TIME	CMD_TO_HEX('s', 'X')		//设置图像扫描曝光时间数据
#define CMD_GET_IMG_AFE_GAIN		CMD_TO_HEX('s', 'g')		//获取图像扫描单元AFE增益参数  
#define CMD_SET_IMG_AFE_GAIN		CMD_TO_HEX('s', 'G')		//设置图像扫描单元AFE增益参数 

//5)支票售卖命令
#define CMD_PHOTO_LIGHT			CMD_TO_HEX('r', 'l')		//后存票箱补光灯
#define CMD_STORE_BOX_POSIT		CMD_TO_HEX('r', 'u')		//存票箱定位01上升，00下降
#define CMD_REAR_MEDIA_POSIT		CMD_TO_HEX('r', 'p')		/* 纸张从票仓运动到指定位置
									00无动作(保留),01准备照相位置,02准备打印位置 */
#define CMD_REAR_MEDIA_PAPER_PATTERN	CMD_TO_HEX('r', 's')		/* 纸张移动到纸纹模块 */
#define CMD_FRONT_GATE			CMD_TO_HEX('f', 'g')		//前端门动作
//6)打印命令
#define CMD_PRINT_AGEIN			CMD_TO_HEX('p', 'p')		//重复打印上一次数据
#define CMD_PRINT_DATA			CMD_TO_HEX('p', 'd')		//打印数据
#define CMD_PRINT_SETTING		CMD_TO_HEX('p', 's')		/* 打印机设置命令
									00读取设置，01写入设置
									*/

/* parameter definition of command CMD_GET_DEVICE_PARTLIST: partlist type */
enum partlist_type
{
	PARTLIST_MOTOR,			// motors
	PARTLIST_PHOTOSENSOR,		// photosensors
};


/* parameter definition of command CMD_GET_LOG: log data type  */
enum log_type
{
	LOG_GENERIC = 0,		// request generic log data
	LOG_SYSTEM = 1,			// request system log by index
	LOG_APPLICATION	= 2,		// request application log by index
	LOG_FILENAME = 0xff,		// request log data by file name
};

/* parameter definition of command CMD_RESET: reset mode  */
enum reset_mode
{
	RESET_MODE0 = 0,		//reset no action
	RESET_MODE1 = 1,		//reset mode 1
	RESET_MODE2 = 2,		//reset mode 2
	RESET_MODE3 = 3,		//reset mode 3
	RESET_MODE4 = 4,		//reset mode 4
	RESET_MODE5 = 5,		//reset mode 5
	RESET_MODE6 = 6,		//reset mode 6
	RESET_MODE7 = 7,		//reset mode 7
};

/* positive response status definitions */
#define RES_NO_MEDIA			0x0000		// no media presented 
#define RES_MEDIA_AT_FRONT		0x0001		// media detected at front gate
#define RES_MEDIA_ALIGNED		0x0002		// media is aligned at the aligner
#define RES_MEDIA_LOADED		0x0003		// media is loaded into paperpath (during scanning/printing/moving)
#define RES_MEDIA_ACCEPTED		0x0004		// media is accepted into acceptor
#define RES_MEDIA_PRINTER		0x0005		// media is in printer
#define RES_MEDIA_JAM			0x0015		// media jam

#endif /* __COMMAND_H__ */

