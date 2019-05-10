#ifndef __IMAGELIB_H__
#define __IMAGELIB_H__

int img_rawdata_single_color_averages_by_line(unsigned char *rawdata, int totalpixels, int linepixels, int *average);
int img_rawdata_single_color_distribution(unsigned char *rawdata, int pixels, int *distribution);
int img_rawdata_section(unsigned char *rawdata, unsigned char *outdata,int totalpixels, int linepixels, int start_pixel, int end_pixel);
int img_rawdata_color_distribution(unsigned char *rawdata, int pixels, int *distribution_red, int *distribution_green, int *distribution_blue);
int img_rawdata_color_averages_by_line(unsigned char *rawdata, int totalpixels, int linepixels, int *average_reds, int *average_greens, int *average_blues);

#endif /* __IMAGELIB_H__ */
