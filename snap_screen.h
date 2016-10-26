#ifndef __SNAP_SCREEN_H__
#define __SNAP_SCREEN_H__
#include <Windows.h>

struct t_bmp {
	unsigned char *data;
	unsigned int len;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
};

struct tag_rgba {
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char reservered;
};

enum {
	BINARY_WEIGHTED_MEAN = 1,
	BINARY_MEAN
};

extern int get_screen(HWND hwnd, wchar_t *path, struct t_bmp *out_ptr);
extern int save_picture(wchar_t *path, struct t_bmp *in_ptr);
extern int load_picture(wchar_t *path, struct t_bmp *out_ptr);
extern int get_screen_rect(struct t_bmp *in_ptr, RECT target_rc, struct t_bmp *out_ptr);

extern int convert2blackwhite(struct t_bmp *in_ptr);
extern int convert_gray(struct t_bmp *in_ptr, int mode);
extern unsigned char get_average_color(unsigned char *gray, int x, int y, int w, int h);

#endif /* __SNAP_SCREEN_H__ */