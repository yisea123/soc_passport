#include <stdio.h>
#ifdef __ICCARM__
#include <yfuns.h>
#endif

#include "sys_config.h"
#include "m2sxxx.h"
#include "mss_gpio.h"
#include "core_pwm.h"

#include "sensor.h"
#include "pwm.h"

#include "ID_Card_Scanner_hw_platform.h"

#include "nvram.h"
#include "socnvm.h"
#include "at24cxx.h"
#include "spiflash.h"

#include "fpgadrv.h"

#include "photosensor.h"
#include "gpio_photosensor.h"

#include "steppermotor.h"
#include "fpga_stepmotor.h"
#include "steptable.h"
#include "fpga.h"

#include "imagedigitiser.h"
#include "wm8235.h"
#include "scanunit.h"
#include "scancalib.h"

#include "imagesensor.h"
#include "fpga_cis.h"

#include "mechdev.h"
#include "mechlib.h"
#include "mech_motor.h"
#include "mech_sensor.h"
#include "mech_unit.h"

#include "scandrv.h"
#include "gpiokey.h"

#define SYS_PCLK_FREQENCY	100000000L
#define SYS_PCLK_PERIOD		(1000000000L/SYS_PCLK_FREQENCY)

#define DEVICE_COUNT(device)	(sizeof(##device##_list)/sizeof(struct device))


/* nvram resources */
static const struct spiflash_resource spiflash_rc={
	.spi_instance = &g_mss_spi0,
	.spi_slave_id = MSS_SPI_SLAVE_0,
	.pspiflash_info = &spiflash_info_W25Q32JV
};

static const struct at24cxx_resource at24c16_rc = {
	.i2c=&g_mss_i2c0,
	.i2c_address=0xa0,
	.size = 0x800,
	.pagesize=8,
};

static const struct socnvm_resource envm_rc = {
	.start_addr = (void *)0x6001F400,
	.size = 0x400,
};

/* nvram configuration */
struct nvram nvram_list[] = {
	{	// nvram device 0: SPIFLASH region
		.resource = &spiflash_rc,
		.install = spiflash_install,
	},
	#if 0
	{	// nvram device 1: AT24C16 EEPROM
		.resource = &at24c16_rc,
		.install = at24cxx_install,
	},
	#else
	{	// nvram device 1: SOC eNVM region
		.resource = &envm_rc,
		.install = socnvm_install,
	},
	#endif
};

const int nvram_num = DEVICE_COUNT(nvram);


/* CorePWM instances */
struct pwm_chip pwm_chip0 = {
	.base_addr = COREPWM_CHIP_0,
	.apb_dwidth = 16,
	.prescale = 3,
	.pclk_period = SYS_PCLK_PERIOD,
	.period = 10000,
};


/* photosensor resources */
static const struct gpio_photosensor_resource sensor0_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_8,
	.pwmchip = &pwm_chip0,
	.pwm = PWM_1,
};

static const struct gpio_photosensor_resource sensor1_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_9,
	.pwmchip = &pwm_chip0,
	.pwm = PWM_2,
};

static const struct gpio_photosensor_resource sensor2_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_10,
	.pwmchip = &pwm_chip0,
	.pwm = PWM_3,
};

static const struct gpio_photosensor_resource sensor3_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_11,
	.pwmchip = &pwm_chip0,
	.pwm = PWM_4,
};

static const struct gpio_photosensor_resource sensor4_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_12,
	.pwmchip = &pwm_chip0,
	.pwm = PWM_5,
};

