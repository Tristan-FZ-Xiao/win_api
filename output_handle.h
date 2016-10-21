#ifndef __OUTPUT_HANDLE_H__
#define __OUTPUT_HANDLE_H__

struct oskinfo {
	HWND hwnd;
};

extern struct oskinfo osk_info;
extern int osk_init(void);
extern int osk_to_target(HWND target, char key);
extern int get_random(int n);
extern int move_and_click(int x, int y);
extern void osk_send_del(HWND target, int n);
extern int osk_send_string(HWND target, const char *keys, int len);
extern int osk_send_char(HWND target, const char key);

#endif /* __OUTPUT_HANDLE_H__ */