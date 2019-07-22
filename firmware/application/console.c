#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <scanunit.h>
#include "version.h"
#include "fpgadrv.h"
#include "fpga_io.h"
#include "scandrv.h"
#include "scancalib.h"
#include "scanner.h"
#include "command.h"
#include "card_scanpath_drv.h"
#include "FreeRTOS.h"
#include "task.h"
#include "actions.h"

extern int scanmode;
extern struct scanunit_scanmode mode;
extern struct scanunit checkscanner;
//extern mech_operation_t cardpath_operation;

const char cmdprompt[]="scanner> ";

#define CMDPROMPT_LEN	strlen(cmdprompt)

static inline void skip_blanks(char **ptr)
{
	while (**ptr == ' ' || **ptr == '\t')
		(*ptr)++;
}

static void print_cmderror(int pos, const char *errmsg)
{
	int err = CMDPROMPT_LEN + 1 + pos;
	printf("\r%*c %s", err, '^', errmsg);
}

const char *helpmsg[] = {
	"scantest commands:",
	"  h,H\t\tprint help",
	"  q,Q\t\tquit",
	"  3\t\tscanner_afe_setvalue(device, channelid, address, afevalue)",
	"  4\t\tscanner_afe_getvalue(device, channelid, address)",
	"  5\t\scanner_afe_set_pll(device, channelid, value)",
	"  6\t\scanner_set_afe_clock_monitor_source(device, channelid, value)",
	"  l,L <action>\t\tturn ON/OFF CIS light sources<device><value>",
	"  k,K <action>\t\tstart/stop scanning without media movement<device><value>",
	"  s,S <mode>\t\tstart scanning",
	"  d,D\t\tstop scanning",
	"  r,R\t\treset scanning unit",
	"  e\t\tscanner_set_led_ontime(device, value, lightsource)",
	"  E\t\tscanner_reset_cisled_config(device, value)=<0:set all to 0; 1:set all to current buffer value>",
	"  u,U\t\tset_afe_vsmpup_config(device, side, vsmpup, vsmpdown, shift)",
	"  t,T\t\tset_afe_siginac_config(device,side, siginac, sigacti)",
	"  i,I\t\tinitialize scanning unit with default configuration, scanner_set_sensor_default_config(device)"
	"  o,O <side> <offset>\n\t\tset image digitisers (AFE) offset.<device><side><offset>",
	"  p\t\tprint photosensor status.",
	"  P\t\tcalibrate photosensors.",
	"  a,A <side>\tautomatically adjust image digitisers (AFE) offset.\n\t\t<resolution><ledmode/scanmode><device>",
	"  c,C <side>\tautomatically adjust image sensors (CIS) LEDs brightness\n\t\t<resolution><ledmode/scanmode><device><lightsel>",
	"  m,M <dir> <speed> <steps>\tmove media with <direction>, <speed>, <steps>",
	"  g,G\t\tdo paper_park(return media to gate)",
	"  x,X\tcreate calibrate data",	"  z,Z\tMedia alignment",
};
#define HELP_MSG_LINES	(sizeof(helpmsg)/sizeof(char *))


static void print_help()
{
	int i;
	for (i=0; i<HELP_MSG_LINES; i++)
		printf("%s\r\n", helpmsg[i]);
}

static int readline(const char *prompt, char *buffer, int bufsiz)
{
	int cnt = 0;

	printf("\r%s", prompt);
	while (cnt < bufsiz)
	{
		int ch = getchar();
		switch (ch)
		{
		case '\r':
		case '\n':
			printf("\r\n");
			return cnt;
		case '\b':	/* Backspace */
		case 0x7f:	/* Delete */
			if (cnt != 0)
			{
				printf("\b \b");
				--cnt;
			}
			break;
		default:
			if (ch >= ' ' && ch < 0x7f)
			{
				putchar(ch);
				buffer[cnt] = ch;
				++cnt;
			}
		}
	}
	return cnt;
}


#if 0
void init_device_information()
{
	int rs;
	unsigned short fpga_version;
	extern unsigned int sysinfo_get_uid_hi(void);
	extern unsigned int sysinfo_get_uid_lo(void);

	deviceinfo.uid_hi = sysinfo_get_uid_hi();
	deviceinfo.uid_lo = sysinfo_get_uid_lo();

	rs = fpga_get_version(&fpga_version);
	if (rs == 0)
		deviceinfo.fpga_version = fpga_version;
}


