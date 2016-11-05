#include <stdio.h>
#include<windows.h>
#include <wchar.h>
#include <math.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"
#include "recognise.h"

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

struct role_status {
	struct hp_info hp;
	struct mp_info mp;
	int gold;
	int level;
} role_info;
/*	What we need in the module: do some basic recognition likes:
 *	1. Number;
 *	2. map info;
 */
/*	The charactor of 0 - 9
 */
int num_charactor[11] = {
	62226,	/* 0 */
	281,	/* 1 */
	43333,	/* 2 */
	22335,	/* 3 */
	22482,	/* 4 */
	63334,	/* 5 */
	53334,	/* 6 */
	1532,	/* 7 */
	53335,	/* 8 */
	43335,	/* 9 */
	2222	/* / */
};

/* The charactors of the graph that we need to go for current task. */
POINT task_black_char[8] = {
	{4,	0},
	{6,	0},
	{0,	7},
	{10,	7},
	{0,	10},
	{10,	10},
	{3,	13},
	{7,	13}
};

POINT task_white_char[8] = {
	{3,	0},
	{7,	0},
	{-1,	7},
	{11,	7},
	{-1,	10},
	{11,	10},
	{2,	13},
	{8,	13}
};

/* The charactors of that role graph. */
POINT role_black_char[10] = {
	{5,	0},
	{6,	0},
	{3,	3},
	{8,	3},
	{2,	5},
	{9,	5},
	{0,	9},
	{11,	9},
	{0,	11},
	{11,	11}
};

POINT role_white_char[10] = {
	{4,	0},
	{7,	0},
	{2,	3},
	{9,	3},
	{1,	5},
	{10,	5},
	{-1,	9},
	{12,	9},
	{-1,	11},
	{12,	11}
};

/* The charactors of little map graph. */
POINT task_map_black_char[9] = {
	{1,	0},
	{1,	1},
	{5,	0},
	{10,	0},
	{15,	0},
	{0,	1},
	{0,	5},
	{0,	10},
	{0,	15}
};

POINT task_map_white_char[9] = {
	{0,	0},
	{2,	1},
	{7,	1},
	{12,	1},
	{17,	1},
	{1,	2},
	{1,	7},
	{1,	12},
	{1,	17}
};

/* The charactors of the role graph which in little map. */
POINT map_role_black_char[7] = {
	{2,	2},
	{4,	2},
	{5,	3},
	{1,	4},
	{0,	6},
	{5,	7},
	{2,	10}
};

POINT map_role_white_char[7] = {
	{0,	0},
	{2,	3},
	{3,	2},
	{4,	3},
	{2,	9},
	{3,	9},
	{3,	10}
};

/* The charactors of the next step graph which in little map. */
POINT map_next_black_char[] = {
	{0,	0},
	{3,	2},
	{3,	3},
	{3,	6},
	{4,	7},
	{1,	8},
	{2,	9},
	{3,	10}
};

POINT map_next_white_char[] = {
	{3,	0},
	{3,	1},
	{3,	4},
	{3,	5},
	{0,	8},
	{1,	9},
	{2,	10},
	{3,	11},
	{4,	10},
	{4,	7}
};

/* The charactors of the boss graph which in little map. */
POINT map_boss_black_char[] = {
	{0,	0},
	{5,	0},
	{4,	1},
	{3,	2},
	{8,	0},
	{9,	1},
	{10,	2},
	{5,	4},
	{8,	4},
	{1,	8},
	{12,	8}
};

POINT map_boss_white_char[] = {
	{6,	0},
	{7,	0},
	{5,	1},
	{4,	2},
	{3,	3},
	{8,	1},
	{9,	2},
	{10,	3},
	{1,	6},
	{12,	6}
};

int get_num_digit(int num)
{
	int i;

	for (i = 0; i <= 10; i ++) {
		num /= 10;
		if (num == 0) {
			return i;
		}
	}
	return -1;
}

void print_num(int *ptr, int len)
{
	int i;

	for (i = 0; i < len; i ++) {
		printf("%d ", *(ptr + i));
	}
	printf("\n");
}

void __calc_image_charactor(struct t_bmp *input, int *output)
{
	int w = input->bih.biWidth;
	int h = input->bih.biHeight;
	int i, j;

	for (i = 0; i < h; i ++) {
		for (j = 0; j < w; j ++) {
			*(output + j) += 
				*(input->data + (i * w + j) * 4) == 0xff ? 1 : 0;
		}
	}
}

int get_num(int num, int new_bit)
{
	return num * 10 + new_bit;
}

