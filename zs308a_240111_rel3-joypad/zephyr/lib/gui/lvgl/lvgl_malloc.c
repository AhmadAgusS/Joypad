/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr.h>
#include <init.h>
#include <sys/sys_heap.h>
#include <lvgl.h>
#include "lvgl_malloc.h"


#include <logging/log.h>
LOG_MODULE_DECLARE(lvgl, LOG_LEVEL_INF);

#ifdef CONFIG_LV_MEM_CUSTOM

#define HEAP_BYTES (CONFIG_LV_MEM_SIZE_KILOBYTES * 1024)

__in_section_unique(lvgl.noinit.malloc) __aligned(4) static char lvgl_malloc_heap_mem[HEAP_BYTES];
__in_section_unique(lvgl.noinit.malloc) static struct sys_heap lvgl_malloc_heap;

#ifdef CONFIG_LV_MEM_DEBUG
#include "mem_guard.h"
static struct mem_guard_head mem_guard;
#endif

static void *base_malloc(size_t size)
{
	return sys_heap_aligned_alloc(&lvgl_malloc_heap, sizeof(void *), size);
}

static void base_free(void *ptr)
{
	sys_heap_free(&lvgl_malloc_heap, ptr);
}

static void *base_realloc(void *ptr, size_t requested_size)
{
	return sys_heap_aligned_realloc(&lvgl_malloc_heap, ptr, sizeof(void *), requested_size);
}

static void _assert_lvgl_tid(const char *tag)
{
	static k_tid_t lvgl_tid = NULL;
	k_tid_t tid = k_current_get();

	if (lv_is_initialized() == false) {
		return;
	}

	if (lvgl_tid == NULL) {
		LOG_INF("lvgl thread tid %p\n", tid);
		lvgl_tid = tid;
		return;
	}

	__ASSERT(lvgl_tid == tid, "%s called in thread %p\n", tag, tid);

	if (lvgl_tid != tid) {
		LOG_ERR("%s called in thread %p\n", tag, tid);
		k_panic();
	}
}

void *lvgl_malloc(size_t size, const char *func)
{
	_assert_lvgl_tid(__func__);

#ifdef CONFIG_LV_MEM_DEBUG
    return mem_guard_malloc(&mem_guard, base_malloc, size, func);
#else
	return base_malloc(size);
#endif
}
void *lvgl_realloc(void *ptr, size_t requested_size, const char *func)
{
	_assert_lvgl_tid(__func__);
#ifdef CONFIG_LV_MEM_DEBUG
	return mem_guard_realloc(&mem_guard, base_realloc, ptr, requested_size, func);
#else
	return base_realloc(ptr,requested_size);
#endif
}

void lvgl_free(void *ptr, const char *func)
{
	_assert_lvgl_tid(__func__);

#ifdef CONFIG_LV_MEM_DEBUG
    mem_guard_free(&mem_guard, base_free, ptr);
#else
	base_free(ptr);
#endif
}

static int lvgl_malloc_prepare(const struct device *unused)
{
	sys_heap_init(&lvgl_malloc_heap, lvgl_malloc_heap_mem, HEAP_BYTES);
#ifdef CONFIG_LV_MEM_DEBUG
	mem_guard_init(&mem_guard);
#endif
	return 0;
}

void lvgl_mem_leak_check(int view_id)
{
#ifdef CONFIG_LV_MEM_DEBUG
	_assert_lvgl_tid(__func__);
	LOG_INF("lvgl_mem check: \n");
	mem_guard_leak_check(&mem_guard, view_id);
	LOG_INF("lvgl_mem check: end\n");
#endif
}

SYS_INIT(lvgl_malloc_prepare, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

void lvgl_heap_dump(void)
{
#ifdef CONFIG_LV_MEM_DEBUG
	mem_guard_dump(&mem_guard);
	LOG_INF("total size of pool %d \n", HEAP_BYTES);
#else
	sys_heap_dump(&lvgl_malloc_heap);
#endif
}

#else /* CONFIG_LV_MEM_CUSTOM */

void lvgl_heap_dump(void)
{
	lv_mem_monitor_t monitor;

	lv_mem_monitor(&monitor);

	printk("            total: %u\n", monitor.total_size);
	printk("         free_cnt: %u\n", monitor.free_cnt);
	printk("        free_size: %u\n", monitor.free_size);
	printk("free_biggest_size: %u\n", monitor.free_biggest_size);
	printk("         used_cnt: %u\n", monitor.used_cnt);
	printk("         max_used: %u\n", monitor.max_used);
}

#endif /* CONFIG_LV_MEM_CUSTOM */
