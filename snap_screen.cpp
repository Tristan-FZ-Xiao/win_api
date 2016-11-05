#include<windows.h>
#include <wchar.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"

/*	What we need in the module:
 *	(DONE) 1. Get picture from desk screen snap, store it in the memory.
 *	(DONE) 2. Do some BMP transfer: 1) color to black-white; 2) color to gray;
 */

static unsigned char get_gray(unsigned char *ptr, int mode)
{
	unsigned char new_color;

	if (mode == BINARY_MEAN) {
		new_color = (unsigned char)((float)(ptr[0] + ptr[1] + ptr[2]) / 3.0f);
		//TRACE(T_INFO, "ptr[%d]'s new color %d", i, new_color);	
	}
	else if (mode == BINARY_WEIGHTED_MEAN) {
		new_color = (unsigned char)((float)ptr[0] * 0.114f + (float)ptr[1] * 0.587f 
			+ (float)ptr[2] * 0.299f);
	}
	return new_color;
}

/* Get the average gray value: get value from all around 9.
 *	x1 x2 x3
 *	x4 T  x5
 *	x6 x7 x8
 */
unsigned char get_average_color(unsigned char *gray, int x, int y, int w, int h)
{
	int rs = 0;
	/* If it's black or white, just return the original value */
	if (*(gray + (y * w + x) * 4) == 255 || *(gray + (y * w + x) * 4) == 0) {
		return *(gray + (y * w + x) * 4);
	}

	rs = *(gray + (y * w + x) * 4)									// T
		+ (x == 0 || y == 0 ? 255 : *(gray + ((y - 1) * w + x - 1) * 4))			// x1
		+ (y == 0 ? 255 : *(gray + ((y - 1) * w + x) * 4))					// x2
		+ (x == w - 1 || y == 0 ? 255 : *(gray + ((y - 1) * w + x + 1) * 4))		// x3
		+ (x == 0 ? 255 : *(gray + (y * w + x - 1) * 4))					// x4
		+ (x == w - 1 ? 255 : *(gray + (y * w + x + 1) * 4))					// x5
		+ (x == 0 || y == h - 1 ? 255 : *(gray + ((y + 1) * w + x - 1) * 4))		// x6
		+ (y == h - 1 ? 255 : *(gray + ((y + 1) * w + x) * 4))				// x7
		+ (x == w - 1 || y == h - 1 ? 255 : *(gray + ((y + 1) * w + x + 1) * 4));	// x8

		return (unsigned char) (rs / 9);
}

static unsigned char get_color(unsigned char *gray, int x, int y, int w, int h)
{
	//TRACE(T_INFO, "value %d", *(gray + (y * w + x) * 4));
	return *(gray + (y * w + x) * 4);
}

/* convert picture from color to gray */
int convert_gray(struct t_bmp *in_ptr, int mode)
{
	unsigned int i = 0;
	unsigned char *ptr = NULL;
	unsigned char new_color;	

	if (in_ptr == NULL) {
		TRACE(T_ERROR, "in_ptr == NULL");	
		return ERR_COMMON_NULL_ERROR;
	}
	//TRACE(T_INFO, "Do the gray convert process");	
	for (ptr = in_ptr->data; i < in_ptr->len; i += 4) {
		new_color = get_gray((ptr + i), mode);
		ptr[i + 0] = new_color;
		ptr[i + 1] = new_color;
		ptr[i + 2] = new_color;
		ptr[i + 3] = new_color;
	}
	return ERR_NO_ERR;
}

