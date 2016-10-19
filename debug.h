#ifndef __DEBUG_H__
#define __DEBUG_H__

#define T_DEBUG 1

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
