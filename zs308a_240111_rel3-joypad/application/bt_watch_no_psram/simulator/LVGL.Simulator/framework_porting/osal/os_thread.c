/*
 * Copyright (c) 2017 Actions Semi Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 * Author: wh<wanghui@actions-semi.com>
 *
 * Change log:
 *	2021/12/9: Created by wh.
 */

#include "os_common_api.h"
#include "stdbool.h"
#include "string.h"

#ifdef CONFIG_SIMULATOR
#include <Windows.h>
#endif

#define MAX_THREAD_TERMINAL_NUM 3

struct thread_terminal_info_t {
	os_thread *wait_terminal_thread;
	os_sem terminal_sem;
};

static struct thread_terminal_info_t thread_terminal_info[MAX_THREAD_TERMINAL_NUM] = {0};

static void os_thread_abort_callback(os_thread *aborted)
{
	os_sched_lock();
	for (int i = 0; i < MAX_THREAD_TERMINAL_NUM; i++){
		if(thread_terminal_info[i].wait_terminal_thread == aborted) {
			os_sem_give(&thread_terminal_info[i].terminal_sem);
			SYS_LOG_INF(" %p \n",aborted);
		}
	}
	os_sched_unlock();
}

/**thread function */
int os_thread_create(char *stack, size_t stack_size,
					 void (*entry)(void *, void *, void*),
					 void *p1, void *p2, void *p3,
					 int prio, uint32_t options, int delay) {
#ifdef CONFIG_SIMULATOR
	DWORD ThreadID;
	CreateThread( 
                     NULL,       // default security attributes
                     0,          // default stack size
                     (LPTHREAD_START_ROUTINE) entry, 
                     NULL,       // no thread function arguments
                     0,          // default creation flags
                     &ThreadID); // receive thread identifier
	return ThreadID;
#else
	k_tid_t tid = NULL;

	os_thread *thread = NULL;

	thread = (os_thread *)stack;

	tid = k_thread_create(thread, (os_thread_stack_t *)&stack[sizeof(os_thread)],
							stack_size - sizeof(os_thread),
							entry,
							p1, p2, p3,
							prio,
							options,
							SYS_TIMEOUT_MS(delay));

	thread->fn_abort = os_thread_abort_callback;

	return (int)tid;
#endif
}

int os_thread_prepare_terminal(int tid)
{

	int ret = 0;
#ifndef CONFIG_SIMULATOR
	struct thread_terminal_info_t *terminal_info = NULL;

	os_sched_lock();

	for (int i = 0; i < MAX_THREAD_TERMINAL_NUM; i++){
		if(!thread_terminal_info[i].wait_terminal_thread) {
			terminal_info = &thread_terminal_info[i];
			break;
		}
	}

	if (!terminal_info) {
		SYS_LOG_ERR("%p busy\n", tid);
		ret = -EBUSY;
		goto exit;
	}

	terminal_info->wait_terminal_thread = tid;
	os_sem_init(&terminal_info->terminal_sem, 0, 1);

	SYS_LOG_INF(" 0x%x ok\n",tid);
exit:
	os_sched_unlock();
#endif
	return ret;
}

int os_thread_wait_terminal(int tid)
{
	int ret = 0;
#ifndef CONFIG_SIMULATOR
	struct thread_terminal_info_t *terminal_info = NULL;

	os_sched_lock();
	for (int i = 0; i < MAX_THREAD_TERMINAL_NUM; i++){

		if(thread_terminal_info[i].wait_terminal_thread == tid) {
			terminal_info = &thread_terminal_info[i];
		}
	}
	os_sched_unlock();

	if (!terminal_info) {
		SYS_LOG_ERR("terminal tid %p not found\n",tid);
		ret = -EBUSY;
	}

	if (os_sem_take(&terminal_info->terminal_sem, 5*1000)) {
		SYS_LOG_ERR("timeout \n");
		ret = -EBUSY;
	}

	os_sched_lock();
	terminal_info->wait_terminal_thread = NULL;
	os_sched_unlock();

	SYS_LOG_INF(" 0x%x ok\n",tid);
#endif
	return ret;
}

const char *os_thread_get_name_by_prio(int prio)
{
#ifndef CONFIG_SIMULATOR
	struct k_thread *thread_list = (struct k_thread *)(_kernel.threads);
	unsigned int key = irq_lock();	

	while (thread_list != NULL) {
		int thread_prio = os_thread_priority_get(thread_list);
		if (prio == thread_prio) {
			break;
		}

		thread_list = (struct k_thread *)thread_list->next_thread;
	}
	irq_unlock(key);

	if (thread_list) {
		return k_thread_name_get(thread_list);
	}
#endif
	return "NULL";
}

int os_thread_priority_get(int osthread)
{
	return 0;
}

int os_thread_priority_set(int osthread, int prio)
{
	return 0;
}

int os_thread_name_set(int thread_id, const char * value)
{

}
void os_thread_abort(int thread)
{
	
}
void os_thread_cancel(int thread)
{
	
}
