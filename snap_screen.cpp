#include<windows.h>
#include <wchar.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"

/*	What we need in the module:
 *	1. Get picture from desk screen snap, store it in the memory.
 *	2. Do some BMP transfer: 1) color to black-white; 2) color to gray;
 *	3. Prepare for Opencv;
 *	4. Do some basic recognition likes: 1) Number; 2) small icon;
 */

struct t_bmp {
	char *data;
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

/* convert picture from color to gray */
int convert_gray(char *src, unsigned int len, int mode)
{
	unsigned int i = 0;
	char *ptr = NULL;
	unsigned char new_color;	

	if (src == NULL) {
		return ERR_COMMON_NULL_ERROR;
	}
	TRACE(T_INFO, "Do the binary process");	
	for (ptr = src; i < len; i += 3) {
		if (mode == BINARY_MEAN) {
			new_color = (unsigned char)((float)(ptr[i + 0] + ptr[i + 1] + ptr[i + 2]) / 3.0f);
			//TRACE(T_INFO, "ptr[%d]'s new color %d", i, new_color);	
		}
		else if (mode == BINARY_WEIGHTED_MEAN) {
			new_color = (unsigned char)((float)ptr[i + 0] * 0.114f + (float)ptr[i + 1] * 0.587f 
				+ (float)ptr[i + 2] * 0.299f);
		}
		ptr[i + 0] = new_color;
		ptr[i + 1] = new_color;
		ptr[i + 2] = new_color;
	}
	return ERR_NO_ERR;
}

int get_screen(HWND hwnd, const wchar_t *path, struct t_bmp *out_ptr)
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
	char *bm_data;

	GetObject(desk_bmp, sizeof(BITMAP), &bmInfo);

	bm_dataSize = bmInfo.bmWidthBytes * bmInfo.bmHeight;
	bm_data = new char[bm_dataSize];

	out_ptr->bfh.bfType			= 0x4d42;
	out_ptr->bfh.bfSize			= bm_dataSize + 54;
	out_ptr->bfh.bfReserved1		= 0;
	out_ptr->bfh.bfReserved2		= 0;
	out_ptr->bfh.bfOffBits			= 54;

	out_ptr->bih.biSize			= 40;
	out_ptr->bih.biWidth			= bmInfo.bmWidth;
	out_ptr->bih.biHeight			= bmInfo.bmHeight;
	out_ptr->bih.biPlanes			= 1;
	out_ptr->bih.biBitCount			= 24;
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
	ret = convert_gray(bm_data, bm_dataSize, BINARY_WEIGHTED_MEAN);
	if (ret != ERR_NO_ERR) {
		TRACE(T_ERROR, "binary process failed");
	}

	if (path) {
		DWORD dwSize;
		HANDLE hFile=CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, 0);
		WriteFile(hFile, (void *)&out_ptr->bfh, sizeof(BITMAPFILEHEADER), &dwSize, 0);
		WriteFile(hFile, (void *)&out_ptr->bih, sizeof(BITMAPINFOHEADER), &dwSize, 0);
		WriteFile(hFile, (void *)bm_data, out_ptr->len, &dwSize, 0);
		::CloseHandle(hFile);
	}
	return ERR_NO_ERR;
}

int get_screen_rect(struct t_bmp *in_ptr, RECT target_rc, struct t_bmp *out_ptr)
{
	int i = 0;
	int rc_width = target_rc.right - target_rc.left;
	if (rc_width < 0 || out_ptr == NULL || in_ptr == NULL) {
		return ERR_COMMON_PARAM_ERR;
	}
	rc_width = (rc_width % 4 == 0) ? (rc_width / 4) * 4 : ((rc_width / 4) + 1) * 4;
	int len = (target_rc.bottom - target_rc.top) * rc_width * 3;

	char *t_buf = new char[len];

	/* As pixel - buffer mapping from the left-bottom, do the transfer. */
	for (; i < (target_rc.bottom - target_rc.top); i ++) {
		memcpy(t_buf + rc_width * i * 3, in_ptr->data + ((in_ptr->bih.biHeight - target_rc.bottom + i) * 
			in_ptr->bih.biWidth + target_rc.left) * 3, rc_width * 3);
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
	out_ptr->bih.biBitCount			= 24;
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
	out_ptr->data = new char[out_ptr->len];
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

void unit_test_picture(void)
{
	struct t_bmp tmp = {};

	load_picture(L"D://1.bmp", &tmp);
	save_picture(L"D://1.1.bmp", &tmp);
}

void unit_test_get_screen_rect(void)
{
	struct t_bmp input = {};
	struct t_bmp output = {};
	int ret = 0;
	RECT target_rc = {};

	/* This should be the rect of role's level: likes Lv 14 */
	target_rc.left = 24;
	target_rc.right = 70;
	target_rc.top = 588;
	target_rc.bottom = 599;

	ret = load_picture(L"D://1.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return;
	}

	ret = get_screen_rect(&input, target_rc, &output);
	if (ret != ERR_NO_ERR) {
		delete[] input.data;
		return;
	}
	delete[] input.data;
	ret = save_picture(L"D://1.2.bmp", &output);
	delete[] output.data;
}

int unit_test_get_screen(void)
{
	int ret = 0;
	struct t_bmp target = {};

	ret = auto_mob_init();
#if 0
	//HWND hwnd = FindWindow(NULL, L"无标题 - 记事本");
	if (hwnd == NULL) {
		return ERR_HWND_NOT_FOUND;
	}
#else
	auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
#endif
	ret = get_screen(auto_mob.mob_hwnd, L"D://1.bmp", &target);
	if (ret != ERR_NO_ERR)
		return ret;
	delete[] target.data;
	return ERR_NO_ERR;

}

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	while (1) {
		unit_test_get_screen_rect();
		Sleep(500);
	}
}
#endif /* __OWN_MAIN__ */
