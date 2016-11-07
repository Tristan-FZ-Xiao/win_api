#include <stdio.h>
#include<windows.h>
#include <wchar.h>
#include <math.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"
#include "recognise.h"

int task_init(void)
{
	int ret = 0;

	ret = auto_mob_init();
	auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
	return ret;
}

int get_role_info(void)
{
	struct t_bmp input = {};
	int ret = 0;

	/* send 'm' */
	osk_send_char(NULL, (char)77);
	ret = get_screen(auto_mob.mob_hwnd, NULL, &input);
	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 0);
	ret = get_role_hp_mp_level(&input, &role_info);
	osk_send_char(NULL, (char)77);
	return ret;
}

int get_task_map_info(void)
{
	struct t_bmp input = {};
	int ret = 0;

	ret = get_screen(auto_mob.mob_hwnd, NULL, &input);
	ret = get_normal_info(&input, &map_info, false);
	delete input.data;

	ret = get_screen(auto_mob.mob_hwnd, NULL, &input);
	ret = get_littlemap_info(&input, &map_info, false);
	delete input.data;
	return ret;
}

int collect_info(void)
{
	int ret = 0;
	int flag = 0;
	struct t_bmp input = {};

	ret = get_role_info();
	ret = get_task_map_info();
	return ret;
}

int dealwith_task_item(void)
{
	while (1) {
		collect_info();
		map_info_output(&map_info);
		Sleep(5000);
	}
	return ERR_NO_ERR;
}

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	task_init();
	dealwith_task_item();
}
#endif