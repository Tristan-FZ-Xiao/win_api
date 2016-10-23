#include <windows.h>

#include "init.h"
#include "output_handle.h"

struct auto_mob_info auto_mob = {NULL, NULL, L"���³�����ʿ��¼����", L"���³�����ʿ"};

int auto_mob_init(void)
{
	if (osk_init() == -1) {
		TRACE(T_ERROR, "Could not find the OSK(on-screen-keyboard), please open it");
		return ERR_OSK_NOT_FOUND;
	}

	auto_mob.login_hwnd = FindWindow(NULL, auto_mob.login_name);
	if (auto_mob.login_hwnd == NULL) {
		TRACE(T_ERROR, "Could not find the auto-mob, please open it");
		return ERR_HWND_NOT_FOUND;
	}

	return ERR_NO_ERR;
}
