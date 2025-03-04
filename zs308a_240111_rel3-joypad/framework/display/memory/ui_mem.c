/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_MODULE_CUSTOMER

#include <os_common_api.h>
#include <string.h>
#include <ui_mem.h>
#ifdef CONFIG_UI_MEMORY_DEBUG
#  include <mem_guard.h>
#endif

LOG_MODULE_REGISTER(ui_mem, LOG_LEVEL_INF);

#if CONFIG_UI_MEM_NUMBER_BLOCKS > 0
extern void * ui_mem_fb_alloc(size_t size);
extern void ui_mem_fb_free(void *ptr);
extern void ui_mem_fb_dump(void);
extern bool ui_mem_is_fb(const void * ptr);
#endif /* CONFIG_UI_MEM_NUMBER_BLOCKS */

#if CONFIG_UI_GUI_MEM_POOL_SIZE > 0
extern int ui_mem_gui_init(void);
extern void * ui_mem_gui_alloc(size_t size);
extern void * ui_mem_gui_realloc(void *ptr, size_t size);
extern void ui_mem_gui_free(void *ptr);
extern void ui_mem_gui_dump(void);
extern bool ui_mem_is_gui(const void * ptr);
#endif /* CONFIG_UI_GUI_MEM_POOL_SIZE */

#if CONFIG_UI_RES_MEM_POOL_SIZE > 0
extern int ui_mem_res_init(void);
extern void * ui_mem_res_alloc(size_t size);
extern void ui_mem_res_free(void *ptr);
extern void ui_mem_res_dump(void);
extern bool ui_mem_is_res(const void * ptr);
#endif /* CONFIG_UI_RES_MEM_POOL_SIZE */

struct ui_mem_func {
	const char * name;
	int (* init_fn)(void);
	void * (* alloc_fn)(size_t);
	void * (* realloc_fn)(void *, size_t);
	void (* free_fn)(void *);
	void (* dump_fn)(void);
	bool (* is_type_fn)(const void *);

#ifdef CONFIG_UI_MEMORY_DEBUG
	struct mem_guard_head *guard;
#endif
};

#ifdef CONFIG_UI_MEMORY_DEBUG
/* exclude the MEM_FB */
static struct mem_guard_head mem_guard[NUM_UI_MEM_TYPES - 1];
#endif

static const struct ui_mem_func mem_table[NUM_UI_MEM_TYPES] = {
#if CONFIG_UI_MEM_NUMBER_BLOCKS > 0
	[MEM_FB] = {
		.name = "FB",
		.alloc_fn = ui_mem_fb_alloc,
		.free_fn = ui_mem_fb_free,
		.dump_fn = ui_mem_fb_dump,
		.is_type_fn = ui_mem_is_fb,
	},
#endif /* CONFIG_UI_MEM_NUMBER_BLOCKS > 0 */

#if CONFIG_UI_GUI_MEM_POOL_SIZE > 0
	[MEM_GUI] = {
		.name = "GUI",
		.init_fn = ui_mem_gui_init,
		.alloc_fn = ui_mem_gui_alloc,
		.realloc_fn = ui_mem_gui_realloc,
		.free_fn = ui_mem_gui_free,
		.dump_fn = ui_mem_gui_dump,
		.is_type_fn = ui_mem_is_gui,
#ifdef CONFIG_UI_MEMORY_DEBUG
		.guard = &mem_guard[0],
#endif
	},
#endif /* CONFIG_UI_GUI_MEM_POOL_SIZE > 0 */

#if CONFIG_UI_RES_MEM_POOL_SIZE > 0
	[MEM_RES] = {
		.name = "RES",
		.init_fn = ui_mem_res_init,
		.alloc_fn = ui_mem_res_alloc,
		.free_fn = ui_mem_res_free,
		.dump_fn = ui_mem_res_dump,
		.is_type_fn = ui_mem_is_res,
#ifdef CONFIG_UI_MEMORY_DEBUG
		.guard = &mem_guard[1],
#endif
	},
#endif /* CONFIG_UI_RES_MEM_POOL_SIZE > 0 */
};

int ui_mem_init(void)
{
	uint8_t i;

	for (i = 0; i < NUM_UI_MEM_TYPES; i++) {
		if (mem_table[i].init_fn) {
			mem_table[i].init_fn();
		}

#ifdef CONFIG_UI_MEMORY_DEBUG
		if (mem_table[i].guard) {
			mem_guard_init(mem_table[i].guard);
		}
#endif /* CONFIG_UI_MEMORY_DEBUG */
	}

	return 0;
}

