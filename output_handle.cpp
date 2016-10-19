#include <stdio.h>
#include <windows.h>
#include "debug.h"

struct oskinfo {
	HWND hwnd;
} osk_info;

POINT key_pos_info[] = {
	{87, 152},	/* a */
	{251, 196},	/* b */
	{185, 191},	/* ... */
	{161, 151},
	{143, 102},
	{196, 145},
	{231, 151},
	{271, 151}, /* h */
	{327, 104},
	{309, 148},
	{343, 151},
	{383, 151},
	{325, 190},
	{284, 190},
	{362, 102},	/* o */
	{296, 102},
	{72,  102},
	{182, 102},
	{122, 149},
	{213, 102},	/* t */
	{285, 102},
	{218, 189},
	{111, 102},	/* w */
	{144, 193},
	{253, 102},
	{107, 192},	/* z */
};

int osk_init(void)
{
	osk_info.hwnd = FindWindow(NULL, L"屏幕键盘");
	if (osk_info.hwnd != NULL) {
		osk_info.hwnd = FindWindowEx(osk_info.hwnd, NULL, NULL, NULL);
	}

	if (osk_info.hwnd == NULL) {
		TRACE(T_ERROR, "OSK is not found\n");
		return -1;
	}
	else
		return 0;
}

int get_key_pos(char key, int *x, int *y)
{
	if (key < 'A' || key > 'z')
		return -1;
	if (key >= 'A' && key <= 'Z')
		key = key - 'A' + 'a';
	*x = key_pos_info[key - 'a'].x;
	*y = key_pos_info[key - 'a'].y;
	return 0;
}

int osk_key_down(char key)
{
	int x, y;
	int ret = 0;
	LPARAM lp = 0;

	get_key_pos(key, &x, &y);
	lp = (LPARAM) (x + (y << 16));
	ret = PostMessage(osk_info.hwnd, WM_LBUTTONDOWN , 0, lp);
	return ret;
}

int osk_key_up(char key)
{
	int x, y;
	int ret = 0;
	LPARAM lp = 0;

	get_key_pos(key, &x, &y);
	lp = (LPARAM) (x + (y << 16));
	ret = PostMessage(osk_info.hwnd, WM_LBUTTONUP, 0, lp);
	return ret;
}

int osk_to_target(HWND target, char key)
{
	TRACE(T_INFO, "Begin send key(%c) to target(%x)", key, target);
	if (target) {
		SetForegroundWindow(target);
		osk_key_down(key);
		osk_key_up(key);
		return 0;
	}
	else {
		TRACE(T_ERROR, "Could not find target(HWND)");
		return -1;
	}
}

int unit_test_send_info_to_notepad(void)
{
	HWND notepad_hwnd = NULL;

	if (osk_init() == -1)
		return -1;
	notepad_hwnd = FindWindow(NULL, L"无标题 - 记事本");
	osk_to_target(notepad_hwnd, 'a');
	return 0;
}

int main(int argc, char *argv)
{
	return unit_test_send_info_to_notepad();
}