/* photosensor configuration */
struct photosensor photosensor_list[] = {
	{	// sensor 0
		.resource = &sensor0_rc,
		.install = gpio_photosensor_install,
		.feature = {
			.led_brightness_max = MAXIMUM_BRIGHTNESS,
			.raw_input_max = 1,
			.input_scale_mv = 3300000, /* scale = 3.3V */
			.calibrate_mode = 0
		},
		.type = PHOTOSENSOR_DIGITAL,
		.sensor_mode = PHOTOSENSOR_THROUGHBEAM,
		.status_mapping = SENSOR_ST_DETETED_IS_HIGHLEVEL,
	},
	{	// sensor 1
		.resource = &sensor1_rc,
		.install = gpio_photosensor_install,
		.feature = {
			.led_brightness_max = MAXIMUM_BRIGHTNESS,
			.raw_input_max = 1,
			.input_scale_mv = 3300000, /* scale = 3.3V */
			.calibrate_mode = 0
		},
		.type = PHOTOSENSOR_DIGITAL,
		.sensor_mode = PHOTOSENSOR_THROUGHBEAM,
		.status_mapping = SENSOR_ST_DETETED_IS_HIGHLEVEL,
	},
	{	// sensor 2
		.resource = &sensor2_rc,
		.install = gpio_photosensor_install,
		.feature = {
			.led_brightness_max = MAXIMUM_BRIGHTNESS,
			.raw_input_max = 1,
			.input_scale_mv = 3300000, /* scale = 3.3V */
			.calibrate_mode = 0
		},
		.type = PHOTOSENSOR_DIGITAL,
		.sensor_mode = PHOTOSENSOR_THROUGHBEAM,
		.status_mapping = SENSOR_ST_DETETED_IS_HIGHLEVEL,
	},
	#ifdef IC61S
	{	// sensor 3
		.resource = &sensor3_rc,
		.install = gpio_photosensor_install,
		.feature = {
			.led_brightness_max = MAXIMUM_BRIGHTNESS,
			.raw_input_max = 1,
			.input_scale_mv = 3300000, /* scale = 3.3V */
			.calibrate_mode = 0
		},
		.type = PHOTOSENSOR_DIGITAL,
		.sensor_mode = PHOTOSENSOR_THROUGHBEAM,
		.status_mapping = SENSOR_ST_DETETED_IS_HIGHLEVEL,
	},
	#endif
	{	// sensor 4
		.resource = &sensor4_rc,
		.install = gpio_photosensor_install,
		.feature = {
			.led_brightness_max = MAXIMUM_BRIGHTNESS,
			.raw_input_max = 1,
			.input_scale_mv = 3300000, /* scale = 3.3V */
			.calibrate_mode = 0
		},
		.type = PHOTOSENSOR_DIGITAL,
		.sensor_mode = PHOTOSENSOR_THROUGHBEAM,
		.status_mapping = SENSOR_ST_DETETED_IS_HIGHLEVEL,
	}
	
};

const int photosensor_num = DEVICE_COUNT(photosensor);

/* FPGA configuration */
const struct fpga_resource fpga_rc = {
	.ctrl_reg_base = (void *)0x30000000u,
	.ints_reg_base = (void *)0x30400000u,
	.mclk_frequency = 100000000u,
};

/* steppermotor configuration */
extern struct motor_speedtable	motor_speed_acc_300dpi_color;
extern struct motor_speedtable	motor_speed_dec_300dpi_color;
extern struct motor_speedtable	motor_speed_acc_300dpi_gray;
extern struct motor_speedtable	motor_speed_dec_300dpi_gray;
extern struct motor_speedtable	motor_speed_acc_cardmove;
extern struct motor_speedtable	motor_speed_dec_cardmove;

/* size of steppermotor speed ramp tables: */
#define FPGA_RAM_MOTOR_TABLE_RAMP_DEPTH		1024
#define FPGA_RAM_MOTOR_TABLE_COUNT_DEPTH	3

//#define MOTOR_DEV_STEPMOTOR_NUM	1

#define CARDPATH_STEPMOTOR_SPEED_NUM			3
#ifdef IC61S_4M
#define FPGA_STEP_CLOCK_PERIOD		250	//ns. Freq=4M
#endif
#ifdef IC62S_4M
#define FPGA_STEP_CLOCK_PERIOD		250	//ns. Freq=4M
#endif
#ifdef IC61S_10M
#define FPGA_STEP_CLOCK_PERIOD		100	//ns. Freq=10M
#endif
#define CARDPATH_STEPMOTOR_PULLIN_SPEED	300

struct ramp_info cardpath_ramp_info={
	.num_speed=CARDPATH_STEPMOTOR_SPEED_NUM,
	.speeds=
	{{.accel_table=&motor_speed_acc_300dpi_color, 
	   .decel_table=&motor_speed_dec_300dpi_color},
	  {.accel_table=&motor_speed_acc_300dpi_gray, 
	  .decel_table=&motor_speed_dec_300dpi_gray},
	  {.accel_table=&motor_speed_acc_cardmove, 
	  .decel_table=&motor_speed_dec_cardmove}
	}
};

