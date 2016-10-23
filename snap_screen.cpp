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
int sumSize;
char* get_screen(HWND hwnd, const wchar_t *path){
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
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	GetObject(desk_bmp, sizeof(BITMAP), &bmInfo);
	bm_dataSize = bmInfo.bmWidthBytes * bmInfo.bmHeight;
	bm_data = new char[bm_dataSize];

	bfh.bfType			= 0x4d42;
	bfh.bfSize			= bm_dataSize + 54;
	bfh.bfReserved1		= 0;
	bfh.bfReserved2		= 0;
	bfh.bfOffBits		= 54;

	bih.biSize			= 40;
	bih.biWidth			= bmInfo.bmWidth;
	bih.biHeight			= bmInfo.bmHeight;
	bih.biPlanes			= 1;
	bih.biBitCount		= 24;
	bih.biCompression		= BI_RGB;
	bih.biSizeImage		= bm_dataSize;
	bih.biXPelsPerMeter	= 0;
	bih.biYPelsPerMeter	= 0;
	bih.biClrUsed		= 0;
	bih.biClrImportant		= 0;

	::GetDIBits(desk_dc, desk_bmp, 0, bmInfo.bmHeight, bm_data, (BITMAPINFO *)&bih, DIB_RGB_COLORS);

	sumSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bm_dataSize;
	char * stream = new char[sumSize];
	memcpy(stream, &bfh, sizeof(BITMAPFILEHEADER));
	memcpy(stream + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));
	memcpy(stream + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), bm_data, bm_dataSize);
	delete(bm_data);

	if (path) {
		HANDLE hFile=CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, 0);
		DWORD dwSize;
		WriteFile(hFile, (void *)stream, sumSize, &dwSize, 0);
		::CloseHandle(hFile);
	}
	return stream;
}

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	auto_mob_init();
	char *ptr = get_screen(auto_mob.login_hwnd, L"D://1.bmp");
	delete(ptr);
	return 0;
}
#endif /* __OWN_MAIN__ */
