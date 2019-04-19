
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	"  l,L <action>\t\tturn ON/OFF CIS light sources",
	"  k,K <action>\t\tstart/stop scanning without media movement",
	"  s,S <mode>\t\tstart scanning",
	"  d,D\t\tstop scanning",
	"  r,R\t\treset scanning unit",
	"  e,E\t\tset led light(side, value, lightsource)",
	"  u,U\t\tset_afe_vsmpup_config(vsmpup, vsmpdown, shift)",
	"  t,T\t\tset_afe_siginac_config(siginac, sigacti)",
	"  i,I\t\tinitialize scanning unit with default configuration"
	"  o,O <side> <offset>\n\t\tset image digitisers (AFE) offset.",
	"  p\t\tprint photosensor status.",
	"  P\t\tcalibrate photosensors.",
	"  a,A <side>\tautomatically adjust image digitisers (AFE) offset.\n\t\t<side> = (0: side A; 1: side B)",
	"  c,C <side>\tautomatically adjust image sensors (CIS) LEDs brightness\n\t\t<side> = (0: side A; 1: side B)",
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


char cmdbuf[80];

int console(void)
{
	int rs;

	while (1) {
		int cmd, value, lightsource, side;
		unsigned int offset, vsmpup, vsmpdown, shift, siginac, sigacti, sih, sil, sil_plus;
		unsigned int lightsel,side_a,side_b, channelid;
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
		case '1':
			nvram_write_test();
			printf("nvram write is ok\n\r");
			break;
		case '2':
			nvram_read_test();
			printf("nvram read is ok\n\r");
			break;
		case '3':
			nvm_write_test();
			printf("nvm write is ok\n\r");
			break;
		case '4':
			nvm_read_test();
			printf("nvm read is ok\n\r");
			break;
		case '5':
			nvm_rw_test();
			printf("nvm read is ok\n\r");
			break;
		default:
			print_cmderror((cp-cmdbuf), "invalid command\n");
		}
	}
	return 0;
}


