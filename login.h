#ifndef __LOGIN_H__
#define __LOGIN_H__
#include "debug.h"

#define REGION_BASE_X	420
#define REGION_BASE_Y	175
#define REGION_OFFSET_X	121
#define REGION_OFFSET_Y	50

#define SERVER_BASE_X	420
#define SERVER_BASE_Y	420
#define SERVER_OFFSET_X	121
#define SERVER_OFFSET_Y	50
#define NUM_EACH_ROW	5

enum {
	ISP_NAME_TELECOM = 100,
	ISP_NAME_UNICOM
};

struct account_info {
	char user_name[COMMON_LEN];
	char password[COMMON_LEN];
	int isp;
	int region;
	int server;
};

extern int do_login(HWND hwnd, int isp_type, int region, int server, const char *user_name,
		const char *password);
#endif /*__LOGIN_H__*/
