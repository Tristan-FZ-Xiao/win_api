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
int num_charactor[10] = {
	62226,	/* 0 */
	281,	/* 1 */
	43333,	/* 2 */
	22335,	/* 3 */
	22482,	/* 4 */
	63334,	/* 5 */
	53334,	/* 6 */
	1532,	/* 7 */
	53335,	/* 8 */
	43335	/* 9 */	
};

#include <math.h>

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

void __get_num_charactor_by_num(int *num, int in_len, char target, int bit,
				int *output, int *out_len)
{
	int i;
	int j = 0;
	int n = 0; //(int)pow((long double)10, bit);

	for (i = 0; i < in_len; i ++) {
		n = get_num_digit(num[i]);
		n = (int) pow((long double)10, n - bit);
		if ((num[i] / n % 10) == target) {
			*(output + j) = num[i];
			j ++;
		}
	}
	*out_len = j;
}

int get_num_by_charactor(int charactor)
{
	int i;

	for (i = 0; i < 10; i ++) {
		if (charactor == num_charactor[i]) {
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

void unit_test_get_num_charctor_by_num(void)
{
	int output[10] = {};
	int out_len;
	int i;

	__get_num_charactor_by_num(num_charactor, 10, 6, 0,
				output, &out_len);
	for (i = 0; i < out_len; i ++) {
		print_num(output, 5);
	}
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


char * __recognise_image_num(char *ptr, int *len, int *out_num)
{
	int output[10] = {};
	int out_len;
	int i;

	__get_num_charactor_by_num(num_charactor, 10, *ptr - 48, 0,
				output, &out_len);
	for (i = 1; i < 5; i ++) {
		__get_num_charactor_by_num(output, out_len, *(ptr + i) - 48, i,
				output, &out_len);
		if (out_len == 1) {
			*out_num = get_num_by_charactor(output[0]);
			*len = *len - get_num_digit(output[0]) - 1;
			return (ptr + get_num_digit(output[0]) + 1);
		}
	}
}

void unit_test_recognise_image_num(void)
{
	const char *num_char = "28162226281433332233522482633345333415325333543335";
	char *ptr = NULL;

	int len = strlen(num_char);
	int recognise_num = 0;
	ptr = (char *)num_char;

	do {
		ptr = __recognise_image_num(ptr, &len, &recognise_num);
		printf("recognise number: %d\n", recognise_num);
	} while (len > 0 && *ptr != ' ');
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
	//unit_test_calc_image_charactor();
	unit_test_recognise_image_num();

	return 0;
}
#endif /* __OWN_MAIN__ */