void print_banner()
{
	printf("%s ", deviceinfo.devicetype);
	printf("(Hardware V%d.%02d, FPGA V%d.%02d, Firmware V%d.%02d)\r",
		version_major(deviceinfo.hw_version), version_minor(deviceinfo.hw_version),
		version_major(deviceinfo.fpga_version), version_minor(deviceinfo.fpga_version),
		version_major(deviceinfo.fw_version), version_minor(deviceinfo.fw_version));
	printf("[UID = %08X-%08X]\r\n", deviceinfo.uid_hi, deviceinfo.uid_lo);
}
#endif

char cmdbuf[80];

int console(void)
{
	int rs;

	while (1) {
		unsigned int cmd, value, lightsource, side, device, address;
		unsigned int offset, vsmpup, vsmpdown, shift, siginac, sigacti, sih, sil, sil_plus;
		unsigned int lightsel, side_a, side_b, channelid;
		unsigned int resolution, mode;
		unsigned char afevalue;
		struct afe_test_pattern_generator testpara;
		char *p, *cp;

		memset(cmdbuf, 0, sizeof(cmdbuf));
		readline(cmdprompt, cmdbuf, sizeof(cmdbuf));

		p = strchr(cmdbuf, '\n');
		if (p)
			*p = ' ';	// replace Newline (0x0A) with SPACE (0x20)
		p = cmdbuf;
		skip_blanks(&p);	// skip preceding blanks before command
		if (strlen(p) == 0)
			continue;
		cp = p++;		//save command char position
		cmd = *cp;
		skip_blanks(&p);	// skip preceding blanks before parameters
		switch (cmd) {
		case 'h':
		case 'H':
			if (*p) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
			print_help();
			break;
		case '.':
			rs = sscanf(p, "%x", &address);
			if (rs != 1 ) {
				print_cmderror((p - cmdbuf), "invalid afe parameters\n");
				break;
			}
			fpga_readl(&value, (char *)address);
			printf("fpga_readl(0x%08x) = %x\r\n", address, value);
			break;
		case 'w':
			rs = sscanf(p, "%x%x", &address, &value);
			if (rs != 2 ) {
				print_cmderror((p - cmdbuf), "invalid afe parameters\n");
				break;
			}
			fpga_writel(value, (char *)address);
			printf("fpga_writel %x to address (0x%08x)\r\n", value, address);
			break;
		case '1':
			rs = sscanf(p, "%d%x", &value, &checkscanner.afe_info.flagpixlen);
			//scanner_set_all_digitiser_default_config();
			checkscanner.afe_info.outdataen = (value==0)?AFE_07_OUTPD:0;
			printf("scanner_cmos_output_config %s!\r\n", (checkscanner.afe_info.outdataen==0)?"flagenabled":"flagclosed");
			scanner_cmos_output_config(&checkscanner, checkscanner.hwinfo.sectinfo_a->digitiser_id);
			break;
		case '2':
			//check_scanner_get_digitiser_default_config();
			break;
		case '3':
			rs = sscanf(p, "%d%d%x%02x", &device, &channelid, &address, &afevalue);
			if (rs != 4 ) {
				print_cmderror((p - cmdbuf), "invalid afe parameters\n");
				break;
			}
			if(device == CHECK_SCANUNIT)
			{
				scanner_afe_setvalue(&checkscanner, channelid, address, afevalue);
			}
			break;
		case '4':
			rs = sscanf(p, "%d%d%x", &device, &channelid, &address);
			if (rs != 3 ) {
				print_cmderror((p - cmdbuf), "invalid afe parameters\n");
				break;
			}

			if(device == CHECK_SCANUNIT)
			{
				if(channelid > checkscanner.hwinfo.sectinfo_a->digitiser_id)
				{
					print_cmderror((p - cmdbuf), "invalid afe channel id\n");
					break;
				}
				scanner_afe_getvalue(&checkscanner, channelid, address, &afevalue);
			}
			printf("do scanner_afe_getvalue(%s:afeid %x, address 0x%x) = %x\r\n", (device==CHECK_SCANUNIT)?"checkscanner":"ocrscanner", channelid, address, afevalue);
			break;
		case '5':
			rs = sscanf(p, "%d%d%x", &device, &channelid, &value);
			if(device == CHECK_SCANUNIT)
			{
				checkscanner.afe_info.pll_exdiv_sel = value;
				scanner_afe_pll_dll_config(&checkscanner, channelid);
			}
			printf("do scanner_afe_pll_dll_config(%s:afeid %x, pll_exdiv_sel 0x%x)\r\n", (device==CHECK_SCANUNIT)?"checkscanner":"ocrscanner", channelid, value);
			break;
		case '6':
			rs = sscanf(p, "%d%d%x", &device, &channelid, &value);
			scanner_set_afe_clock_monitor_source(&checkscanner, channelid, value);
			printf("do scanner_set_afe_clock_monitor_source(%s:feid %x, monclkcrl1 0x%x)\r\n", (device==CHECK_SCANUNIT)?"checkscanner":"ocrscanner", channelid, value);
			break;
		case '7':
			rs = sscanf(p, "%x%x%04x%x%x", &testpara.output_type, &testpara.output_invert, &testpara.pgcode, &testpara.pgwidth1, &testpara.pgwidth2);
			if (rs != 5 ) {
				print_cmderror((p - cmdbuf), "invalid afe test pattern generetor parameters\n");
				break;
			}
                        scanner_afe_test_pattern_generator_config(&checkscanner, 0, testpara);
			break;
		case 'a':
		case 'A':
			rs = sscanf(p, "%x%x%d", &resolution, &mode, &side);
			if (rs != 3 || resolution > 5) {
				print_cmderror((p - cmdbuf), "invalid scanning parameters\n");
				break;
			}

			scanning_ctrl.scanmode = mode;
			scanning_ctrl.hwresolution = resolution;
			printf("do adjust_afe_offset(resolution %x, mode %x, sideid %d)\r\n", scanning_ctrl.hwresolution, scanning_ctrl.hwmode, side);
			rs = check_scanner_adjust_afe_offset(scanning_ctrl.hwresolution, scanning_ctrl.scanmode, side);
			if(rs== 0)
				check_scanner_save_ajustable_config(scanning_ctrl.hwresolution, scanning_ctrl.scanmode, CONFIG_ADC, side);
			break;
		case 'c':
		case 'C':
			rs = sscanf(p, "%x%x%d%x", &resolution, &mode, &side, &lightsel);
			if (rs != 4 || resolution > 5 ) {
				print_cmderror((p-cmdbuf), "invalid scanning parameters\n");
				break;
			}
			printf("do adjust_cis_led_brightness(resolution%x, mode%x, sideid%d, lightsel%x)\r\n", resolution, mode, side, lightsel);
			{
				scanning_ctrl.scanmode = mode;
				scanning_ctrl.hwresolution = resolution;
				//rs = spv_do_scanning(&checkscanner, IAM_A_PAGE, scanning_ctrl, 0);
				rs = check_scanner_adjust_cis_led_brightness(scanning_ctrl.hwresolution, scanning_ctrl.scanmode, side, lightsel);
				if( rs== 0)
					check_scanner_save_ajustable_config(scanning_ctrl.hwresolution, scanning_ctrl.scanmode, CONFIG_CIS, value);
			}
			break;
		case 'e':
			rs = sscanf(p, "%d %x %d", &side, &value, &lightsource);
			if (rs != 3 || value > 0x7ea4 || lightsource > 7) {
				print_cmderror((p-cmdbuf), "invalid ontime value or lightsource\n");
				break;
			}
			printf("do scanner_set_led_ontime(sideid%d, value%x,lightsource%d)\r\n", side, value, lightsource);
			check_scanner_set_led_ontime(side, value, lightsource);
			break;
		case 'E':
			rs = sscanf(p, "%d%d", &side, &value);
			printf("do scanner_reset_cisled_config(%d)\r\n", value);
			check_scanner_reset_cisled_config(value);
			break;
		case 'f':
		case 'F':
			if (*p) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
//			scanner_show_hwinfo();
			break;
		case 'g':
			if (*p) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
			printf("calling spv_do_media_return(%s)\r\n", (cmd == 'g') ? "DO_MEDIA_RETURN_GATE" : "DO_MEDIA_EJECT_GATE");
//			spv_do_media_return((cmd == 'g') ? DO_MEDIA_RETURN_GATE : DO_MEDIA_EJECT_GATE);
			break;
		case 'G':
			rs = sscanf(p, "%d%d%x", &device, &side, &value);
			if (rs != 3 || value > 255) {
				print_cmderror((p - cmdbuf), "invalid gain parameter\n");
				break;
			}
			printf("do scanner_set_afe_gainmsb_config(device%d,value%x)\r\n", device, value);
			if(device == CHECK_SCANUNIT)
				scanner_set_afe_gainmsb_config(&checkscanner, side, value);
			break;
		case 'i':
		case 'I':
			rs = sscanf(p, "%d", &device);
			if (rs != 1) {
				print_cmderror((p - cmdbuf), "invalid device parameter\n");
				break;
			}
			printf("do scanner_set_default_config(device%d)\r\n", device);
			//check_scanner_set_sensor_default_config();
			break;
		case 'k':
		case 'K':
			rs = sscanf(p, "%d%d", &device, &value);
			if (rs != 2 || value > 1) {
				print_cmderror((p-cmdbuf), "invalid scanning status\n");
				break;
			}
			printf("unint%s %s scanning without media movement\r\n", (device == CHECK_SCANUNIT) ? "CHECK" : "OCR", (value == 1) ? "start" : "stop");

			if(device == CHECK_SCANUNIT)
			{
				if (value)
				{
					scanner_start_scanning(&checkscanner, IAM_A_PAGE);
					vTaskDelay(500/ portTICK_RATE_MS);
					scanner_stop_scanning(&checkscanner, IAM_A_PAGE);
				}
				else
				{
					scanner_stop_scanning(&checkscanner, IAM_A_PAGE);
				}
			}
			break;
		case 'l':
			rs = sscanf(p, "%d%d", &device, &value);
			if (rs != 2 || value > 1) {
				print_cmderror((p-cmdbuf), "invalid value\n");
				break;
			}
			printf("turn %s unint%s scanning light sources\r\n", (value == 1) ? "on" : "off", (device == CHECK_SCANUNIT) ? "CHECK" : "OCR");
			if(device == CHECK_SCANUNIT)
			{
				if (value)
					scanunit_turnon_lights(&checkscanner);
				else
					scanunit_turnoff_lights(&checkscanner);
			}
			break;
		case 'L':
			rs = sscanf(p, "%d %x %x", &device, &side_a, &side_b);
			if (rs != 3 || side_a > 0xff || side_b > 0xff) {
				print_cmderror((p-cmdbuf), "invalid parameter\n");
				break;
			}
			printf("turn on unit %s lightsource side_a %x side_b %x\r\n", (device == CHECK_SCANUNIT) ? "CHECK" : "OCR", side_a, side_b);
			if(device == CHECK_SCANUNIT)
			{
				scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_a, side_a);
				scanner_set_cis_lightsource(&checkscanner, checkscanner.hwinfo.sensor_b, side_b);
			}
			break;
		case 'm':
			{
				int dir;
				rs = sscanf(p, "%d", &dir);
				if (rs != 1 || dir < 0 || dir > 1 ) {
					print_cmderror((p-cmdbuf), "invalid media move parameters\n");
					break;
				}
				rs = Actions_Move(dir);
//				rs = cardpath_step(dir, speed, steps);
				printf("Actions_Move: dir=%d.\r\n", dir, rs);
			}
			break;
		case 'M':
			{
				int dir, speed, steps;
				rs = sscanf(p, "%d %d", &dir, &steps);
				if (rs != 2 || dir < 0 || dir > 1 || steps < 0 || steps > 40000) {
					print_cmderror((p-cmdbuf), "invalid media move parameters\n");
					break;
				}
				motor1_run(dir, steps);
//				rs = cardpath_step(dir, speed, steps);
				printf("motor_step: dir=%d, speed=%d, steps=%d. return val = %d\r\n", dir, speed, steps, rs);
			}
			break;
		case 'o':
		case 'O':
			rs = sscanf(p, "%d%d%x", &device, &side, &offset);
			if (rs != 3 || offset > 0xff || side > 1 ) {
				print_cmderror((p-cmdbuf), "invalid AFE offset value or invalid side value\n");
				break;
			}
			printf("do set_afe_offset(%s:%d, 0x%02X)\r\n", (device== CHECK_SCANUNIT)?"checkscanner":"ocrscanner", side, offset);

			if(device == CHECK_SCANUNIT)
				scanner_set_afe_offset(&checkscanner, side, offset);
			break;
		case 'p':
			if (*p) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
			printf("print photosensor status\r\n");
			sensor_test();
			break;
		case 'P':
			rs = sscanf(p, "%d", &value);
			if (rs != 1 || value > 1) {
				print_cmderror((p-cmdbuf), "invalid unit number (should be 0 - paperpath; 1 - acceptor)\n");
				break;
			}
			printf("spv_do_sensor_calibration(%d)\r\n", value);
			check_scanner_calibrate_adjustable_paramters();
//			spv_do_sensor_calibration(value);
			break;
		case 't':
			rs = sscanf(p, "%x %x %x ", &sih, &sil, &sil_plus);
			if (rs != 3) {
				print_cmderror((p-cmdbuf), "invalid parameter number\n");
				break;
			}
			printf("do scanner_set_cis_commom_config(%x, %x, %x)\r\n", sih, sil, sil_plus);
			//check_scanner_set_cis_commom_config(sih, sil, sil_plus);
			break;
		case 'T':
			rs = sscanf(p, "%d %d%x %x", &device, &side, &siginac, &sigacti);
			if (rs != 4) {
				print_cmderror((p-cmdbuf), "invalid parameter number\n");
				break;
			}
			printf("do set_afe_siginac_config(%s:%d, %x, %x)\r\n", (device== CHECK_SCANUNIT)?"checkscanner":"ocrscanner", side, siginac, sigacti);
			if(device == CHECK_SCANUNIT)
				scanner_set_afe_siginac_config(&checkscanner, side, siginac, sigacti);
			break;
		case 'u':
		case 'U':
			rs = sscanf(p, "%d %d%x %x %x", &device, &side, &vsmpup, &vsmpdown, &shift);
			if (rs != 5) {
				print_cmderror((p-cmdbuf), "invalid parameter number\n");
				break;
			}
			printf("do set_afe_vsmpup_config(%s:%d, %x, %x, %x)\r\n", (device== CHECK_SCANUNIT)?"checkscanner":"ocrscanner", side, vsmpup, vsmpdown, shift);
			if(device == CHECK_SCANUNIT)
				scanner_set_afe_vsmpup_config(&checkscanner, side, vsmpup, vsmpdown);
			fpga_writel(shift, (char *)0x30200080);
			printf("fpga_writel %x to address (0x%08x)\r\n", shift, 0x30200080);
			break;
		case 'r':
		case 'R':
			if (*p) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
//			spv_do_reset(0);
			break;
		case 's':
		case 'S':
			rs = sscanf(p, "%d%x%x", &device, &resolution, &mode);
			if (rs != 3) {
				print_cmderror((p-cmdbuf), "invalid scanning parameter\n");
				break;
			}
			if(device == CHECK_SCANUNIT)
			{
				scanning_ctrl.scanmode = mode;
				scanning_ctrl.hwresolution = resolution;
				scanner_set_sensor_config(&checkscanner, scanning_ctrl.hwresolution, scanning_ctrl.scanmode, SIDE_A);
			}

			if (cmd == 's') {
				check_scanner_setup_ajustable_config(scanning_ctrl.hwresolution, scanning_ctrl.hwmode);
				if(device == CHECK_SCANUNIT)
				{
					scanunit_turnon_lights(&checkscanner);
					scanunit_start_scanning(&checkscanner);
//					cardpath_cardscan((move_scantype_t)(check_scanner_modetospeedtype(paraconfig.para)));
					scanunit_stop_scanning(&checkscanner);
				}
			}else
				printf("scanunit %s scanunit_set_scanning_mode with dpi=%d, mode = 0x%02x\r\n", (device == CHECK_SCANUNIT) ? "CHECK" : "OCR", resolution, mode);

			break;
		case 'd':
		case 'D':
			if (*p) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
			break;
		case 'x':
		case 'X':
			rs = sscanf(p, "%d", &device);
			if (rs != 1) {
				print_cmderror((p-cmdbuf), "unnecessary parameter\n");
				break;
			}
//			spv_do_scan_calibration(1, 100);
			if(device == CHECK_SCANUNIT)
			{
				motor1_run(0, 500);
				check_scanner_calibrate_adjustable_paramters();
			}
			break;
		default:
			print_cmderror((cp-cmdbuf), "invalid command\n");
		}
	}
	return 0;
}


