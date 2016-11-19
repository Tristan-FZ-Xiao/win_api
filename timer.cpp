#include <stdio.h>
#include <Winsock2.h>
#include<windows.h>
#include <wchar.h>
#include <math.h>

#include "init.h"
#include "debug.h"
#include "output_handle.h"
#include "snap_screen.h"
#include "timer.h"

struct t_timer *timer_head = NULL;

int timer_init(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested,&wsaData);

	if (err != 0) {
		printf("WSAStartup failed witherror: %d\n", err);
		return ERR_COMMON_NULL_ERROR;
	}
	return ERR_NO_ERR;
}

int timer_add(struct t_timer *entry)
{
	if (timer_head) {
		/* only one entry */
		if (timer_head->next == timer_head) {
			timer_head->next = entry;
			timer_head->prev = entry;
			entry->next = timer_head;
			entry->prev = timer_head;
		}
		else {
			timer_head->prev->next = entry;
			entry->next = timer_head;
			entry->prev = timer_head->prev;
			timer_head->prev = entry;
		}
	}
	else {
		timer_head = entry;
		entry->next = entry;
		entry->prev = entry;
	}
	return 0;
}

int timer_del(struct t_timer *entry)
{
	if (entry) {
		if (entry->next == entry) {
			timer_head = NULL;
		}
		else {
			entry->prev->next = entry->next;
			entry->next->prev = entry->prev;
		}
	}
	return 0;
}

void timer_run(void)
{
	int ret = 0;
	struct timeval tv;
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fd_set rfds;
	sockaddr_in service;
	struct t_timer *ptr = NULL;

	if (fd <= 0) {
		TRACE(T_ERROR, "socket create error\n");
		return;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(34000);
	ret = bind(fd, (SOCKADDR *) &service, sizeof(service));
	if (ret == -1) {
		TRACE(T_ERROR, "bind error\n");
		return;
	}
	ret = listen(fd, SOMAXCONN);
	if (ret == -1) {
		TRACE(T_ERROR, "listen error\n");
		return;
	}
	
	while (1) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		ret = select(1, &rfds, NULL, NULL, &tv);
		//TRACE(T_INFO, "%d %d\n", ret, WSAGetLastError());
		for (ptr = timer_head; ptr != NULL; ptr = ptr->next) {
			if (ptr->expired) {
				ptr->expired --;
			}
			else {
				ptr->cb(ptr->data);
				if (ptr->init_expired)
					ptr->expired = ptr->init_expired;
				else
					timer_del(ptr);
			}
			if (ptr->next == timer_head) {
				break;
			}
		}
	}
	return;
}

void test_1(void *data)
{
	TRACE(T_ERROR, "test_1\n");
}

void test_2(void *data)
{
	TRACE(T_ERROR, "test_2\n");
}

struct t_timer a, b;

int unit_test_timer(void)
{

	a.cb = test_1;
	a.data = NULL;
	a.expired = 100;
	a.init_expired = 100;
	b.cb = test_2;
	b.data = NULL;
	b.expired = 200;
	b.init_expired = 200;

	timer_add(&a);
	timer_add(&b);
	timer_run();
	return 0;
}

//#define __OWN_MAIN__ 1
#ifdef __OWN_MAIN__
int main()
{
	int ret = ERR_NO_ERR;

	ret = timer_init();
	if (ret != ERR_NO_ERR) {
		return unit_test_timer();
	}
	WSACleanup();
}
#endif /* __OWN_MAIN__ */