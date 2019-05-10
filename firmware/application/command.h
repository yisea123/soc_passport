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


//ɨ��ģʽ
//bit7	bit6  bit5	bit4  bit3	bit2  bit1	bit0
//����	�Ҷ�  ��ɫ	 ��    ��	 ��   ����	����
enum _IAM_SCAN_MODE
{
	IAM_GREY_MODE			= 0x40,			//�Ҷ�ģʽ
	IAM_COLOR_MODE			= 0x20,			//��ɫģʽ
	IAM_RED_MODE			= 0x10,			//��ɫģʽ
	IAM_GREEN_MODE			= 0x08,			//��ɫģʽ
	IAM_BLUE_MODE			= 0x04,			//��ɫģʽ
	IAM_INFRARED_MODE		= 0x02,			//����ģʽ
	IAM_ULTRAVIOLET_MODE	= 0x01			//����ģʽ
};

/* packet flag definition */
#define PACKET_FLAG_XMIT_END		0x01

/* macro for command word definition */
#define CMD_TO_HEX(b1, b2)		(unsigned short)(((b2)<<8)+(b1))
#define CMD_BYTE1(cmd)			(unsigned char)(cmd)
#define CMD_BYTE2(cmd)			(unsigned char)(cmd>>8)


/* command definitions */
//1) �豸����[d*]
#define CMD_RESET			CMD_TO_HEX('d', 'R')		//�豸��λ
#define CMD_GET_DEVICE_INFO		CMD_TO_HEX('d', 'i')		//��ȡ�豸������Ϣ
#define CMD_GET_DEVICE_STATUS		CMD_TO_HEX('d', 's')		//��ȡ�豸״̬
#define CMD_GET_DEVICE_ERROR		CMD_TO_HEX('d', 'e')		//��ȡ�豸������Ϣ
#define CMD_GET_DEVICE_PARTLIST		CMD_TO_HEX('d', 'p')		//��ȡ�豸�����б�
#define CMD_UPGRADE_FW			CMD_TO_HEX('d', 'u')		//�̼���������
#define CMD_CALIBRATE_SENSOR		CMD_TO_HEX('d', 'c')		//λ�ô�����У׼
#define CMD_GET_SENSOR_STATUS		CMD_TO_HEX('d', 'd')		//��ѯ���д�����
#define CMD_GET_LIFETIME_INFO		CMD_TO_HEX('d', 'l')		//��ȡ����ʹ��������Ϣ
#define CMD_GET_LOG			CMD_TO_HEX('d', 'L') 		//��ȡ�豸��־����
#define CMD_GET_STATISTICS		CMD_TO_HEX('d', 'S')		//��ȡ����ͳ����Ϣ
#define CMD_CLEAR_STATISTICS		CMD_TO_HEX('d', 'C')		//�������ͳ����Ϣ							//
#define CMD_GET_CLOCKTIME		CMD_TO_HEX('d', 't') 		//��ȡ�豸����ʱ��
#define CMD_SET_CLOCKTIME		CMD_TO_HEX('d', 'T') 		//�����豸����ʱ�� 

//2) ���ʿ�������[m*]
#define CMD_MOVE_MEDIA			CMD_TO_HEX('m', 'm')		//�ƶ�����
#define CMD_STOP_MEDIA			CMD_TO_HEX('m', 'p')		//ֹͣ�����ƶ�
#define CMD_EJECT_REAR			CMD_TO_HEX('m', 'r')		//����˦������
#define CMD_RETURN_REAR			CMD_TO_HEX('m', 'e')		//���ʻ��˵����
#define CMD_EJECT_FRONT			CMD_TO_HEX('m', 'f')		//��ǰ��˦������
#define CMD_RETURN_FRONT		CMD_TO_HEX('m', 'n')		//���ʻ��˵�ǰ��
#define CMD_MEDIA_ALIGNMENT		CMD_TO_HEX('m', 'a')		//�������
#define CMD_ENTER_WAITALIGNMENT		CMD_TO_HEX('m', 'A')		//����ȴ����ʶ���ģʽ
#define CMD_OPEN_ALIGNER		CMD_TO_HEX('m', 'o')		//�򿪶��뵲צ
#define CMD_CLOSE_ALIGNER		CMD_TO_HEX('m', 'c')		//�رն��뵲צ
#define CMD_GET_MEDIA_LENGTH		CMD_TO_HEX('m', 'l')		//��ý��ʳ���

