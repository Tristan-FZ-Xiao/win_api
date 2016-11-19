#include <stdio.h>
#include<windows.h>
#include <wchar.h>
#include <math.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"
#include "recognise.h"

enum {
	WALK_STATE_START = 100,
	WALK_STATE_NORMAL,
	WALK_STATE_STUN,
	WALK_STATE_STOP,
};

int start_walk(HWND hwnd, unsigned int direct)
{
	SetForegroundWindow(auto_mob.mob_hwnd);
	return osk_key_down((unsigned char)direct);
}

int stop_walk(HWND hwnd, unsigned int direct)
{
	return osk_key_up((unsigned char)direct);
}

void walk_step(HWND hwnd, int direct)
{
	start_walk(hwnd, direct);
	Sleep(200);
	stop_walk(hwnd, direct);
}

int get_current_time_ms(void)
{
	SYSTEMTIME sys;

	GetLocalTime(&sys);
	return (sys.wSecond * 1000 + sys.wMilliseconds); 
}

#define DIFF	15
#define NEAR_POINT(a, b)	(abs(a.x -b.x) < DIFF ? (abs(a.y - b.y) < DIFF ? 1 : 0) : 0)
#define NEAR_X(x1, x2)	(abs(x1 - x2) < DIFF ? 1 : 0)
#define NEAR_Y(y1, y2)	(abs(y1 - y2) < DIFF ? 1 : 0)

int walk_to(POINT p, int timeout)
{
	struct t_bmp input = {};
	int ret = 0;
	int cur_t = get_current_time_ms();

	while (1) {
		ret = get_screen(auto_mob.mob_hwnd, NULL, &input);
		if (ret != ERR_NO_ERR) {
			return 0;
		}
		ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
		ret = convert2blackwhite(&input, ONLY_BLACK, 0);
		ret = get_normal_info(&input, &map_info, true);
		delete input.data;
		TRACE(T_INFO, "Role info (%d\t%d), target(%d\t%d)", map_info.role.x, map_info.role.y,
			p.x, p.y);
		if ((cur_t - get_current_time_ms()) > timeout) {
			return ERR_COMMON_TIMEOUT;
		}

		if (NEAR_POINT(map_info.role, p)) {
			return ERR_NO_ERR;
		}

		if (!NEAR_X(map_info.role.x, p.x) && (map_info.role.x > p.x)) {
			TRACE(T_INFO, "WALK LEFT");
			walk_step(auto_mob.mob_hwnd, T_LEFT);
		}
		else if (!NEAR_X(map_info.role.x, p.x) && (map_info.role.x < p.x)) {
			TRACE(T_INFO, "WALK RIGHT");
			walk_step(auto_mob.mob_hwnd, T_RIGHT);
		}
		else if (!NEAR_Y(map_info.role.y, p.y) && (map_info.role.y > p.y)) {
			TRACE(T_INFO, "WALK UP");
			walk_step(auto_mob.mob_hwnd, T_UP);
		}
		else if (!NEAR_Y(map_info.role.y, p.y) && (map_info.role.y < p.y)) {
			TRACE(T_INFO, "WALK DOWN");
			walk_step(auto_mob.mob_hwnd, T_DOWN);
		}
	}
}

int unit_test_walk_to(void)
{
	int ret = 0;
	POINT target = {400, 400};

	ret = auto_mob_init();
	auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
	if (!auto_mob.mob_hwnd) {
		return ERR_HWND_NOT_FOUND;
	}

	ret = walk_to(target, 5000);
	TRACE(T_INFO, "Now go to POINT(%d\t%d) is %s\n", target.x, target.y, 
		ret == ERR_NO_ERR ? "finished" : "not finished");
	return 0;
}

struct walk_info {
	RECT map;
	POINT start;
	POINT end;
	POINT prev;
	POINT current;
	int state;
	int gate_edge;
};

int walk_fn(struct map_status *map_info, struct walk_info *walk)
{
	switch (walk->state) {
	case WALK_STATE_NORMAL:
		break;
	case WALK_STATE_STUN:
		break;
	default:
		TRACE(T_ERROR, "walk_state(%d) error\n", walk->state);
	}
	return ERR_NO_ERR;
}

//#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	unit_test_walk_to();
	return 0;
}
#endif /* __OWN_MAIN__ */