int get_num_charactor(int num)
{
	int i = 0;

	for (; i < sizeof(num_charactor); i ++) {
		if (num_charactor[i] == num) {
			return i;
		}
	}
	return -1;
}

char *__recognise_image_num(char *ptr, int *len, int *out_num)
{
	int i;
	int num = 0;
	int target = 0;

	for (i = 0; i < 15; i ++) {
		if (0 == *(ptr + i)) {
			break;
		}
		num = get_num(num, *(ptr + i));
		target = get_num_charactor(num);
		if (-1 == target) {
			continue;
		}
		else {
			*out_num = target;
			*len = *len - get_num_digit(num) - 1;
			return ptr + get_num_digit(num) + 1;
		}
	}
	*out_num = -1;
	*len = *len - i; 
	return ptr + i;
}

char *recognise_image_num(char *ptr, int *len, int *out_num)
{
	if (*ptr == 0) {
		*len = *len - 1;
		*out_num = -1;
		return ptr + 1;
	}
	return __recognise_image_num(ptr, len, out_num);
}

int calc_number_charactor(unsigned char *ptr, int len, unsigned char *num_charactor)
{
	int i, j;
	int w = len / (8 * 4);

	for (i = 0; i < 8; i ++) {
		for (j = 0; j < w; j ++) {
			*(num_charactor + j) +=
				*(ptr + (i * w + j) * 4) == 0xff ? 1 : 0;
		}
	}
	return ERR_NO_ERR;
}

void print_charactor(unsigned char ptr[][5], int len)
{
	int i,j;

	for (i = 0; i < len; i ++) {
		for (j = 0; j < 5; j ++) {
			printf("%d",ptr[i][j]);
		}
		printf(",\n");
	}
}

int get_role_item(struct t_bmp *input, RECT target_rc, int *info)
{
	struct t_bmp output = {};
	unsigned char *nums_charactor = NULL;
	char *ptr = NULL;
	int ret = 0;
	int len = 0;
	int tmp = 0;
	int i;

	ret = get_screen_rect(input, target_rc, &output);
	if (ret != ERR_NO_ERR) {
		return ret;
	}
	//save_picture(L"D://tmp.bmp", &output);
	len = output.len / ((target_rc.bottom - target_rc.top) * 4);
	nums_charactor = new unsigned char[len];
	memset(nums_charactor, 0 , len);
	calc_number_charactor(output.data, output.len, nums_charactor);
	ptr = (char *)nums_charactor;
#if 0
	for (i = 0; i < len; i ++) {
		printf("%d ", *(ptr + i));
	}
	printf("\n");
#endif
	i = 0;
	do {
		ptr = recognise_image_num(ptr, &len, &tmp);
		if (tmp == -1) {
			continue;
		}

		if (NUM_M == tmp) {
			i ++;
		}
		else {
			info[i] = info[i] * 10 + tmp;
		}
		//printf("recognise number: %d\n", tmp);
	} while (len > 0 && *ptr != ' ');

	delete[] nums_charactor;
	delete[] output.data;
	return ERR_NO_ERR; 
}

int get_role_hp_mp_level(struct t_bmp *input, struct role_status *role)
{
	RECT rc;
	int ret = 0;

	/* Get the role's HP position*/
	rc.left = 160;
	rc.right = 250;
	rc.top = 283;
	rc.bottom = 291;
	ret = get_role_item(input, rc, (int *)&role->hp);
	printf("Current HP %d, HP %d\n", role->hp.cur_hp, role->hp.hp);

	/* Get the role's MP position*/
	rc.left = 290;
	rc.right = 375;
	rc.top = 283;
	rc.bottom = 291;
	ret = get_role_item(input, rc, (int *)&role->mp);
	printf("Current MP %d, MP %d\n", role->mp.cur_mp, role->mp.mp);

	/* Get the role's level position*/
	rc.left = 200;
	rc.right = 242;
	rc.top = 243;
	rc.bottom = 251;
	ret = get_role_item(input, rc, (int *)&role->level);
	printf("Level %d\n", role->level);
	return ret;
}

int get_role_gold(struct t_bmp *input, struct role_status *role)
{
	RECT rc;
	int ret = 0;

	/* Get the role's Gold position*/
	rc.left = 620;
	rc.right = 690;
	rc.top = 535;
	rc.bottom = 543;
	ret = get_role_item(input, rc, (int *)&role->gold);
	printf("Current gold %d\n", role->gold);

	return ret;
}

