#ifndef __OUTPUT_HANDLE_H__
#define __OUTPUT_HANDLE_H__

struct oskinfo {
	HWND hwnd;
};

#define T_SPACE		32
#define T_UP		128
#define T_DOWN		129
#define T_LEFT		130
#define T_RIGHT		131

extern struct oskinfo osk_info;
extern int osk_init(void);
extern int get_random(int n);
extern int move_and_click(int x, int y);
extern int move_and_rclick(int x, int y);
extern void osk_send_del(HWND target, int n);
extern int osk_send_string(HWND target, const char *keys, int len);
extern int osk_send_char(HWND target, const unsigned char key);
extern int osk_key_down(unsigned char key);
extern int osk_key_up(unsigned char key);

#endif /* __OUTPUT_HANDLE_H__ */