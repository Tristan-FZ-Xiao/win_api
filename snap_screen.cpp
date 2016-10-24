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

int binary_process(struct t_bmp *ptr)
{
	int i = 0;

	if (ptr == NULL) {
		return ERR_COMMON_NULL_ERROR;
	}

	for (; i < ptr->len; i++) {
		;
	}
	return ERR_NO_ERR;
}

int get_screen(HWND hwnd, const wchar_t *path, struct t_bmp *out_ptr)
{
	HWND desk_hwnd=::GetDesktopWindow();
	RECT target_rc;
	POINT lp = {0, 0};

	::GetClientRect(hwnd,&target_rc);
	ClientToScreen(hwnd, &lp);
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

	TRACE(T_INFO, "file size %d width %d height %d \n", out_ptr->bfh.bfSize,
		out_ptr->bih.biWidth, out_ptr->bih.biHeight);

	::GetDIBits(desk_dc, desk_bmp, 0, bmInfo.bmHeight, bm_data, (BITMAPINFO *)&out_ptr->bih, DIB_RGB_COLORS);

	out_ptr->len = bm_dataSize;
	out_ptr->data = bm_data;

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

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	int ret = 0;
	//auto_mob_init();
	struct t_bmp target = {};
	HWND hwnd = FindWindow(NULL, L"无标题 - 记事本");
	ret = get_screen(hwnd, L"D://1.bmp", &target);
	delete(target.data);
	return 0;
}
#endif /* __OWN_MAIN__ */
