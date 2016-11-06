#ifndef __RECOGNISE_H__
#define __RECOGNISE_H__

struct hp_info {
	int cur_hp;
int hp;
};

struct mp_info {
	int cur_mp;
	int mp;
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


#endif /* __RECOGNISE_H__ */