#ifndef __RECOGNISE_H__
#define __RECOGNISE_H__

#include "timer.h"

struct hp_info {
	int cur_hp;
	int hp;
	int last_hp;
};

struct mp_info {
	int cur_mp;
	int mp;
	int last_mp;
};

struct role_status {
	struct hp_info hp;
	struct mp_info mp;
	int gold;
	int level;
	struct t_timer role_timer;
};

struct map_status {
	POINT	bmap_next_target;	/* The position in the big map(N) which after task, need to go to do something. */
	POINT	bmap_role;		/* Role position in the big map(N) */
	POINT	role;			/* Role position in Normal picture */
	RECT	little_map;
	POINT	lmap_role;		/* Role position in the little map(right-top) */
	POINT	lmap_next_target;	/* The position in the littele map which need to go in the task. */
	POINT	lmap_boss;		/* The position where is boss. */
	POINT	*gold;			/* golden position */
	POINT	*goods;			/* goods position */
	struct t_timer map_timer;
};

enum {
	WHITE_KEY_VALUE = 0xff,
	BLACK_KEY_VALUE = 0x0,
};

enum {
	FIND_FIRST = 1,
	FIND_ALL,
};

enum {
	NUM_0 = 0,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	NUM_7,
	NUM_8,
	NUM_9,
	NUM_M,	/* / */
};

#define GOODS_NUM	20

extern struct role_status role_info;
extern struct map_status map_info;

extern int get_role_hp_mp_level(struct t_bmp *input, struct role_status *role);
extern int get_role_gold(struct t_bmp *input, struct role_status *role);
/*
 *	input:		should be binary-handle(only black and white).
 *	charactor:	the charactor of things that you want to recognise.
 *	mode:		support 1. find first one, 2. find all.
 */
extern int get_target_position(struct t_bmp *input, POINT *charactor_black,
			int black_len, POINT *charactor_white, int white_len,
			int mode, POINT *target, int *target_len, int target_center);
extern void map_info_output(struct map_status *map_info);
extern int get_bigmap_info(struct t_bmp *input, struct map_status *map_info, int is_convert);
extern int get_littlemap_info(struct t_bmp *input, struct map_status *map_info, int is_convert);
extern int get_normal_info(struct t_bmp *input, struct map_status *map_info, int is_convert);

#endif /* __RECOGNISE_H__ */