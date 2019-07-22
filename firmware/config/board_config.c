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
#include "board_config.h"
#include "fpga_timer2.h"
#include "fpga_stepmotor_driver2.h"
#include "simplesensor.h"
#include "gpio_simplesensor.h"


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

struct nvram nvram_1 =
{	// nvram device 0: SPIFLASH region
		.resource = &spiflash_rc,
		.install = spiflash_install,
};

struct nvram nvram_2 =
{	// nvram device 1: SOC eNVM region
		.resource = &envm_rc,
		.install = socnvm_install,
};

/* nvram configuration */
struct nvram * nvram_list[] =
{
	&nvram_1,
    &nvram_2,
};

const int nvram_num = 2;//DEVICE_COUNT(nvram);

static const struct gpio_simplesensor_resource sensor0_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_3,
};

static const struct gpio_photosensor_resource sensor1_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_4,
};

static struct pwm_chip pwm_chip0 =
{
	.base_addr = COREPWM_CHIP_0,
	.apb_dwidth = 16,
	.pclk_period = SYS_PCLK_PERIOD,
	.prescale = 3,
	.period = 1000,
};

static const struct gpio_photosensor_resource sensor2_rc = {
	.gpiochip = NULL,
	.gpio = MSS_GPIO_8,
	.pwmchip = &pwm_chip0,
	.pwm = PWM_1,
};

struct simplesensor sensor_front =
{
	.resource = &sensor0_rc,
	.install = gpio_simplesensor_install,
	.status_mapping = SENSOR_ST_DETETED_IS_LOWLEVEL,
};

struct simplesensor sensor_rear =
{
	.resource = &sensor1_rc,
	.install = gpio_simplesensor_install,
	.status_mapping = SENSOR_ST_DETETED_IS_LOWLEVEL,
};

/* photosensor configuration */
struct simplesensor * simplesensor_list[] =
{
	&sensor_front,
	&sensor_rear,
};

struct photosensor sensor_exist =
{
	.resource = &sensor2_rc,
	.install = gpio_photosensor_install,
	.feature =
	{
		.led_brightness_max = MAXIMUM_BRIGHTNESS,
		.raw_input_max = 1,
		.input_scale_mv = 3300000, /* scale = 3.3V */
		.calibrate_mode = 0
	},
	.type = PHOTOSENSOR_DIGITAL,
	.sensor_mode = PHOTOSENSOR_THROUGHBEAM,
	.status_mapping = SENSOR_ST_DETETED_IS_LOWLEVEL,
};

const int simplesensor_num = (sizeof(simplesensor_list)/sizeof(struct simplesensor *));

struct photosensor * photosensor_list[] =
{
	&sensor_exist,
};

const int photosensor_num = (sizeof(photosensor_list)/sizeof(struct photosensor *));

/* FPGA configuration */
const struct fpga_resource fpga_rc = {
	.ctrl_reg_base = (void *)0x30000000u,
	.ints_reg_base = (void *)0x30000030u,
	.mclk_frequency = 100000000u,
};

/* steppermotor configuration */
extern struct motor_speedtable	motor_speed_acc_740_half;
extern struct motor_speedtable	motor_speed_dec_740_half;
extern struct motor_speedtable	motor_speed_acc_1200_half;
extern struct motor_speedtable	motor_speed_dec_1200_half;

/* size of steppermotor speed ramp tables: */
#define FPGA_RAM_MOTOR_TABLE_RAMP_DEPTH		1024
#define FPGA_RAM_MOTOR_TABLE_COUNT_DEPTH	3

#define MOTOR_DEV_STEPMOTOR_NUM	        1

struct ramp_info passport_scanner_ramp_info =
{
	.num_speed = 1,
	.speeds =
	{
		{
			.accel_table=&motor_speed_acc_740_half,
			.decel_table=&motor_speed_dec_740_half
		},
	}
};

static struct fpga_timer_resource fpga_timer1_rc =
{
	.mmio_base = (void *)0x30301000,
	.timer_index = 0,
	.clock_period = 10,	//ns. Freq=100M,
	.fpga_irq_mask = 0x02,
	.fabric_irq = 1,
	.preload_max = FPGA_TIMER_PRELOAD_MAX
};

static struct fpga_timer fpga_timer1 =
{
	.resource = &fpga_timer1_rc,
};

static struct fpga_stepmotor_driver_resource steppermotor_1_rc =
{
	.mmio_base=(void *)0x30300000,
	.pfpga_stepmotor_timer = &fpga_timer1,
	.select = 1,
	.stepping_mode=STEP_MODE_HALF,
	.rampinfo=&passport_scanner_ramp_info,
	.pullin_speed = 300,
};

struct steppermotor steppermotor_1 =
{
	.resource = &steppermotor_1_rc,
	.install = fpga_stepmotor_driver_install,
	.callback = NULL
};

struct steppermotor * steppermotor_list[] =
{
	&steppermotor_1,
};

const int steppermotor_num = (sizeof(steppermotor_list)/sizeof(struct steppermotor *));

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
const int digitisers_num = 1;
const int imagesensors_num = 1;

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
			.outdataen = 0,
			.master_mode_en = 0,
			.flagsig_sel = AFE_B4_SEL_FLAG_FLAGPIX,
			.linelength = 3024,
			.flagpixlen = 56, //56-719, 83-718
			.pllctrl1 = 0x00,
			.pllctrl2 = 0x09,
			.pll_exdiv_sel = 0,
			.dllconfig1 = 0x20,
			.dllconfig2 = 0x10,
		},
};

//-----------------------------------------------------------------


//-----------------------------------------------------------------
struct gpiokey_resource force_card_eject_rc={NULL, MSS_GPIO_0, GPIOKEY_TYPE_LEVEL_LOW};
struct gpiokey eject_key =
{
	.resource = &force_card_eject_rc,
	.install = gpiokey_install,
};

struct gpiokey * gpiokey_list[]={
	&eject_key,
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
	if(rs)
		return -1;

	rs = fpga_install(&fpga_rc);
	if(rs)
		return -1;
	rs = gpiokey_install_devices();
	if(rs)
		return -1;
	
	rs = simplesensor_install_devices();
	if(rs)
		return -1;

	rs = photosensor_install_devices();
	if(rs)
		return -1;

	rs = steppermotor_install_devices();
	if(rs)
		return -1;
	fpga_cis_drvinit();
	wm8235_drvinit();

	rs = scanunit_install(&checkscanner);
	if(rs)
		return -1;

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