static struct fpga_stepmotor_resource	motor_scan_rc=
{
	.mmio_base=(void *)0x30300000, 	
	.ram_base=(void *)0x30200000,	
	.ram_ramp_offset = FPGA_RAM_MOTOR_TABLE_RAMP,
	.ram_size=FPGA_RAM_MOTOR_TABLE_RAMP_DEPTH,
	.table_ramp_size = (4*FPGA_RAM_MOTOR_TABLE_RAMP_DEPTH),
	.table_count_size = (4*FPGA_RAM_MOTOR_TABLE_COUNT_DEPTH),
	.stepping=STEP_MODE_16MICRO,	
	.clock_period=FPGA_STEP_CLOCK_PERIOD,	
	.rampinfo=&cardpath_ramp_info,
	.fpga_irq_mask = 0x02,
	.fabric_irq = 1,
	.pullin_speed = CARDPATH_STEPMOTOR_PULLIN_SPEED
};
		
struct steppermotor steppermotor_list[] = {
	{
		.resource = &motor_scan_rc,
		.install = fpga_stepmotor_install,
		.callback = NULL
	}
};


const int steppermotor_num = DEVICE_COUNT(steppermotor);

/* scanunit configuration */
/* scanunit afe device definition */
typedef enum
{
	CHECK_AFE_A,
}device_id;

static const struct wm8235_resource check_afe_a_rc = {
	.mask =	1,
	.mss_spi = &g_mss_spi1,
	.spi_clk_freq = 10000000u,
};

static const struct fpga_cis_resource check_cis_a_rc = {
	.ctrl_base = (void *)0x30200000u,
	.mmio_base = (void *)0x30200008u,
	.mask =	1,
};

/* define scanunit hardware information */
struct scanunit checkscanner = {

		// check  scanunit
		.resource = {
			.ctrl_base = (void *)0x30200000,
			.int_mask = 1,
			.fabric_irq = 0,
		},
		.hwinfo = {
			.sides = 1,
			.colors = 3,
			.lightsources = 10,
			.sensors = 2,
			.digitisers = 1,
			.sensor_a = 0,
			.sections_a = 1,
			.sectinfo_a = {{1, 1008, 1, 1008, 0, 0},},
		},
		.sensorlist = {
			{	// check CIS of side A
				.resource = &check_cis_a_rc,
				.install = fpga_cis_install,
			}
		},
		.digitiserlist =  {
			{
				.resource = &check_afe_a_rc,
				.install = wm8235_install,
				.imagedigitiserid = CHECK_AFE_A,
			}
		},
		.afe_info ={
			.flag_en = 1,
			.master_mode_en = 0,
			.flagsig_sel = AFE_B4_SEL_FLAG_FLAGPIX,
			.linelength = 4032,
			.flagpixlen = 63,
			.pllctrl1 = 0x00,
			.pllctrl2 = 0x09,
			.pll_exdiv_sel = AFE_1C_PLLEXDIV_SEL_1,
			.dllconfig1 = 0x20,
			.dllconfig2 = 0x10,
		},
};

//-----------------------------------------------------------------
static struct motor_data	mechunit_cardpath_motor_data[]={
	{
	.motor_mask=CARDPATH_MOTOR_MASK,
	//.motor_name="motor_cardpath",
	.motor_dev={
		.psteppermotor=&steppermotor_list[0],
	},
	.motor_type=STEPPERMOTOR
	}
};

#ifdef IC61S
#define CARDPATH_MECHUNIT_SENSOR_NUM	5
#endif
#ifdef IC62S_4M
#define CARDPATH_MECHUNIT_SENSOR_NUM	4
#endif
#define CARDPATH_MECHUNIT_MOTOR_NUM     1

static struct sensor_data mechunit_cardpath_sensor_data[CARDPATH_MECHUNIT_SENSOR_NUM]={
	{	
		.sen_mask=CARDPATH_CD_SENSOR_MASK, 
		//.sen_name="sen_card_detect",  
		.sen_dev={
			.pphotosensor=&photosensor_list[0]
		}
	},
	{	
		.sen_mask=CARDPATH_CS0_SENSOR_MASK, 
		//.sen_name="sen_card_read",  
		.sen_dev={
			.pphotosensor=&photosensor_list[1]
		}
	},
	{	
		.sen_mask=CARDPATH_CS1_SENSOR_MASK, 
		//.sen_name="sen_card_scan",  
		.sen_dev={
			.pphotosensor=&photosensor_list[2]
		}
	},
	#ifdef IC61S
	{	
		.sen_mask=CARDPATH_CS2_SENSOR_MASK, 
		//.sen_name="sen_card_scanend",  
		.sen_dev={
			.pphotosensor=&photosensor_list[3]
		}
	},
	
