/*
 * Copyright (c) 2019 Actions Semi Co., Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief mem manager.
*/
#include <mem_manager.h>
#include "mem_inner.h"

#ifdef CONFIG_SYS_MEMORY_DEBUG
#include "mem_guard.h"
static struct mem_guard_head mem_guard;
#endif

void *mem_malloc_debug(size_t size,const char *func)
{
#ifdef CONFIG_SYS_MEMORY_DEBUG
    return mem_guard_malloc(&mem_guard, mem_pool_malloc, size, func);
#else
	return mem_pool_malloc(size);
#endif
}

void mem_free(void *ptr)
{
	if (ptr == NULL)
		return;

#ifdef CONFIG_SYS_MEMORY_DEBUG
    mem_guard_free(&mem_guard, mem_pool_free, ptr);
#else
	mem_pool_free(ptr);
#endif
}

void mem_manager_dump(void)
{
#ifdef CONFIG_SYS_MEMORY_DEBUG
	mem_guard_dump(&mem_guard);
#else
	mem_pool_dump();
#endif
}

void mem_manager_dump_ext(int dump_detail, const char* match_value)
{

}

int mem_manager_init(void)
{
#ifdef CONFIG_SYS_MEMORY_DEBUG
	mem_guard_init(&mem_guard);
#endif
	mem_pool_init();
	return 0;
}