int convert2blackwhite(struct t_bmp *in_ptr, int mode, int thresh)
{
	int h = in_ptr->bih.biHeight;
	int w = in_ptr->bih.biWidth;
	int i, j; 
	int SW = 160;
	unsigned char *ptr = new unsigned char[in_ptr->len];
	int flag = 0;

	if (0 == thresh) {
		thresh = SW;
	}

	for (i = 0; i < h; i ++) {
		for (j = 0; j < w; j ++) {
			if (mode == ONLY_BLACK ||
				mode == ONLY_WHITE) {
				if (get_color(in_ptr->data, j, i, w, h) > thresh) {
					flag = 1;			
				}
				else {
					flag = 0;
				}
			}
			else {
				if (get_average_color(in_ptr->data, j, i, w, h) > thresh) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
			if (flag == 1) {
				//TRACE(T_INFO, "x %d y %d WHITE", j, i);
				*(ptr + ((i * w + j) * 4)) = 255;
				*(ptr + ((i * w + j) * 4 + 1)) = 255;
				*(ptr + ((i * w + j) * 4 + 2)) = 255;
				*(ptr + ((i * w + j) * 4 + 3)) = 255;
			}
			else {
				//TRACE(T_INFO, "x %d y %d BLACK", j, i);
				*(ptr + ((i * w + j) * 4)) = 0;
				*(ptr + ((i * w + j) * 4 + 1)) = 0;
				*(ptr + ((i * w + j) * 4 + 2)) = 0;
				*(ptr + ((i * w + j) * 4 + 3)) = 0;
			}
		}
	}
	delete(in_ptr->data);
	in_ptr->data = ptr;
	return ERR_NO_ERR;
}

static int __get_screen_rect(struct t_bmp *in_ptr, RECT target_rc, struct t_bmp *out_ptr)
{
	int i = 0;
	int rc_width = 0;
	int len = 0;
	unsigned char *t_buf =  NULL;

	rc_width = target_rc.right - target_rc.left;
	/* align 4 Byte */ 
	//rc_width = (rc_width % 4 == 0) ? (rc_width / 4) * 4 : ((rc_width / 4) + 1) * 4;
	len = (target_rc.bottom - target_rc.top) * rc_width * 4;
	t_buf = new unsigned char[len];

	/* As pixel - buffer mapping from the left-bottom, do the transfer. */
	for (; i < (target_rc.bottom - target_rc.top); i ++) {
		memcpy(t_buf + rc_width * i * 4, in_ptr->data + ((in_ptr->bih.biHeight - target_rc.bottom + i) * 
			in_ptr->bih.biWidth + target_rc.left) * 4, rc_width * 4);
	}
	memcpy(&out_ptr->bfh, &in_ptr->bfh, sizeof(BITMAPFILEHEADER));
	memcpy(&out_ptr->bih, &in_ptr->bih, sizeof(BITMAPINFOHEADER));

	out_ptr->bfh.bfType			= 0x4d42;
	out_ptr->bfh.bfSize			= len + 54;
	out_ptr->bfh.bfReserved1		= 0;
	out_ptr->bfh.bfReserved2		= 0;
	out_ptr->bfh.bfOffBits			= 54;

	out_ptr->bih.biSize			= 40;
	out_ptr->bih.biWidth			= rc_width;
	out_ptr->bih.biHeight			= target_rc.bottom - target_rc.top;
	out_ptr->bih.biPlanes			= 1;
	out_ptr->bih.biBitCount			= 32;
	out_ptr->bih.biCompression		= BI_RGB;
	out_ptr->bih.biSizeImage		= len;
	out_ptr->bih.biXPelsPerMeter		= 0;
	out_ptr->bih.biYPelsPerMeter		= 0;
	out_ptr->bih.biClrUsed			= 0;
	out_ptr->bih.biClrImportant		= 0;

	out_ptr->data = t_buf;
	out_ptr->len = len;

	return ERR_NO_ERR;
}

int get_screen_rect(struct t_bmp *in_ptr, RECT target_rc, struct t_bmp *out_ptr)
{
	if (out_ptr == NULL || in_ptr == NULL) {
		return ERR_COMMON_PARAM_ERR;
	}
	if (target_rc.bottom < 0 || target_rc.left < 0 || target_rc.right < 0 || target_rc.top < 0
		|| target_rc.right < target_rc.left || target_rc.bottom < target_rc.top
		|| target_rc.right > in_ptr->bih.biWidth || target_rc.bottom > in_ptr->bih.biHeight) {
		return ERR_COMMON_PARAM_ERR;
	}
	return __get_screen_rect(in_ptr, target_rc, out_ptr);
}

int load_picture(wchar_t *path, struct t_bmp *out_ptr)
{
	unsigned int file_len = 0;
	unsigned long read_len = 0;
	int ret = 0;

	if (!path || !out_ptr) {
		return ERR_COMMON_PARAM_ERR;
	}

	HANDLE h_file=CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL, 0);

	if (h_file == NULL) {
		return ERR_FILE_NOT_FOUND;
	}

	file_len = GetFileSize(h_file, NULL);
	if (file_len == INVALID_FILE_SIZE) {
		return ERR_GET_FILE_SIZE_FAILED;
	}

	if (file_len < 54) {
		return ERR_GET_FILE_SIZE_FAILED;
	}

	out_ptr->len = file_len - 54;
	out_ptr->data = new unsigned char[out_ptr->len];
	ret = ReadFile(h_file, &out_ptr->bfh, sizeof(BITMAPFILEHEADER), (LPDWORD)&read_len, NULL);
	ret = ReadFile(h_file, &out_ptr->bih, sizeof(BITMAPINFOHEADER), (LPDWORD)&read_len, NULL);
	ret = ReadFile(h_file, out_ptr->data, out_ptr->len, (LPDWORD)&read_len, NULL);
	::CloseHandle(h_file);
	if (ret == TRUE) {
		return ERR_NO_ERR;
	}
	else {
		TRACE(T_ERROR, "Read File %s error, errno: %d", path, GetLastError());
		return ERR_READ_FILE_FAILED;
	}
}

