/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "lvgl_malloc.h"
#include <stdlib.h>
#include <zephyr.h>
#include <init.h>
#include <string.h>
#include <sys/sys_heap.h>
#include <zephyr/types.h>

#define HEAP_BYTES (CONFIG_LV_MEM_SIZE_KILOBYTES * 1024)

__in_section_unique(lvgl.noinit.malloc) __aligned(4) static char lvgl_malloc_heap_mem[HEAP_BYTES];
__in_section_unique(lvgl.noinit.malloc) static struct sys_heap lvgl_malloc_heap;

void *lvgl_malloc(size_t size)
{
	return sys_heap_aligned_alloc(&lvgl_malloc_heap, sizeof(void *), size);
}

void *lvgl_realloc(void *ptr, size_t requested_size)
{
	return sys_heap_aligned_realloc(&lvgl_malloc_heap, ptr, sizeof(void *), requested_size);
}

void lvgl_free(void *ptr)
{
	sys_heap_free(&lvgl_malloc_heap, ptr);
}

static int lvgl_malloc_prepare(const struct device *unused)
{
	sys_heap_init(&lvgl_malloc_heap, lvgl_malloc_heap_mem, HEAP_BYTES);
	return 0;
}

SYS_INIT(lvgl_malloc_prepare, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
