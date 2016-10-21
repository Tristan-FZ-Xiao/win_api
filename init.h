#ifndef __INIT_H__
#define __INIT_H__

#include "debug.h"
struct auto_mob_info {
	HWND hwnd;
	wchar_t mob_name[COMMON_LEN];
};

extern struct auto_mob_info auto_mob; 
extern int auto_mob_init(void);

#endif __INIT_H__