void unit_test_show_nums_charactor(void)
{
	unsigned char nums_charactor[10][5];
	struct t_bmp input = {};
	int ret = 0;
	int i;
	wchar_t cmd[128] = {};

	for (i = 0; i < 10; i ++) {
		wsprintf(cmd, L"D://0.%d.bmp", i);
		ret = load_picture(cmd, &input);
		if (ret != ERR_NO_ERR) {
			return;
		}
		calc_number_charactor(input.data, input.len, nums_charactor[i]);
		delete[] input.data;
	}
	print_charactor(nums_charactor, 10);
}

void unit_test_recognise_image_num(void)
{
	const char *num_char = "28162226281433332233522482633345333415325333543335";
	char *ptr = NULL;
	int recognise_num = 0;
	int len = strlen(num_char);
	ptr = (char *)num_char;

	do {
		ptr = recognise_image_num(ptr, &len, &recognise_num);
		if (recognise_num == -1) {
			continue;
		}
		printf("recognise number: %d\n", recognise_num);
	} while (len > 0 && *ptr != ' ');
}

void unit_test_calc_image_charactor(void)
{
	int *output = NULL; 
	struct t_bmp input = {};
	wchar_t cmd[128] = {};
	int ret = 0;
	int i = 0;

	for (i = 0; i < 10; i ++) {
		wsprintf(cmd, L"D://0.%d.bmp", i);
		ret = load_picture(cmd, &input);
		if (ret != ERR_NO_ERR) {
			return;
		}
		output = new int[input.bih.biWidth];
		memset(output, 0, sizeof(int) * input.bih.biWidth);
		__calc_image_charactor(&input, output);
		print_num(output, input.bih.biWidth);
		delete(input.data);
		delete(output);
	}
}

int unit_test_get_role_info(void)
{
	struct t_bmp input = {};
	int ret = 0;

	ret = load_picture(L"D://role_m.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}
	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 0);
	ret = get_role_hp_mp_level(&input, &role_info);
	delete input.data;

	ret = load_picture(L"D://role_i.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}

	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 0);
	ret = get_role_gold(&input, &role_info);

	if (ret != ERR_NO_ERR) {
		return ret;
	}

	//save_picture(L"D://tmp_role_m.bmp", &input);
	delete input.data;

	return 0;
}

/*
 *	input:		should be binary-handle(only black and white).
 *	charactor:	the charactor of things that you want to recognise.
 *	mode:		support 1. find first one, 2. find all.
 */

int get_target_position(struct t_bmp *input, POINT *charactor_black,
			int black_len, POINT *charactor_white, int white_len,
			int mode, POINT *target, int target_center)
{
	int i, j;
	int x, y;
	int d_x, d_y;		//delta_x and delta_y
	int w = input->bih.biWidth;
	int h = input->bih.biHeight;

	//TODO: need to achieve the FIND_ALL mode
	//TODO: need to do the vague

	if (NULL == input && NULL == input->data) {
		return NULL;
	}
	for (i = 0; i < (int)input->len; i ++) {
		y = (i / 4) / w;
		x = (i / 4) % w;
		if (white_len > 0) {
			for (j = 0; j < white_len; j ++) {
				d_x = (charactor_white + j)->x;
				d_y = (charactor_white + j)->y;
				if (x + d_x <= 0 || y + d_y <= 0 || x + d_x >= w ||
					y + d_y >= h) {
					break;
				}
				if (*(input->data + (x + d_x + (y + d_y) * w) * 4)
					!= WHITE_KEY_VALUE) {
					break;
				}
			}
			if (j != white_len) {
				continue;
			}
		}

		if (black_len > 0) {
			for (j = 0; j < black_len; j ++) {
				d_x = (charactor_black + j)->x;
				d_y = (charactor_black + j)->y;
				if (x + d_x <= 0 || y + d_y <= 0 || x + d_x >= w ||
					y + d_y >= h) {
					break;
				}
				if (*(input->data + (x + d_x + (y + d_y) * w) * 4)
					!= BLACK_KEY_VALUE) {
					break;
				}
			}
			if (j != black_len) {
				continue;
			}
		}
		if (j == black_len || j == white_len) {
			if (target_center) {
				target->x = x + (charactor_white + white_len - 1)->x / 2;
				target->y = h - y - (charactor_white + white_len - 1)->y / 2;
			}
			else {
				target->x = x;
				target->y = h - y;
			}
			return ERR_NO_ERR;
		}
	}
	return ERR_TARGET_NOT_FOUND;
}