int save_picture(wchar_t *path, struct t_bmp *in_ptr)
{
	if (path && in_ptr) {
		DWORD dwSize;
		HANDLE hFile=CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, 0);
		WriteFile(hFile, (void *)&in_ptr->bfh, sizeof(BITMAPFILEHEADER), &dwSize, 0);
		WriteFile(hFile, (void *)&in_ptr->bih, sizeof(BITMAPINFOHEADER), &dwSize, 0);
		WriteFile(hFile, (void *)in_ptr->data, in_ptr->len, &dwSize, 0);
		::CloseHandle(hFile);
	}
	return ERR_NO_ERR;
}

int get_screen(HWND hwnd, wchar_t *path, struct t_bmp *out_ptr)
{
	HWND desk_hwnd=::GetDesktopWindow();
	RECT target_rc;
	POINT lp = {0, 0};
	int ret = 0;

	::GetClientRect(hwnd,&target_rc);
	ClientToScreen(hwnd, &lp);
	if (lp.x < 0 || lp.y < 0) {
		return ERR_WINDOW_NOT_FOUND;
	}
	target_rc.left += lp.x;
	target_rc.right += lp.x;
	target_rc.top += lp.y;
	target_rc.bottom += lp.y;

	HDC desk_dc=GetDC(desk_hwnd);
	HBITMAP desk_bmp=::CreateCompatibleBitmap(desk_dc, target_rc.right - target_rc.left,
		target_rc.bottom - target_rc.top);
	HDC mem_dc=::CreateCompatibleDC(desk_dc);
	SelectObject(mem_dc,desk_bmp);
	BitBlt(mem_dc, 0, 0, target_rc.right - target_rc.left, target_rc.bottom - target_rc.top,
		desk_dc, target_rc.left, target_rc.top, SRCCOPY);

	BITMAP bmInfo;
	DWORD bm_dataSize;
	unsigned char *bm_data;

	GetObject(desk_bmp, sizeof(BITMAP), &bmInfo);

	bm_dataSize = bmInfo.bmWidthBytes * bmInfo.bmHeight;
	bm_data = new unsigned char[bm_dataSize];

	out_ptr->bfh.bfType			= 0x4d42;
	out_ptr->bfh.bfSize			= bm_dataSize + 54;
	out_ptr->bfh.bfReserved1		= 0;
	out_ptr->bfh.bfReserved2		= 0;
	out_ptr->bfh.bfOffBits			= 54;

	out_ptr->bih.biSize			= 40;
	out_ptr->bih.biWidth			= bmInfo.bmWidth;
	out_ptr->bih.biHeight			= bmInfo.bmHeight;
	out_ptr->bih.biPlanes			= 1;
	out_ptr->bih.biBitCount			= bmInfo.bmBitsPixel;
	out_ptr->bih.biCompression		= BI_RGB;
	out_ptr->bih.biSizeImage		= bm_dataSize;
	out_ptr->bih.biXPelsPerMeter		= 0;
	out_ptr->bih.biYPelsPerMeter		= 0;
	out_ptr->bih.biClrUsed			= 0;
	out_ptr->bih.biClrImportant		= 0;

	TRACE(T_INFO, "file size %d width %d height %d bmWidthByte %d bmHeight %d\n", out_ptr->bfh.bfSize,
		out_ptr->bih.biWidth, out_ptr->bih.biHeight, bmInfo.bmWidthBytes, bmInfo.bmHeight);

	::GetDIBits(desk_dc, desk_bmp, 0, bmInfo.bmHeight, bm_data, (BITMAPINFO *)&out_ptr->bih,
		DIB_RGB_COLORS);

	out_ptr->len = bm_dataSize;
	out_ptr->data = bm_data;

	/* do the convertion */
	//ret = convert_gray(out_ptr, BINARY_MEAN);
	//ret = convert2blackwhite(out_ptr, BOTH_BLACKWHITE, 0);

	if (ret != ERR_NO_ERR) {
		TRACE(T_ERROR, "binary process failed");
		return ERR_DO_BINARY_CONVERT_FAILED; 
	}

	ret = save_picture(path, out_ptr);
	if (ret != ERR_NO_ERR) {
		TRACE(T_ERROR, "save file failed");
		return ERR_SAVE_FILE_FAILED;
	}
	return ERR_NO_ERR;
}

