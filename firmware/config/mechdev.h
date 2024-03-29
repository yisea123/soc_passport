#ifndef __MECHDEV_H__
#define __MECHDEV_H__

#define CARDPATH_MOTOR_MASK	0x01

/* card pos sensors masks .*/
#define CARDPATH_CD_SENSOR_MASK		0x01	//card detecting
#define CARDPATH_CS0_SENSOR_MASK		0x02	//card reading
#define CARDPATH_CS1_SENSOR_MASK		0x04	//card scanning
#ifdef IC61S_4M
#define CARDPATH_CS2_SENSOR_MASK	0x08	//card scanning end
#endif
#ifdef IC61S_10M
#define CARDPATH_CS2_SENSOR_MASK	0x08	//card scanning end
#endif

#define CARDPATH_COVER_SENSOR_MASK	0x10	//cover open

#ifdef IC61S_UV 
#define CARDPATH_ALL_CSX_SENSORS_MASK	(CARDPATH_CS0_SENSOR_MASK|CARDPATH_CS1_SENSOR_MASK)
#endif

#ifdef IC62S_4M
#define CARDPATH_ALL_CSX_SENSORS_MASK	(CARDPATH_CS0_SENSOR_MASK|CARDPATH_CS1_SENSOR_MASK)
#endif

#ifdef IC61S_4M
#define CARDPATH_ALL_CSX_SENSORS_MASK	(CARDPATH_CS0_SENSOR_MASK|CARDPATH_CS1_SENSOR_MASK|CARDPATH_CS2_SENSOR_MASK)
#endif
#ifdef IC61S_10M
#define CARDPATH_ALL_CSX_SENSORS_MASK	(CARDPATH_CS0_SENSOR_MASK|CARDPATH_CS1_SENSOR_MASK|CARDPATH_CS2_SENSOR_MASK)
#endif

#define CARDPATH_ALL_SENSORS_MASK	(CARDPATH_CD_SENSOR_MASK|CARDPATH_ALL_CSX_SENSORS_MASK)
#define CARDSCANNER_ALL_SENSORS_MASK    (CARDPATH_ALL_SENSORS_MASK | CARDPATH_COVER_SENSOR_MASK)

typedef enum{
	FPGA_CIS_INTERRUPT,
	FPGA_MOTOR_CARDPATH_INTERRUPT,
	FPGA_INTERRUPT_NUM
}card_board_irq_type_t;

#endif

