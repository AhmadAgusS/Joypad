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

void os_work_init(os_work *work, os_work_handler_t handler)
{
#ifdef CONFIG_SIMULATOR
	
#else
	return  k_work_init(work, handler);
#endif
}

void os_work_submit_to_queue(os_work_q *work_q, os_work *work)
{
#ifndef CONFIG_SIMULATOR
	return  k_work_submit_to_queue(work_q, work);
#endif
}

void os_work_submit(os_work *work)
{
#ifndef CONFIG_SIMULATOR
	return  k_work_submit(work);
#endif
}

void os_delayed_work_init(os_delayed_work *work, os_work_handler_t handler)
{
#ifndef CONFIG_SIMULATOR
	return  k_delayed_work_init(work, handler);
#endif
}

int os_delayed_work_submit(os_delayed_work *work, int32_t delay)
{
#ifndef CONFIG_SIMULATOR
	return  k_delayed_work_submit(work,SYS_TIMEOUT_MS(delay));
#endif
}

int os_delayed_work_cancel(os_delayed_work *work)
{
#ifndef CONFIG_SIMULATOR
	return  k_delayed_work_cancel(work);
#endif
}

int os_delayed_work_submit_to_queue(os_work_q *work_q, os_delayed_work *work, int32_t delay)
{
#ifndef CONFIG_SIMULATOR
	return  k_delayed_work_submit_to_queue(work_q, work, SYS_TIMEOUT_MS(delay));
#endif
}
