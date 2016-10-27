#include<windows.h>
#include<stdio.h>

#include "debug.h"

/*	
 *	What we need in the module: find a best way to the boss based on BFs.
 */
#define MAP_X	10
#define MAP_Y	10

struct route_node {
	POINT pos;
	int parent;
	int weight;
	int used;
};

enum {
	T_UNUSED = 0,
	T_USED = 1,
};

/* Check whether the node is repeated */
int check_repeat_node(struct route_node *ptr, int x, int y, int n)
{
	int i = 0;

	for (; i < n; i++) {
		if ((ptr + i)->pos.x == x && (ptr + i)->pos.y == y)
			return 1;
	}
	return 0;
}

int save_neighbour_pos(int map[][10], int x, int y, int target_x, int target_y, struct route_node *node, int current)
{
	int i, j;
	int n = 0;

	struct route_node *ptr = node + current;

	for (i = x - 1; i <= x + 1; i ++) {
		if (i < 0)
			continue;
		for (j = y - 1; j <= y + 1; j++) {
			/*	Only match UP DOWN LEFT RIGHT, need to filter x.
			 *	x		UP		x
			 *	LEFT		(x, y)		RIGHT
			 *	x		DOWN	x
			 */
			if (j < 0 || (abs(j - y) == abs(i - x)) || check_repeat_node(node, i, j, current + n)) {
				continue;
			}
			if (map[j][i] == 1) {
				(ptr + n)->pos.x = i;
				(ptr + n)->pos.y = j;
				(ptr + n)->used = T_UNUSED;
				(ptr + n)->parent = current;
				/* Key point: calculate the weight of position */
				(ptr + n)->weight = MAP_X + MAP_Y - abs(target_x - i) - abs(target_y - j);
				n++;
			}
		}
	}
	return current + n;
}

/* Get the position with the valid max weight */
int get_next_pos(struct route_node *node, int current, int n)
{
	int i;
	int max = -1;
	int flag = 0;

	/* Only one node */
	if (n == 1) {
		if (node->used == T_UNUSED)
			return 0;
		else
			return -1;
	}

	for (i = 0, max = 0; i < n; i++) {
		/* Find the first unused node */
		if (!flag) {
			if ((node + i)->used == T_UNUSED) {
				max = i;
				flag = 1;
			}
			else {
				continue;
			}
		}
		else {
			if ((node + i)->used == T_UNUSED &&
			(node + max)->weight < (node + i)->weight) {
				max = i;
			}
		}
	}
	if (max > 0)
		(node + max)->used = T_USED;
	return max;
}

int all_node_used(struct route_node *node, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if ((node + i)->used == T_UNUSED)
			return 0;
	}
	return 1;
}

void print_node(struct route_node *ptr, int n)
{
	if ((ptr + n)->parent != 0) {
		print_node(ptr, (ptr + n)->parent);
	}
	printf("X: %d, Y: %d\n", (ptr+n)->pos.x, (ptr+n)->pos.y);
}

int map_route(int map[][10], int x, int y, int target_x, int target_y)
{
	struct route_node node[100] = {};

	int i = x;
	int j = y;
	int n = 0;
	int current = 0;
	int old_cur = 0;

	node->parent = 0;
	node->pos.x = x;
	node->pos.y = y;
	node->used = T_USED;
	node->weight = MAP_X + MAP_Y - abs(target_x - x) - abs(target_y - y);
	n++;

	do {
		n = save_neighbour_pos(map, i, j, target_x, target_y, node, n);
		/* Get the max valid weight node */
		current = get_next_pos(node, current, n);
		(node + current)->parent = old_cur;
		i = (node + current)->pos.x;
		j = (node + current)->pos.y;
		old_cur = current;
	} while (all_node_used(node,n) || (!(i == target_x && j == target_y)));
	print_node(node, current);
	return ERR_NO_ERR;
}

int test_map[10][10] = {
	0,	1,	0,	0,	0,	0,	1,	1,	1,	0,
	0,	1,	1,	0,	1,	0,	1,	0,	1,	0,
	0,	1,	0,	0,	1,	0,	1,	0,	1,	0,
	1,	1,	1,	1,	1,	0,	1,	0,	1,	0,
	1,	0,	1,	0,	0,	0,	1,	0,	1,	0,
	1,	1,	1,	0,	0,	0,	1,	0,	1,	1,
	1,	0,	1,	0,	0,	0,	1,	0,	1,	0,
	0,	0,	1,	0,	0,	0,	1,	0,	1,	0,
	0,	0,	1,	1,	0,	1,	1,	0,	1,	0,
	0,	0,	0,	1,	1,	1,	0,	0,	1,	0
};

#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__
int main()
{
	map_route(test_map, 0, 5, 9, 5);
	return 0;
}
#endif /* __OWN_MAIN__ */
