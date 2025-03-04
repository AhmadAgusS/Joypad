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
#include "stdint.h"
#include "string.h"

#ifdef CONFIG_SIMULATOR
#include <Windows.h>
#endif

int os_sem_create(uint32_t initial_count, uint32_t limit)
{
#ifdef CONFIG_SIMULATOR
	return CreateSemaphore( 
        NULL,           // default security attributes
        initial_count,  // initial count
        limit,  // maximum count
        NULL);          // unnamed semaphore
#else
	return 0;
#endif	
}

int os_sem_init(os_sem *sem, uint32_t initial_count, uint32_t limit)
{
#ifdef CONFIG_SIMULATOR
	HANDLE ghSemaphore = CreateSemaphore( 
        NULL,           // default security attributes
        initial_count,  // initial count
        limit,  // maximum count
        NULL);          // unnamed semaphore
	*sem = ghSemaphore;
	return 0;
#else
	return k_sem_init(sem, initial_count, limit);
#endif
}

int os_sem_take(os_sem *sem, int32_t timeout)
{
#ifdef CONFIG_SIMULATOR
#if 0
	return WaitForSingleObject( 
            sem,   // handle to semaphore
            timeout);           // zero-second time-out interval
#endif
	return 0;
#else
	return os_sem_take(sem, SYS_TIMEOUT_MS(timeout));
#endif
}

void os_sem_give(os_sem *sem)
{
#ifdef CONFIG_SIMULATOR
	return ReleaseSemaphore( 
                        sem,  // handle to semaphore
                        1,            // increase count by one
                        NULL);      // not interested in previous count
#else
	return  k_sem_give(sem);
#endif
}

void os_sem_release(os_sem *sem)
{
#ifdef CONFIG_SIMULATOR
	CloseHandle(sem);
#endif
}

int os_sem_reset(os_sem *sem)
{
#ifdef CONFIG_SIMULATOR
	return 0;
#else
	return os_sem_reset(sem);
#endif
}
