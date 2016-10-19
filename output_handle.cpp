#include <stdio.h>
#include <windows.h>

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
	osk_info.hwnd = FindWindow(NULL, L"ÆÁÄ»¼üÅÌ");
	osk_info.hwnd = FindWindowEx(osk_info.hwnd, NULL, NULL, NULL);
	if (osk_info.hwnd == NULL)
		return -1;
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

int main(int argc, char *argv)
{
	int ret = 0;
	LPARAM lp = 0;
	POINT point;
	
	ret = osk_init();
	if (ret == -1)
		return -1;

	osk_key_down('z');
	osk_key_up('z');
	return ret;
}