//3) ��Ʊ������������[a*]
#define CMD_RESET_ACCEPTOR		CMD_TO_HEX('a', 'R')		//��Ʊ������λ
#define CMD_ACCEPT_MEDIA		CMD_TO_HEX('a', 'i')		//��Ʊ��Ʊ��
#define CMD_SELECT_ACCEPTOR		CMD_TO_HEX('a', 's')		//ѡ����Ʊ��

//4) ͼ��ɨ���������[s*]
#define CMD_RESET_SCANUNIT		CMD_TO_HEX('s', 'R')		//ɨ�赥Ԫ��λ
#define CMD_SET_SCANMODE		CMD_TO_HEX('s', 'm')		//����ɨ�跽ʽ
#define CMD_GET_SCAN_FEATURE		CMD_TO_HEX('s', 'f')		//��ȡɨ�赥Ԫ������Ϣ
#define CMD_GET_IMG_CALIB_DATA		CMD_TO_HEX('s', 'r')		//��ȡͼ��У׼����
#define CMD_SET_IMG_CALIB_DATA		CMD_TO_HEX('s', 'w')		//����ͼ��У׼����
#define CMD_START_SCANNING		CMD_TO_HEX('s', 's')		//��ֽ��ɨ��
#define CMD_SCAN_CALIB_DATA		CMD_TO_HEX('s', 'c')		//��ֽ��ɨ��ͼ��У׼����
#define CMD_SCANNING_CALIBRATION	CMD_TO_HEX('s', 'C')		//��ֽ��ɨ�輰����ͼ��У׼����
#define CMD_SCANNING_ADJUSTMENT		CMD_TO_HEX('s', 'j')		//����ɨ�谵��ƽ/����ƽ
#define CMD_STOP_SCANNING		CMD_TO_HEX('s', 'p')		//ֹͣɨ��
#define CMD_DUMMY_SCANNING		CMD_TO_HEX('s', 'd')		//ģ��ɨ��
#define CMD_GET_IMG_EXPOSURE_TIME	CMD_TO_HEX('s', 'x')		//��ȡͼ��ɨ���ع�ʱ������ 
#define CMD_SET_IMG_EXPOSURE_TIME	CMD_TO_HEX('s', 'X')		//����ͼ��ɨ���ع�ʱ������
#define CMD_GET_IMG_AFE_GAIN		CMD_TO_HEX('s', 'g')		//��ȡͼ��ɨ�赥ԪAFE�������  
#define CMD_SET_IMG_AFE_GAIN		CMD_TO_HEX('s', 'G')		//����ͼ��ɨ�赥ԪAFE������� 

//5)֧Ʊ��������
#define CMD_PHOTO_LIGHT			CMD_TO_HEX('r', 'l')		//���Ʊ�䲹���
#define CMD_STORE_BOX_POSIT		CMD_TO_HEX('r', 'u')		//��Ʊ�䶨λ01������00�½�
#define CMD_REAR_MEDIA_POSIT		CMD_TO_HEX('r', 'p')		/* ֽ�Ŵ�Ʊ���˶���ָ��λ��
									00�޶���(����),01׼������λ��,02׼����ӡλ�� */
#define CMD_REAR_MEDIA_PAPER_PATTERN	CMD_TO_HEX('r', 's')		/* ֽ���ƶ���ֽ��ģ�� */
#define CMD_FRONT_GATE			CMD_TO_HEX('f', 'g')		//ǰ���Ŷ���
//6)��ӡ����
#define CMD_PRINT_AGEIN			CMD_TO_HEX('p', 'p')		//�ظ���ӡ��һ������
#define CMD_PRINT_DATA			CMD_TO_HEX('p', 'd')		//��ӡ����
#define CMD_PRINT_SETTING		CMD_TO_HEX('p', 's')		/* ��ӡ����������
									00��ȡ���ã�01д������
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

