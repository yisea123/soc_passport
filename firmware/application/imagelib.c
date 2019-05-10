#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern const struct scancalib_data_buf buf;

int img_rawdata_section(unsigned char *rawdata, unsigned char *outdata, int totalpixels, int linepixels, int start_pixel, int end_pixel, int unitid)
{
	int i, j, lines, width;

	if (rawdata==NULL || totalpixels<0 || totalpixels<linepixels ||
		start_pixel<0 || start_pixel >linepixels || end_pixel<0 || end_pixel > linepixels)
		return -1;

	lines = totalpixels/linepixels;
	width = end_pixel - start_pixel;

	for (i=0; i<lines; i++){
		for (j = 0; j < width; j++) {
			outdata[i*width + j] = rawdata[i*linepixels+start_pixel + j];
		}
	}

	return 0;
}

int img_rawdata_single_color_averages_by_line(unsigned char *rawdata, int totalpixels, int linepixels, int *average)
{
	unsigned int *ptr = (unsigned int *)rawdata;
	int i, j, lines;

	if (rawdata==NULL || totalpixels<0 || totalpixels<linepixels ||average==NULL)
		return -1;
	memset((void *)average, 0, linepixels*sizeof(unsigned char));

	lines = totalpixels/linepixels;
	for (i=0; i<lines; i++)
	{
		for (j=0; j<linepixels; j++)
		{
			unsigned int pdata = *ptr++;
			average[j] += (pdata & 0xff);
		}
	}
	for (i=0; i<linepixels; i++)
	{
		average[i] /= lines;
	}
	return 0;
}

int img_rawdata_single_color_distribution(unsigned char *rawdata, int pixels, int *distribution)
{
	unsigned char *ptr = rawdata;
	int i;	

	if (rawdata == NULL || pixels < 0)
		return -1;

	memset((void *)distribution,  0, 256*sizeof(int));
	
	for (i=0; i<pixels; i++)
	{
		unsigned char pdata = *ptr++;
		distribution[pdata]++;
	}
	return 0;
}

int img_rawdata_color_averages_by_line(unsigned char *rawdata, int totalpixels, int linepixels, int *average_reds, int *average_greens, int *average_blues)
{
	unsigned int *ptr = (unsigned int *)rawdata;
	int i, j, lines;

	if (rawdata==NULL || totalpixels<0 || totalpixels<linepixels ||
		average_reds==NULL || average_greens==NULL || average_blues==NULL)
		return -1;
	memset((void *)average_reds, 0, linepixels*sizeof(int));
	memset((void *)average_greens, 0, linepixels*sizeof(int));
	memset((void *)average_blues, 0, linepixels*sizeof(int));
	lines = totalpixels/linepixels;
	for (i=0; i<lines; i++)
	{
		for (j=0; j<linepixels; j++)
		{
			unsigned int pdata = *ptr++;
			average_blues[j] += (pdata & 0xff);
			pdata >>= 8;
			average_greens[j] += (pdata & 0xff);
			pdata >>= 8;
			average_reds[j] += (pdata & 0xff);
		}
	}
	for (i=0; i<linepixels; i++)
	{
		average_reds[i] /= lines;
		average_greens[i] /= lines;
		average_blues[i] /= lines;
	}
	return 0;
}


int img_rawdata_color_distribution(unsigned char *rawdata, int pixels, int *distribution_red, int *distribution_green, int *distribution_blue)
{
	unsigned int *ptr = (unsigned int *)rawdata;
	int i;	

	if (rawdata == NULL || pixels < 0)
		return -1;

	memset((void *)distribution_red,  0, 256*sizeof(int));
	memset((void *)distribution_green, 0, 256*sizeof(int));
	memset((void *)distribution_blue,  0, 256*sizeof(int));
	
	for (i=0; i<pixels; i++)
	{
		int red, green, blue;
		unsigned int pdata = *ptr++;
		blue = (pdata & 0xff);
		distribution_blue[blue]++;
		pdata >>= 8;
		green = (pdata & 0xff);
		distribution_green[green]++;
		pdata >>= 8;
		red = (pdata & 0xff);
		distribution_red[red]++;
	}

	return 0;
}