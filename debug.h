#ifndef __DEBUG_H__
#define __DEBUG_H__

#define T_DEBUG 1

#define COMMON_LEN 256

enum {
	ERR_NO_ERR = 0,
	ERR_COMMON_NULL_ERROR,
	ERR_COMMON_PARAM_ERR,

	ERR_HWND_NOT_FOUND = -10000,
	ERR_OSK_NOT_FOUND,
	ERR_FILE_NOT_FOUND,
	ERR_CHOOSE_REGION_FAILED,
	ERR_GET_REGION_FAILED,
	ERR_GET_SERVER_FAILED,
	ERR_GET_LOGIN_BUTTON_FAILED,
	ERR_INPUT_ACCOUNT_FAILED,
	ERR_CHOOSE_ROLE_FAILED,
	ERR_WINDOW_NOT_FOUND,
	ERR_GET_FILE_SIZE_FAILED,
	ERR_READ_FILE_FAILED,
};

void bt_inc_msglevel(void);
void bt_set_msglevel(int level);
unsigned int bt_get_msglevel(void);

enum MSGLEVEL {
	MSG_ERROR = 0x0001,
	MSG_INFO  = 0x0002,
	MSG_DUMP  = 0x0004
};

#define T_ERROR		MSG_ERROR, "ERROR", __FUNCTION__, __LINE__
#define T_INFO		MSG_INFO, "INFO",  __FUNCTION__, __LINE__
#define T_DUMP		MSG_DUMP, "DUMP", __FUNCTION__, __LINE__

#ifdef T_DEBUG

#define BTTRACELEVEL	(MSG_ERROR | MSG_INFO | MSG_DUMP)

#define TRACE	print_msg

void print_msg(int level, const char *level_str, const char *func, int line, char *format, ...);

#else

#define TRACE(VARGLST) ((void)0)

#endif

#endif /* __DEBUG_H__ */
