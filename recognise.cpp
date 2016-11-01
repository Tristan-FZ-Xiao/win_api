#include <stdio.h>
#include<windows.h>
#include <wchar.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"
#include "recognise.h"

/*	What we need in the module: do some basic recognition likes:
 *	1. Number;
 */
/*	The charactor of 0 - 9
 */
unsigned char num_charactor[10][5] = {
	{6, 2, 2, 2, 6},	/* 0 */
	{2, 8, 1, 0, 0},	/* 1 */
	{4, 3, 3, 3, 3},	/* 2 */
	{2, 2, 3, 3, 5},	/* 3 */
	{2, 2, 4, 8, 2},	/* 4 */
	{6, 3, 3, 3, 4},	/* 5 */
	{5, 3, 3, 3, 4},	/* 6 */
	{1, 5, 3, 2, 0},	/* 7 */
	{5, 3, 3, 3, 5},	/* 8 */
	{4, 3, 3, 3, 5}		/* 9 */	
};

void __get_num_charactor_by_num(unsigned char num[][5], int in_len, unsigned char target, int bit,
				unsigned char **output, int *out_len)
{
	int i;
	int j = 0;

	for (i = 0; i < in_len; i ++) {
		if (num[i][bit] == target) {
			*(output + j) = num[i];
			j ++;
		}
	}
	*out_len = j;
}

void print_num(unsigned char *ptr, int len)
{
	int i;

	for (i = 0; i < len; i ++) {
		printf("%d ", *(ptr + i));
	}
	printf("\n");
}

void unit_test_get_num_charctor_by_num(void)
{
	unsigned char *output[10];
	int out_len;
	int i;

	__get_num_charactor_by_num(num_charactor, 10, 6, 0,
				output, &out_len);
	for (i = 0; i < out_len; i ++) {
		print_num(output[i], 5);
	}
}

void __calc_image_charactor(struct t_bmp *input, unsigned char *output)
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

void unit_test_calc_image_charactor(void)
{
	unsigned char *output = NULL; 
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
		output = new unsigned char[input.bih.biWidth];
		memset(output, 0, input.bih.biWidth);
		__calc_image_charactor(&input, output);
		print_num(output, input.bih.biWidth);
		delete(input.data);
		delete(output);
	}

}

unsigned char nums_charactor[10][5];
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
		printf("{");
		for (j = 0; j < 5; j ++) {
			if (j == 4) {
				printf("%d",ptr[i][j]);
			}
			else {
				printf("%d, ",ptr[i][j]);
			}
		}
		printf("}\n");
	}
}

void show_nums_charactor(void)
{
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


#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__

int main()
{
	//show_nums_charactor();
	//unit_test_get_num_charctor_by_num();
	unit_test_calc_image_charactor();
	return 0;
}
#endif /* __OWN_MAIN__ */