	{	
		.sen_mask=CARDPATH_COVER_SENSOR_MASK, 
		//.sen_name="sen_card_cover",  
		.sen_dev={
			.pphotosensor=&photosensor_list[4]
		}
	},
	#endif

	#ifdef IC62S
	{	
		.sen_mask=CARDPATH_COVER_SENSOR_MASK, 
		//.sen_name="sen_card_cover",  
		.sen_dev={
			.pphotosensor=&photosensor_list[3]
		}
	},
	#endif
};

sen_config_t  mech_cardpath_sensor_config[CARDPATH_MECHUNIT_SENSOR_NUM];
sen_feature_t mech_cardpath_sensor_feature[CARDPATH_MECHUNIT_SENSOR_NUM];
sen_raw_input_t mech_cardpath_sensor_rawinput[CARDPATH_MECHUNIT_SENSOR_NUM];
motor_feature_t mech_cardpath_motor_feature[CARDPATH_MECHUNIT_MOTOR_NUM];

struct mechanism_dev_t mechnism_cardpath_dev={
	.mech_unit_data={
		//.mech_unit_name = "mechunit_cardpath",
		.unit_motor_data={
			.motor_num = CARDPATH_MECHUNIT_MOTOR_NUM,
			.motor = mechunit_cardpath_motor_data,
			
		},
		.bmotor_filled = 1,
		.unit_sensor_data={
			.sensor_num=CARDPATH_MECHUNIT_SENSOR_NUM,
			.sensor = mechunit_cardpath_sensor_data,
			.sensor_masks = CARDSCANNER_ALL_SENSORS_MASK,
		},
		.bsensor_filled = 1
	},
	.mech_unit_control={
		.mech_unit_sen_feature={
			.sen_num=CARDPATH_MECHUNIT_SENSOR_NUM,
			.sen_feature=mech_cardpath_sensor_feature
			},
		.mech_unit_sen_config={
			.sen_num=CARDPATH_MECHUNIT_SENSOR_NUM,
			.sen_config=mech_cardpath_sensor_config
			},
		.mech_unit_sen_raw_input={
			.sen_num=CARDPATH_MECHUNIT_SENSOR_NUM,
			.sen_raw_input=mech_cardpath_sensor_rawinput
			},
		.mech_unit_motor_feature={
			.motor_num = CARDPATH_MECHUNIT_MOTOR_NUM,
			.motor_feature = mech_cardpath_motor_feature
			},
	}
};

//-----------------------------------------------------------------
struct gpiokey_resource force_card_eject_rc={NULL, MSS_GPIO_0, GPIOKEY_TYPE_LEVEL_LOW};

struct gpiokey gpiokey_list[]={
  {
  	.resource = &force_card_eject_rc,
	.install = gpiokey_install,
  },
};

const int gpiokey_num = DEVICE_COUNT(gpiokey);
//-----------------------------------------------------------------

//-----------------------------------------------------------------
static int board_initialize_peripheral()
{
	/* Initialize MSS GPIOs */
	MSS_GPIO_init();

	/* Initialize CorePWM instance */
//	PWM_init(&pwm_chip0.pwm_inst, pwm_chip0.base_addr, pwm_chip0.prescale, PWM_PERIOD(&pwm_chip0, pwm_chip0.period));

	return 0;
}


static int board_install_devices()
{
	int rs;

	socnvm_drvinit();
	rs = nvram_install_devices();

	rs = fpga_install(&fpga_rc);

	rs = gpiokey_install_devices();
	
//	gpio_photosensor_drvinit();
//	rs = photosensor_install_devices();
//
//	fpga_stepmotor_drvinit();
//	rs = steppermotor_install_devices();
	
	fpga_cis_drvinit();
	wm8235_drvinit();

	rs = scanunit_install(&checkscanner);

	return rs;
}

int board_configure(void)
{
	int rs;

#if defined(MICROSEMI_STDIO_THRU_MMUART0) || defined(MICROSEMI_STDIO_THRU_MMUART1)
#ifdef __ICCARM__
	__read(0, NULL, 0);	// workaground to prevent reinitializing MMUART0/1 while calling __write
#endif
#endif

	rs = board_initialize_peripheral();

	rs = board_install_devices();
	return rs;
}


