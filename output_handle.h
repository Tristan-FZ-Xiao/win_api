#ifndef __OUTPUT_HANDLE_H__
#define __OUTPUT_HANDLE_H__

struct oskinfo {
	HWND hwnd;
};

extern struct oskinfo osk_info;
extern int osk_init(void);
extern int osk_to_target(HWND target, char key);

#endif /* __OUTPUT_HANDLE_H__ */