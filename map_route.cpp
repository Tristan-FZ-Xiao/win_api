#include<windows.h>
#include<stdio.h>

#include "debug.h"

/*	
 *	What we need in the module: find a best way to the boss based on BFs.
 */

int save_neighbour_pos(int *map, int width, int heigh, int x, int y)
{
	/*	Only match UP DOWN LEFT RIGHT, need to filter x.
	 *	x		UP		x
	 *	LEFT		(x, y)		RIGHT
	 *	x		DOWN	x
	 */
	// LEFT
	if (x != 0) {
		if (*(map + y * width + x - 1) != 0)
			*(map + y * width + x - 1)  += 1;
	}
	// UP
	if (y != 0) {
		if (*(map + (y - 1) * width + x) != 0)
			*(map + (y - 1) * width + x) += 1;
	}
	// RIGHT
	if (x != width - 1) {
		if (*(map + y * width + x  + 1) != 0)
			*(map + y * width + x + 1) += 1;
	}
	// DOWN
	if (y != heigh - 1) {
		if (*(map + (y + 1) * width + x) != 0)
			*(map + (y + 1) * width + x) += 1;
	}
	return 0;
}

enum {
	DIRECT_UP = 100,
	DIRECT_DOWN,
	DIRECT_LEFT,
	DIRECT_RIGHT,
	DIRECT_UN_DO,
};

/*
 * map:	test map
 * width:	map's width
 * heigh:	map's heigh
 * x:		current position (x)
 * y:		current position (y)
 * out_i:	next step(x)
 * out_j:	next step(y)
 * direct_x, direct_y:	show relate position between target_x and current_x, it help more effective way to the target.
 *
 */
int get_next_pos(int map[], int width, int heigh, int x, int y, int *out_i, int * out_j, int direct_x, int direct_y)
{
	int min = 9999;
	*out_i = 9999;
	*out_j = 9999;

	/* LEFT. */
	if (x != 0) {
		if (*(map + y * width + x - 1) &&  min > *(map + y * width + x - 1)) {
			min = *(map + y * width + x - 1);
			*out_i = x - 1;
			*out_j = y;
		}
	}
	// UP
	if (y != 0) {
		if (*(map + (y - 1) * width + x) && min > *(map + (y - 1) * width + x)) {
			min = *(map + (y - 1) * width + x);
			*out_i = x;
			*out_j = y - 1;
		}
		else if (1 && direct_y == DIRECT_UP && (min == *(map + (y - 1) * width + x))) {
			*out_i = x;
			*out_j = y - 1;
		}
	}
	// RIGHT
	if (x != width - 1) {
		if (*(map + y * width + x + 1) && min > *(map + y * width + x + 1)) {
			min = *(map + y * width + x + 1);
			*out_i = x + 1;
			*out_j = y;
		}
		else if (1 && direct_x == DIRECT_RIGHT && (min == *(map + y * width + x + 1))) {
			*out_i = x + 1;
			*out_j = y;
		}
	}
	// DOWN
	if (y != heigh - 1) {
		if (*(map + (y + 1) * width + x) && min > *(map + (y + 1) * width + x)) {
			*out_i = x;
			*out_j = y + 1;
		}
		else if (1 && direct_y == DIRECT_DOWN && (min == *(map + (y + 1) * width + x))) {
			*out_i = x;
			*out_j = y + 1;
		}
	}

	if (*out_i == 9999) {
		return -1;
	}
	*(map + *out_j * width + *out_i) = *(map + (y * width + x)) + 1;
	return ERR_NO_ERR;
}

void print_map(int map[], int width, int heigh)
{
	int i, j;

	printf("The MAP:\n\n");
	for (i = 0; i < heigh; i ++) {
		for (j = 0; j < width; j ++) {
			printf("%d\t", *(map + i * width + j));
		}
		printf("\n");
	}
}

void print_route(POINT *route, int len)
{
	int i;
	TRACE(T_INFO, "Show the Route INFO:\n");
	for (i = 0; i < len; i++) {
		if (i && (route + i)->x == 0 && (route + i)->y == 0) {
			break;
		}
		TRACE(T_INFO, "[%d]:\t\t (%d,\t%d)", i, (route + i)->x, (route + i)->y);
	}
	TRACE(T_INFO, "Finish the Route INFO\n");
}

POINT *map_route(int map[], int width, int heigh, int x, int y, int target_x, int target_y)
{
	int ret = 0;
	int i = 0;

	POINT *route = new POINT[width * heigh];
	if (!route) {
		return NULL;
	}
	memset(route, 0, sizeof(POINT) * width * heigh);
	(route + i)->x = x;
	(route + i ++)->y = y;

	do {
		save_neighbour_pos(map, width, heigh, x, y);
		print_map(map, width, heigh);
		ret = get_next_pos(map, width, heigh, x, y, &x, &y,
			(x < target_x ? DIRECT_RIGHT : (x == target_x ? DIRECT_UN_DO : DIRECT_LEFT)),
			(y < target_y ? DIRECT_DOWN : (y == target_y ? DIRECT_UN_DO : DIRECT_UP)));
		if (i < width * heigh) {
			(route + i)->x = x;
			(route + i++)->y = y;
		}
	} while (ret != 0 || !(x == target_x && y == target_y));
	return route;
}

int test_map_1[100] = {
	0,	1,	0,	0,	0,	0,	1,	1,	1,	0,
	0,	1,	1,	0,	1,	0,	1,	0,	1,	0,
	0,	1,	0,	0,	1,	0,	1,	0,	1,	0,
	1,	1,	1,	1,	1,	0,	1,	1,	1,	0,
	1,	0,	1,	0,	0,	1,	1,	0,	1,	0,
	1,	1,	1,	0,	1,	1,	1,	0,	1,	1,
	1,	0,	1,	0,	1,	1,	1,	1,	1,	0,
	0,	0,	1,	0,	1,	1,	0,	0,	1,	0,
	0,	0,	1,	1,	0,	1,	1,	0,	1,	0,
	0,	0,	0,	1,	1,	1,	0,	0,	1,	0
};

int test_map_2[50] = {
	1,	0,	1,	1,	1,	0,	1,	1,	1,	1,
	1,	1,	0,	0,	1,	0,	1,	0,	0,	0,
	1,	0,	1,	1,	1,	1,	1,	0,	0,	1,
	1,	1,	1,	0,	0,	0,	1,	1,	1,	1,
	1,	0,	1,	1,	1,	1,	0,	0,	1,	0
};

//#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__
int main()
{
	POINT *ptr = map_route(test_map_1, 10, 10, 0, 5, 9, 5);
	//POINT *ptr = map_route(test_map_2, 10, 5, 0, 0, 9, 2);
	print_route(ptr, 10 * 5);
	return 0;
}
#endif /* __OWN_MAIN__ */
