#include <windows.h>

#include "init.h"
#include "output_handle.h"

struct auto_mob_info auto_mob = {NULL, L"地下城与勇士登录程序"};

int auto_mob_init(void)
{
	if (osk_init() == -1) {
		TRACE(T_ERROR, "Could not find the OSK(on-screen-keyboard), please open it");
		return ERR_OSK_NOT_FOUND;
	}
	auto_mob.hwnd = FindWindow(NULL, auto_mob.mob_name);
	if (auto_mob.hwnd == NULL) {
		TRACE(T_ERROR, "Could not find the auto-mob, please open it");
		return ERR_HWND_NOT_FOUND;
	}
	return ERR_NO_ERR;
}
