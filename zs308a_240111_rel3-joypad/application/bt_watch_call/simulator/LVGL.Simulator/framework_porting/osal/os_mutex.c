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

int os_mutex_create(void)
{
#ifdef CONFIG_SIMULATOR
	return  CreateMutex(NULL,FALSE, NULL);
#endif
}

int os_mutex_init(os_mutex *mutex)
{
#ifdef CONFIG_SIMULATOR
	return 0;
#else
	return k_mutex_init(mutex);
#endif
}

int os_mutex_lock(os_mutex *mutex, int32_t timeout)
{
#ifdef CONFIG_SIMULATOR
	return WaitForSingleObject(mutex,timeout);
#else
	return k_mutex_lock(mutex, SYS_TIMEOUT_MS(timeout));
#endif
}

int os_mutex_unlock(os_mutex *mutex)
{
#ifdef CONFIG_SIMULATOR
	return ReleaseMutex(mutex);
#else
	return k_mutex_unlock(mutex);
#endif
}

int os_mutex_release(os_mutex *mutex)
{
#ifdef CONFIG_SIMULATOR
	return  CloseHandle(mutex);
#endif
}
