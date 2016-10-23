#include <stdio.h>
#include <windows.h>
#include <time.h>

#include "debug.h"
#include "output_handle.h"

struct oskinfo osk_info;

/* Base on the sequence of ASCII */
static POINT key_pos_info[] = {
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {50, 102}, {636, 65}, {0, 0}, {0, 0}, {500, 147}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{237, 234}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{418, 65},	/* 0 */
	{90, 65},	/* 1 */
	{126, 65},	/* 2 */
	{164, 65},	/* 3 */
	{202, 65},	/* 4 */
	{238, 65},	/* 5 */
	{274, 65},	/* 6 */
	{310, 65},	/* 7 */
	{346, 65},	/* 8 */
	{382, 65},	/* 9 */
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
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
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},  /* 95 */
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {548, 102},	/* DEL: 127 */
	{468, 196}, {468, 232}, {440, 232}, {510, 232}	/* UP: 128, DOWN: 129, LEFT: 130, RIGHT: 131 */
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

int get_random(int n)
{
	srand((unsigned int)time(NULL));
	return (rand()%n);
}

static int get_key_pos(unsigned char key, int *x, int *y)
{
	/* Upper to lowwer */
	if (key >= 'a' && key <= 'z')
		key = key - 'a' + 'A';

	*x = key_pos_info[key].x;
	*y = key_pos_info[key].y;
	return 0;
}

static int osk_key_down(unsigned char key)
{
	int x, y;
	int ret = 0;
	LPARAM lp = 0;

	get_key_pos(key, &x, &y);
	lp = (LPARAM) (x + (y << 16));
	TRACE(T_INFO, "X: %d, Y: %d", x, y);
	ret = PostMessage(osk_info.hwnd, WM_LBUTTONDOWN , 0, lp);
	return ret;
}

static int osk_key_up(unsigned char key)
{
	int x, y;
	int ret = 0;
	LPARAM lp = 0;

	get_key_pos(key, &x, &y);
	lp = (LPARAM) (x + (y << 16));
	ret = PostMessage(osk_info.hwnd, WM_LBUTTONUP, 0, lp);
	return ret;
}

int osk_send_string(HWND target, const char *keys, int len)
{
	int i = 0;

	TRACE(T_INFO, "Begin send key(%s) to target(%x)", keys, target);
	for (i = 0; i < len; i++) {
		if (target) {
		/* We found that if we do not use SetCursorPos and mouse_event to set the Focus,
		 * need SetForegroundWindow() to finish the Focus.*/
			SetForegroundWindow(target);
		}
		osk_key_down(*(keys + i));
		osk_key_up(*(keys + i));
	}
	return 0;
}

/* If target == NULL, means uses other way to set focus on window */
int osk_send_char(HWND target, const unsigned char key)
{
	TRACE(T_INFO, "Begin send key(%d) to target(%x)", key, target);
	if (target) {
		SetForegroundWindow(target);
	}
	osk_key_down(key);
	Sleep(100 + get_random(100));
	osk_key_up(key);
	return 0;
}

/* x,y should be the abusolute position */
int move_and_click(int x, int y)
{
	SetCursorPos(x, y);
	mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, 0, 0);
	Sleep(100 + get_random(100));
	mouse_event(MOUSEEVENTF_LEFTUP, x, y, 0, 0);
	return 0;
}

int move_and_rclick(int x, int y)
{
	SetCursorPos(x, y);
	mouse_event(MOUSEEVENTF_RIGHTDOWN, x, y, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_RIGHTUP, x, y, 0, 0);
	return 0;
}

void osk_send_del(HWND target, int n)
{
	int i = 0;

	for (; i < n; i++) {
		osk_send_char(target, (char)127);
		Sleep(get_random(10 + get_random(10)));
	}
}

static int unit_test_send_info_to_notepad(void)
{
	HWND notepad_hwnd = NULL;

	if (osk_init() == -1)
		return -1;
	notepad_hwnd = FindWindow(NULL, L"无标题 - 记事本");
	osk_send_char(notepad_hwnd, 'a');
	return 0;
}

static int unit_test_simple_login_dnf(void)
{
	HWND dnf_hwnd = NULL;
	POINT lp;
	char *usr_id = "2648550849";
	char *passwd = "dnf1234dnf";

	if (osk_init() == -1)
		return -1;
	lp.x = 1112;
	lp.y = 350;
	dnf_hwnd = FindWindow(NULL, L"地下城与勇士登录程序");
	ClientToScreen(dnf_hwnd, &lp);
	SetCursorPos(lp.x, lp.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN, lp.x, lp.y, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, lp.x, lp.y, 0, 0);
	/* Remove orignal user info */
	Sleep(1000 + get_random(300));
	osk_send_del(NULL, 20 + get_random(10));
	Sleep(1000 + get_random(300));
	osk_send_string(NULL, usr_id, strlen(usr_id));
	/* type TAB (ASCII = 9) */
	Sleep(1000 + get_random(500));
	osk_send_char(NULL, (char)9);
	Sleep(1000 + get_random(500));
	osk_send_string(NULL, passwd, strlen(passwd));
	/* type Enter (ASCII = 13) */
	Sleep(1000 + get_random(500));
	osk_send_char(NULL, (char)13);
	return 0;
}

#ifdef __OWN_MAIN__
int main(int argc, char *argv)
{
	return unit_test_simple_login_dnf();
}
#endif
