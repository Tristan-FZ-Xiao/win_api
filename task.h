#ifndef __TASK_H__
#define __TASK_H__

#include "recognise.h"
#include "timer.h"

struct task_status {
	struct role_status role;
	struct map_status map;
	struct t_timer call_guard_timer;
	struct t_timer get_things_timer;
	int call_guard_first;
};

#endif	/* __TASK_H__ */