/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TRANSFER_H__
#define __TRANSFER_H__

struct thread_data {
	int (*prepare_fn)(struct thread_data *data);
	int (*run_fn)(struct thread_data *data);
	int (*unprepare_fn)(struct thread_data *data);

	void *user_data;
};

int create_work_thread(struct thread_data *data, int prio);
/* wait how much time until finished */
void destroy_work_thread(int wait_ms);
bool thread_quitted(void);

#endif /* __TRANSFER_H__ */
