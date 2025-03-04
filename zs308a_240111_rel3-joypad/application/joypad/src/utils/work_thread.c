/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <os_common_api.h> // #include <logging/sys_log.h> #2855
#include <work_thread.h>

static OS_SEM_DEFINE(thread_sem, 0, 2);

// static char __noinit __aligned(STACK_ALIGN) thread_stack[2536];
#define THREAD_STACK_SIZE 2536
static OS_THREAD_STACK_DEFINE(thread_stack, THREAD_STACK_SIZE);

static bool quit_thread = true;

bool thread_quitted(void)
{
	return quit_thread;
}

static void work_thread_loop(void *p1, void *p2, void *p3)
{
	struct thread_data *data = p1;

	if (data->prepare_fn) {
		if (data->prepare_fn(data)) {
			os_sem_give(&thread_sem);
			goto out_exit;
		}
	}

	/* notify thread ready */
	os_sem_give(&thread_sem);

	if (!quit_thread)
		data->run_fn(data);
out_exit:
	if (data->unprepare_fn)
		data->unprepare_fn(data);

	k_thread_priority_set(k_current_get(), -CONFIG_NUM_COOP_PRIORITIES);
	os_sem_give(&thread_sem);
}

void destroy_work_thread(int wait_ms)
{
	if (quit_thread)
		return;

	if (os_sem_take(&thread_sem, wait_ms)) {
		quit_thread = true;
		os_sem_take(&thread_sem, OS_FOREVER);
	} else {
		quit_thread = true;
	}

	SYS_LOG_INF("thread destroyed");
}

int create_work_thread(struct thread_data *data, int prio)
{
	destroy_work_thread(OS_NO_WAIT);

	os_thread_create((char *)thread_stack, sizeof(thread_stack),
			 work_thread_loop, data, NULL, NULL, prio,
			 0, OS_NO_WAIT);
	quit_thread = false;
	os_sem_take(&thread_sem, OS_FOREVER);

	SYS_LOG_INF("thread created");
	return 0;
}
