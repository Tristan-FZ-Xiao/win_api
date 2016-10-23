#include <stdio.h>
#include <windows.h>

#include "output_handle.h"
#include "login.h"
#include "init.h"

static POINT isp_pos_info[] = {
	{328, 175},	/* Telecom */
	{328, 250},	/* Unicom */
	{328, 325}	/* Other */
};

static POINT choose_region_pos[] = {
	{500, 570},
	{1150, 300}
};

static POINT login_button_pos[] = {
	{1150, 580}
};

static POINT login_account_pos[] = {
	{1112, 350}
};

static POINT role_pos[] = {
	{125, 190},
	{300, 450}
};

static POINT region_pos;
static POINT server_pos;

static POINT *get_isp_pos(int index)
{
	TRACE(T_INFO, "index %d, isp_pos_info %d, POINT %d", index, sizeof(isp_pos_info), sizeof(POINT));

	if (index > (sizeof(isp_pos_info) / sizeof(POINT))) {
		TRACE(T_ERROR, "index overflow");
		return NULL;
	}
	else {
		TRACE(T_INFO, "X: %d, Y: %d", isp_pos_info[index].x, isp_pos_info[index].y);
		return &isp_pos_info[index];
	}
}

static POINT *get_region_pos(int type_name, int index)
{
	if (index > 20) {
		return NULL;
	}

	region_pos.x = REGION_BASE_X;
	region_pos.y = REGION_BASE_Y;
	
	if (index) {
		region_pos.x += REGION_OFFSET_X * (index % NUM_EACH_ROW);
		region_pos.y += REGION_OFFSET_Y * (index / NUM_EACH_ROW);
	}
	return &region_pos;
}
	
static POINT *get_server_pos(int index)
{
	if (index > 15) {
		return NULL;
	}

	server_pos.x = SERVER_BASE_X;
	server_pos.y = SERVER_BASE_Y;
	
	if (index) {
		server_pos.x += SERVER_OFFSET_X * (index % NUM_EACH_ROW);
		server_pos.y += SERVER_OFFSET_Y * (index / NUM_EACH_ROW);
	}
	return &server_pos;
}

static POINT *get_choose_region_pos(int index)
{
	if (index > 2) {
		return NULL;
	}
	return &choose_region_pos[index];
}

static POINT *get_login_button_pos(int index)
{
	if (index > 1) {
		return NULL;
	}
	return &login_button_pos[index];
}

static POINT *get_login_account_pos(int index)
{
	if (index > 1) {
		return NULL;
	}
	return &login_account_pos[index];
}

static POINT *get_role_pos(int index)
{
	if (index > 1) {
		return NULL;
	}
	return &role_pos[index];
}

static int choose_region(HWND hwnd, int isp_type, int region, int server)
{
	POINT *lp = NULL;
	/* each some differ between each time on the position */
	int x_r = -10 + get_random(20);
	int y_r = -5 + get_random(10);

	TRACE(T_INFO, "Choose region: ISP_Type: %d, Region %d, Server: %d", 
		isp_type, region, server);

	/* click choose region button */
	lp = get_choose_region_pos(0);
	if (lp == NULL) {
		TRACE(T_ERROR, "Get choose region error");
		return ERR_CHOOSE_REGION_FAILED;
	}
	TRACE(T_INFO, "choose-region pos:  X: %d, Y: %d", lp->x, lp->y);
	ClientToScreen(hwnd, lp);
	move_and_click(lp->x + x_r, lp->y + y_r);
	Sleep(500 + get_random(500));

	TRACE(T_INFO, "choose-region pos:  X: %d, Y: %d", lp->x, lp->y);
	/* choose the region and server */
	lp = get_region_pos(isp_type, region);
	if (lp == NULL) {
		TRACE(T_ERROR, "Get region info error");
		return ERR_GET_REGION_FAILED;
	}
	ClientToScreen(hwnd, lp);
	move_and_click(lp->x + x_r, lp->y + y_r);

	Sleep(500 + get_random(500));
	lp = get_server_pos(server);
	if (lp == NULL) {
		TRACE(T_ERROR, "Get server info error");
		return ERR_GET_SERVER_FAILED;
	}
	ClientToScreen(hwnd, lp);
	move_and_click(lp->x + x_r, lp->y + y_r);
	Sleep(500 + get_random(500));

	/* click LOGIN button */
	lp = get_login_button_pos(0);
	if (lp == NULL) {
		TRACE(T_ERROR, "Get login button info error");
		return ERR_GET_LOGIN_BUTTON_FAILED;
	}
	ClientToScreen(hwnd, lp);
	move_and_click(lp->x + x_r, lp->y + y_r);
	Sleep(5000 + get_random(500));
	return 0;
}

