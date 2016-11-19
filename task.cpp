#include <stdio.h>
#include<windows.h>
#include <wchar.h>
#include <math.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"
#include "timer.h"
#include "task.h"

#define ROLE_EXPIRE_TIME	2000		/* 2000 ms */
#define MAP_EXPIRE_TIME		100		/* 100 ms */

struct task_status task_info;

int task_init(void)
{
	int ret = ERR_NO_ERR;

	do {
		auto_mob_init();
		auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
		if (auto_mob.mob_hwnd == NULL) {
			ret = ERR_COMMON_NULL_ERROR;
			break;
		}
		ret = timer_init();
		return ret;
	} while (0);

	return ret;
}

static int __get_role_info(struct role_status *role_info)
{
	struct t_bmp input = {};
	int ret = 0;
	int count = 0;

	/* send 'm' */
LOOP:
	count ++;
	osk_send_char(auto_mob.mob_hwnd, (char)77);
	ret = get_screen(auto_mob.mob_hwnd, NULL, &input);
	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 0);
	role_info->hp.last_hp = role_info->hp.cur_hp;
	role_info->mp.last_mp = role_info->mp.cur_mp;
	ret = get_role_hp_mp_level(&input, role_info);
	if (ret == ERR_GET_ROLE_INFO_FAILED) {
		if (count <= 5) {
			goto LOOP;
		}
		else
			return ret;
	}
	osk_send_char(auto_mob.mob_hwnd, (char)77);
	return ret;
}

static void get_role_info_cb(void *data)
{
	struct role_status *role_info = (struct role_status *)data;

	if (ERR_NO_ERR != __get_role_info(role_info)) {
		TRACE(T_ERROR, "ERROR to get role info\n");
	}
}

void get_role_info_init(void)
{
	task_info.role.role_timer.cb = get_role_info_cb;
	task_info.role.role_timer.data = &task_info.role;
	task_info.role.role_timer.expired = ROLE_EXPIRE_TIME;
	task_info.role.role_timer.init_expired = ROLE_EXPIRE_TIME;
	timer_add(&task_info.role.role_timer);
}

static int __get_map_info(struct map_status *map_info)
{
	struct t_bmp input = {};
	int ret = 0;

	ret = get_screen(auto_mob.mob_hwnd, NULL, &input);
	ret = get_normal_info(&input, map_info, false);
	delete input.data;
	return ret;
}

static void get_map_info_cb(void *data)
{
	struct map_status *map_info = (struct map_status *)data;

	if (ERR_NO_ERR != __get_map_info(map_info)) {
		TRACE(T_ERROR, "ERROR to get map info\n");
	}
	map_info_output(map_info);
}

void get_map_info_init(void)
{
	task_info.map.map_timer.cb = get_map_info_cb;
	task_info.map.map_timer.data = &task_info.map;
	task_info.map.map_timer.expired = MAP_EXPIRE_TIME;
	task_info.map.map_timer.init_expired = MAP_EXPIRE_TIME;
	timer_add(&task_info.map.map_timer);
}


void unit_test_task_item(void)
{
	get_role_info_init();
	get_map_info_init();
	timer_run();
}

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	int ret = ERR_NO_ERR;

	ret = task_init();
	if (ret != ERR_NO_ERR) {
		TRACE(T_ERROR, "task init failed error(%d)\n", ret);
		return ret;
	}
	unit_test_task_item();
	return ret;
}
#endif