int unit_test_get_big_map_info(void)
{
	struct t_bmp input = {};
	int ret = 0;
	POINT p = {};

	ret = load_picture(L"D://map_1.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}
	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 0);
	ret = get_target_position(&input, task_black_char,
			8, task_white_char, 8, FIND_FIRST, &p, true);
	if (ERR_NO_ERR == ret) {
		TRACE(T_INFO, "The target position (%d, %d)\n", p.x, p.y);
	}
	else {
		TRACE(T_INFO, "Could not find the target\n");
	}

	ret = get_target_position(&input, role_black_char,
			10, role_white_char, 10, FIND_FIRST, &p, true);
	if (ERR_NO_ERR == ret) {
		TRACE(T_INFO, "The role position (%d, %d)\n", p.x, p.y);
	}
	else {
		TRACE(T_INFO, "Could not find the target\n");
	}

	save_picture(L"D://tmp_map_info.bmp", &input);
	delete input.data;

	return 0;
}

int unit_test_get_little_map_info(void)
{
	struct t_bmp input = {};
	struct t_bmp output = {};
	int ret = 0;
	POINT p = {};

	ret = load_picture(L"D://gate_2.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}

	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 80);

	ret = get_target_position(&input, task_map_black_char,
		sizeof(task_map_black_char)/sizeof(POINT),
		task_map_white_char, sizeof(task_map_black_char)/sizeof(POINT),
		FIND_FIRST, &p, false);
	if (ERR_NO_ERR == ret) {
		RECT map_rc = {};

		map_rc.top = 26;
		map_rc.bottom = p.y;
		map_rc.left = p.x;
		map_rc.right = input.bih.biWidth - 4;

		TRACE(T_INFO, "The target position (%d, %d)\n", p.x, p.y);
		ret = get_screen_rect(&input, map_rc, &output);
		save_picture(L"D://little_map.bmp", &output);
	}
	else {
		TRACE(T_INFO, "Could not find the target\n");
	}
	delete input.data;
	delete output.data;
	return 0;
}

int unit_test_get_map_role_info(void)
{
	struct t_bmp input = {};
	int ret = 0;
	POINT p = {};

	ret = load_picture(L"D://gate_2.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}

	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 80);

	ret = get_target_position(&input, map_role_black_char,
		sizeof(map_role_black_char)/sizeof(POINT),
		map_role_white_char, sizeof(map_role_black_char)/sizeof(POINT),
		FIND_FIRST, &p, true);
	if (ERR_NO_ERR == ret) {
		TRACE(T_INFO, "The target position (%d, %d)\n", p.x, p.y);
	}
	else {
		TRACE(T_INFO, "Could not find the target\n");
	}
	save_picture(L"D://map_role.bmp", &input);
	delete input.data;
	return 0;
}

int unit_test_get_map_next_info(void)
{
	struct t_bmp input = {};
	int ret = 0;
	POINT p = {};

	ret = load_picture(L"D://gate_2.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}

	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 80);

	ret = get_target_position(&input, map_next_black_char,
		sizeof(map_next_black_char)/sizeof(POINT),
		map_next_white_char, sizeof(map_next_black_char)/sizeof(POINT),
		FIND_FIRST, &p, true);
	if (ERR_NO_ERR == ret) {
		TRACE(T_INFO, "The target position (%d, %d)\n", p.x, p.y);
	}
	else {
		TRACE(T_INFO, "Could not find the target\n");
	}
	save_picture(L"D://map_role.bmp", &input);
	delete input.data;
	return 0;
}

int unit_test_get_map_boss_info(void)
{
	struct t_bmp input = {};
	int ret = 0;
	POINT p = {};

	ret = load_picture(L"D://gate_2.bmp", &input);
	if (ret != ERR_NO_ERR) {
		return 0;
	}

	ret = convert_gray(&input, BINARY_WEIGHTED_MEAN);
	ret = convert2blackwhite(&input, ONLY_BLACK, 80);

	ret = get_target_position(&input, map_boss_black_char,
		sizeof(map_boss_black_char)/sizeof(POINT),
		map_boss_white_char, sizeof(map_boss_white_char)/sizeof(POINT),
		FIND_FIRST, &p, true);
	if (ERR_NO_ERR == ret) {
		TRACE(T_INFO, "The target position (%d, %d)\n", p.x, p.y);
	}
	else {
		TRACE(T_INFO, "Could not find the target\n");
	}
	save_picture(L"D://map_role.bmp", &input);
	delete input.data;
	return 0;
}

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	//show_nums_charactor();
	//unit_test_get_num_charctor_by_num();
	//unit_test_calc_image_charactor();
	//unit_test_recognise_image_num();
	//unit_test_get_role_info();
	//unit_test_get_big_map_info();
	//unit_test_get_little_map_info();
	//unit_test_get_map_role_info();
	//unit_test_get_map_next_info();
	unit_test_get_map_boss_info();

	return 0;
}
#endif /* __OWN_MAIN__ */