static int input_account(HWND hwnd, const char *user_name, const char *password)
{
	int x_r = -10 + get_random(20);
	int y_r = -5 + get_random(10);
	POINT *lp = get_login_account_pos(0);

	TRACE(T_INFO, "The user name %s, password %s", user_name, password);
	ClientToScreen(hwnd, lp);
	move_and_click(lp->x, lp->y);
	Sleep(3000 + get_random(300));
	/* Remove orignal user info */
	osk_send_del(NULL, 20 + get_random(10));
	Sleep(1000 + get_random(300));
	osk_send_string(NULL, user_name, strlen(user_name));
	/* type TAB (ASCII = 9) */
	Sleep(1000 + get_random(500));
	osk_send_char(NULL, (char)9);
	Sleep(1000 + get_random(500));
	osk_send_string(NULL, password, strlen(password));
	/* type Enter (ASCII = 13) */
	Sleep(1000 + get_random(500));
	osk_send_char(NULL, (char)13);
	return ERR_NO_ERR;
}

int choose_role(void)
{
	int x_r = -20 + get_random(40);
	int y_r = -40 + get_random(80);
	POINT *lp = get_role_pos(0);

	auto_mob.mob_hwnd = FindWindow(NULL, auto_mob.mob_name);
	if (auto_mob.mob_hwnd == NULL) {
		TRACE(T_ERROR, "Could not find the auto-mob, please open it");
		return ERR_HWND_NOT_FOUND;
	}
	ClientToScreen(auto_mob.mob_hwnd, lp);
	move_and_click(lp->x + x_r, lp->y + y_r);
	/* type Space (ASCII = 32) */
	Sleep(500 + get_random(500));
	osk_send_char(NULL, (char)T_SPACE);
	return ERR_NO_ERR;
}

int do_login(HWND hwnd, int isp_type, int region, int server, const char *user_name,
		const char *password)
{
	int ret = 0;

	ret = choose_region(hwnd, isp_type, region, server);
	if (ret) {
		return ERR_CHOOSE_REGION_FAILED;
	}
	ret = input_account(hwnd, user_name, password);
	if (ret) {
		return ERR_INPUT_ACCOUNT_FAILED;
	}
	Sleep(120000);
	ret = choose_role();
	if (ret) {
		return ERR_CHOOSE_ROLE_FAILED;
	}

	return ERR_NO_ERR;
}

static void unit_test_isp(void)
{
	HWND hwnd = NULL;
	POINT *lp = NULL;
	int i = 0;

	if (osk_init() == -1)
		return;

	hwnd = auto_mob.login_hwnd;
	for (; i < 20; i++) {
		lp = get_isp_pos(i);
		if (lp == NULL) {
			return;
		}
		ClientToScreen(hwnd, lp);
		SetCursorPos(lp->x, lp->y);
		Sleep(300);
	}
}

static void unit_test_region(void)
{
	HWND hwnd = NULL;
	POINT *lp = NULL;
	int i = 0;

	if (osk_init() == -1)
		return;

	hwnd = auto_mob.login_hwnd;
	for (; i < 20; i++) {
		lp = get_region_pos(ISP_NAME_TELECOM, i);
		if (lp == NULL) {
			return;
		}
		ClientToScreen(hwnd, lp);
		SetCursorPos(lp->x, lp->y);
		Sleep(300);
	}
}

static void unit_test_server(void)
{
	HWND hwnd = NULL;
	POINT *lp = NULL;
	int i = 0;

	if (osk_init() == -1)
		return;

	hwnd = auto_mob.login_hwnd;
	for (; i < 20; i++) {
		lp = get_server_pos(i);
		if (lp == NULL) {
			return;
		}
		ClientToScreen(hwnd, lp);
		SetCursorPos(lp->x, lp->y);
		Sleep(300);
	}
}

static void unit_test_choose_region(void)
{
	HWND hwnd = NULL;
	POINT *lp = NULL;
	int x_r = -10 + get_random(20);
	int y_r = -5 + get_random(10);
	int i = 0;

	if (osk_init() == -1)
		return;

	hwnd = auto_mob.login_hwnd;
	lp = get_choose_region_pos(0);
	if (lp == NULL) {
		return;
	}
	ClientToScreen(hwnd, lp);
	SetCursorPos(lp->x + x_r, lp->y + y_r);
	Sleep(300);
}

//#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__
int main(int argc, char *argv)
{
	int ret = 0;
#if 1
	ret = auto_mob_init();
	if (ret != ERR_NO_ERR) {
		TRACE(T_ERROR, "Init auto_mob failed");
		return -1;
	}
	ret = do_login(auto_mob.login_hwnd, ISP_NAME_TELECOM, 0, 7, "3509265068", "dnf1234dnf");
	if (ret != ERR_NO_ERR)
		return ret;
#endif
#if 0
	unit_test_isp();
	unit_test_region();
	unit_test_server();
	unit_test_choose_region();
#endif
}
#endif