void unit_test_picture(void)
{
	struct t_bmp tmp = {};

	load_picture(L"D://1.bmp", &tmp);
	save_picture(L"D://1.1.bmp", &tmp);
}

#include <stdio.h>

void output_character(unsigned char *ptr, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i && !(i % 16)) {
			printf("\n");
		}
		printf("0x%02x, ", *(ptr + i));
	}
}

void unit_test_get_screen_rect(void)
{
	struct t_bmp input = {};
	struct t_bmp output = {};
	int ret = 0;
	RECT target_rc = {};

	/* This should be the rect of role's level: likes Lv 14 */
#if 0
	/* Get the role's HP/ position*/
	target_rc.left = 204;
	target_rc.right = 250;
	target_rc.top = 283;
	target_rc.bottom = 291;
#endif
	target_rc.left = 0;
	target_rc.right = 5;
	target_rc.top = 0;
	target_rc.bottom = 8; 

	ret = load_picture(L"D://0.3.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return;
	}

	ret = get_screen_rect(&input, target_rc, &output);
	ret = save_picture(L"D://z.3.bmp", &output);
	delete[] input.data;
	output_character(output.data, output.len);
	delete[] output.data;
}

void unit_test_memleak_get_screen(void)
{
	while (1) {
		unit_test_get_screen_rect();
		Sleep(500);
	}
}

int unit_test_get_screen(void)
{
	int ret = 0;
	struct t_bmp target = {};

	ret = auto_mob_init();
#if 0
	HWND hwnd = FindWindow(NULL, L"无标题 - 记事本");
	if (hwnd == NULL) {
		return ERR_HWND_NOT_FOUND;
	}
	ret = get_screen(hwnd, L"D://1.bmp", &target);
#else
	auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
	ret = get_screen(auto_mob.mob_hwnd, L"D://role_i.bmp", &target);
#endif
	if (ret != ERR_NO_ERR)
		return ret;
	delete[] target.data;
	return ERR_NO_ERR;
}

void unit_test_get_info_from_dnf(void)
{
	int ret = 0;
	struct t_bmp input = {};
	struct t_bmp output = {};
	RECT target_rc = {};

	ret = auto_mob_init();
	auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
	if (!auto_mob.mob_hwnd) {
		return;
	}
	ret = get_screen(auto_mob.mob_hwnd, L"D://game.bmp", &input);
	ret = save_picture(L"D://map.bmp", &input);

	/* This should be the rect of role's level: likes Lv 14 */
	target_rc.left	= 45;
	target_rc.right	= 70;
	target_rc.top	= 588;
	target_rc.bottom	= 599;

	ret = get_screen_rect(&input, target_rc, &output);
	if (ret != ERR_NO_ERR) {
		delete[] input.data;
		return;
	}
	ret = convert_gray(&output, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&output, ONLY_BLACK, 0);
	ret = save_picture(L"D://role_level.bmp", &output);

	/* This should be the rect of gold */
	target_rc.left	= 624;
	target_rc.right	= 688;
	target_rc.top	= 532;
	target_rc.bottom	= 546;

	ret = get_screen_rect(&input, target_rc, &output);
	if (ret != ERR_NO_ERR) {
		delete[] input.data;
		return;
	}
	ret = convert_gray(&output, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&output, ONLY_BLACK, 0);
	ret = save_picture(L"D://gold.bmp", &output);

	delete[] output.data;
	delete[] input.data;
}

//#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	//unit_test_get_screen_rect();
	unit_test_get_info_from_dnf();
	//unit_test_get_screen();
}
#endif /* __OWN_MAIN__ */
