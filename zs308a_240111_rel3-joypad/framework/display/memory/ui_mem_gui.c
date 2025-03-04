/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_MODULE_CUSTOMER

#include <os_common_api.h>
#include <sys/sys_heap.h>
#include <ui_mem.h>
#include <assert.h>
#ifdef CONFIG_LVGL
#  include <lvgl.h>
#endif

LOG_MODULE_DECLARE(ui_mem, LOG_LEVEL_INF);

#if defined(CONFIG_UI_GUI_MEM_POOL_SIZE) && CONFIG_UI_GUI_MEM_POOL_SIZE > 0

#if defined(CONFIG_UI_GUI_SRAM_POOL_SIZE) && CONFIG_UI_GUI_SRAM_POOL_SIZE > 0
__in_section_unique(lvgl.noinit.malloc.0) __aligned(4) static uint8_t gui_heap_sram_mem[CONFIG_UI_GUI_SRAM_POOL_SIZE];
static struct sys_heap gui_heap_sram;
#endif

__in_section_unique(lvgl.noinit.malloc.1) __aligned(4) static uint8_t gui_heap_mem[CONFIG_UI_GUI_MEM_POOL_SIZE];
static struct sys_heap gui_heap;

static void _assert_in_gui_tid(const char *tag)
{
	static uintptr_t gui_tid = 0;
	uintptr_t tid = (uintptr_t)os_current_get();

#ifdef CONFIG_LVGL
	if (lv_is_initialized() == false) {
		return;
	}
#endif

	if (gui_tid == 0) {
		gui_tid = tid;
		return;
	}

	if (gui_tid != tid) {
		os_printk("%s called in thread 0x%x\n", tag, tid);
		assert(gui_tid == tid);

#ifndef CONFIG_SIMULATOR
		k_panic();
#endif /* CONFIG_SIMULATOR */
	}
}

static struct sys_heap* ui_mem_get_heap(const void * ptr)
{
	const uint8_t *ptr8 = ptr;

#if defined(CONFIG_UI_GUI_SRAM_POOL_SIZE) && CONFIG_UI_GUI_SRAM_POOL_SIZE > 0
	if ((ptr8 >= gui_heap_sram_mem) && (ptr8 < (gui_heap_sram_mem + CONFIG_UI_GUI_SRAM_POOL_SIZE))) {
		return &gui_heap_sram;
	}
#endif
	if ((ptr8 >= gui_heap_mem) && (ptr8 < (gui_heap_mem + CONFIG_UI_GUI_MEM_POOL_SIZE))) {
		return &gui_heap;
	}

	if (ptr == NULL) {
#if defined(CONFIG_UI_GUI_SRAM_POOL_SIZE) && CONFIG_UI_GUI_SRAM_POOL_SIZE > 0
		return &gui_heap_sram;
#else
		return &gui_heap;
#endif
	}

	return NULL;
}

int ui_mem_gui_init(void)
{
#if defined(CONFIG_UI_GUI_SRAM_POOL_SIZE) && CONFIG_UI_GUI_SRAM_POOL_SIZE > 0
	sys_heap_init(&gui_heap_sram, gui_heap_sram_mem, CONFIG_UI_GUI_SRAM_POOL_SIZE);
#endif
	sys_heap_init(&gui_heap, gui_heap_mem, CONFIG_UI_GUI_MEM_POOL_SIZE);
	return 0;
}

void * ui_mem_gui_alloc(size_t size)
{
	void *ptr = NULL;

	_assert_in_gui_tid("gui_malloc");

#if defined(CONFIG_UI_GUI_SRAM_POOL_SIZE) && CONFIG_UI_GUI_SRAM_POOL_SIZE > 0
	ptr = sys_heap_alloc(&gui_heap_sram, size);
#endif
	if (ptr == NULL) {
		ptr = sys_heap_alloc(&gui_heap, size);
	}

	return ptr;
}

void * ui_mem_gui_realloc(void * ptr, size_t size)
{
	struct sys_heap *heap = ui_mem_get_heap(ptr);
	void *new_ptr;

	_assert_in_gui_tid("gui_realloc");

	new_ptr = sys_heap_realloc(heap, ptr, size);
	if (new_ptr == NULL) {
		new_ptr = sys_heap_alloc(&gui_heap, size);
		if (new_ptr && ptr) {
			memcpy(new_ptr, ptr, size);
			sys_heap_free(heap, ptr);
		}
	}

	return new_ptr;
}

void ui_mem_gui_free(void * ptr)
{
	_assert_in_gui_tid("gui_free");

	sys_heap_free(ui_mem_get_heap(ptr), ptr);
}

void ui_mem_gui_dump(void)
{
#if defined(CONFIG_UI_GUI_SRAM_POOL_SIZE) && CONFIG_UI_GUI_SRAM_POOL_SIZE > 0
	sys_heap_dump(&gui_heap_sram);
#endif
	sys_heap_dump(&gui_heap);
}

bool ui_mem_is_gui(const void * ptr)
{
	return ((ptr != NULL) && ui_mem_get_heap(ptr)) ? true : false;
}

#endif /* CONFIG_UI_GUI_MEM_POOL_SIZE > 0 */