void * ui_mem_alloc(uint8_t type, size_t size, const void * caller)
{
	void * ptr = NULL;

	if (type >= NUM_UI_MEM_TYPES || mem_table[type].alloc_fn == NULL) {
		return NULL;
	}

#ifdef CONFIG_UI_MEMORY_DEBUG
	if (mem_table[type].guard) {
		ptr = mem_guard_malloc(mem_table[type].guard,
				mem_table[type].alloc_fn, size, caller);
	} else {
		ptr = mem_table[type].alloc_fn(size);
	}
#else
	ptr = mem_table[type].alloc_fn(size);
#endif /* CONFIG_UI_MEMORY_DEBUG */

	if (ptr == NULL) {
		if(type == MEM_FB)
		{
			int try = 3;
			while(try)
			{
				os_sleep(10);
				ptr = mem_table[type].alloc_fn(size);
				if(ptr != NULL)
				{
					break;
				}
				try--;
			}
			if(ptr == NULL)
			{
				SYS_LOG_ERR("UI-%s alloc %u failed 3 times\n", mem_table[type].name, (uint32_t)size);
			}
		}
		else
		{
			SYS_LOG_ERR("UI-%s alloc %u failed\n", mem_table[type].name, (uint32_t)size);
		}
	}

	return ptr;
}

void * ui_mem_realloc(uint8_t type, void * ptr, size_t size, const void * caller)
{
	if (type >= NUM_UI_MEM_TYPES) {
		return ptr;
	}

	if (mem_table[type].realloc_fn == NULL) {
		SYS_LOG_ERR("UI-%s realloc unsupported\n", mem_table[type].name);
		ui_mem_free(type, ptr);
		return NULL;
	}

#ifdef CONFIG_UI_MEMORY_DEBUG
	if (mem_table[type].guard) {
		ptr = mem_guard_realloc(mem_table[type].guard,
				mem_table[type].realloc_fn, ptr, size, caller);
	} else {
		ptr = mem_table[type].realloc_fn(ptr, size);
	}
#else
	ptr = mem_table[type].realloc_fn(ptr, size);
#endif /* CONFIG_UI_MEMORY_DEBUG */

	if (ptr == NULL) {
		SYS_LOG_ERR("UI-%s realloc %u failed\n", mem_table[type].name, (uint32_t)size);
	}

	return ptr;
}

void * ui_mem_calloc(uint8_t type, size_t nmemb, size_t size, const void * caller)
{
	void * ptr = NULL;

	size *= nmemb;

	ptr = ui_mem_alloc(type,  size, caller);
	if (ptr) {
		memset(ptr, 0, size);
	}

	return ptr;
}

void ui_mem_free(uint8_t type, void * ptr)
{
	if (ptr == NULL) {
		return;
	}

	if (type >= NUM_UI_MEM_TYPES || mem_table[type].free_fn == NULL) {
		return;
	}

#ifdef CONFIG_UI_MEMORY_DEBUG
	if (mem_table[type].guard) {
		mem_guard_free(mem_table[type].guard, mem_table[type].free_fn, ptr);
	} else {
		mem_table[type].free_fn(ptr);
	}
#else
	mem_table[type].free_fn(ptr);
#endif /* CONFIG_UI_MEMORY_DEBUG */
}

bool ui_mem_is_type(uint8_t type, const void * ptr)
{
	bool is_type = false;

	if (type < NUM_UI_MEM_TYPES && mem_table[type].is_type_fn) {
		is_type = mem_table[type].is_type_fn(ptr);
	}

	return is_type;
}

void ui_mem_dump(uint8_t type)
{
	if (type >= NUM_UI_MEM_TYPES) {
		return;
	}

	os_printk("UI-%s dump:\n", mem_table[type].name);

#ifdef CONFIG_UI_MEMORY_DEBUG
	if (mem_table[type].guard) {
		mem_guard_dump(mem_table[type].guard);
	} else if (mem_table[type].dump_fn) {
		mem_table[type].dump_fn();
	}
#else
	if (mem_table[type].dump_fn) {
		mem_table[type].dump_fn();
	}
#endif /* CONFIG_UI_MEMORY_DEBUG */
}

void ui_mem_dump_all(void)
{
	uint8_t i;

	for (i = 0; i < NUM_UI_MEM_TYPES; i++) {
		ui_mem_dump(i);
	}
}

void ui_mem_safe_check(uint16_t view_id)
{
#ifdef CONFIG_UI_MEMORY_DEBUG
	uint8_t i;

	os_printk("view_id %d mem check:\n", view_id);

	for (i = 0; i < NUM_UI_MEM_TYPES; i++) {
		if (mem_table[i].guard) {
			os_printk("checking %s:\n", mem_table[i].name);
			mem_guard_leak_check(mem_table[i].guard, view_id);
			os_printk("check end\n");
		}
	}
#endif /* CONFIG_UI_MEMORY_DEBUG */
}
