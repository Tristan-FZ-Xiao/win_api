#ifndef __TIMER_H__
#define __TIMER_H__

typedef void (*fn)(void *data);

struct t_timer {
	struct t_timer *prev;
	struct t_timer *next;
	fn cb;
	void *data;
	int expired;
	int init_expired;
};

extern int timer_add(struct t_timer *entry);
extern int timer_del(struct t_timer *entry);
extern void timer_run(void);
extern int timer_init(void);

#endif __TIMER_H__