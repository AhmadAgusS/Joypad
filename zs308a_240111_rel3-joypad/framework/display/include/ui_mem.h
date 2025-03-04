/*
 * Copyright (c) 2018 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file ui memory interface
 */

#ifndef FRAMEWORK_DISPLAY_INCLUDE_UI_MEM_H_
#define FRAMEWORK_DISPLAY_INCLUDE_UI_MEM_H_

/**
 * @defgroup view_cache_apis View Cache APIs
 * @ingroup system_apis
 * @{
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_LV_MEM_SIZE_KILOBYTES
#  define CONFIG_UI_GUI_MEM_POOL_SIZE (CONFIG_LV_MEM_SIZE_KILOBYTES * 1024)
#else
#  define CONFIG_UI_GUI_MEM_POOL_SIZE (0)
#endif

/**
 * @brief UI Memory Types
 *
 * UI memory type enumeration
 */
enum {
	MEM_FB = 0,
	MEM_GUI,
	MEM_RES,

	NUM_UI_MEM_TYPES,
};

/**
 * @brief Initialize the UI memory
 *
 * @retval 0 on success else negative code.
 */
int ui_mem_init(void);

/**
 * @brief Alloc UI memory
 *
 * @param type UI memory type
 * @param size Allocation size in bytes
 * @param caller Caller address for debug; can be NULL
 *
 * @retval pointer to the allocation memory.
 */
void * ui_mem_alloc(uint8_t type, size_t size, const void * caller);

/**
 * @brief Realloc UI memory
 *
 * @param type UI memory type
 * @param ptr Pointer to original memory
 * @param size Allocation size in bytes
 * @param caller Caller address for debug; can be NULL
 *
 * @retval pointer to the allocation memory.
 */
void * ui_mem_realloc(uint8_t type, void * ptr, size_t size, const void * caller);

/**
 * @brief Calloc UI memory
 *
 * @param type UI memory type
 * @param nmemb number of members
 * @param size size of member
 * @param caller Caller address for debug; can be NULL
 *
 * @retval pointer to the allocation memory.
 */
void * ui_mem_calloc(uint8_t type, size_t nmemb, size_t size, const void * caller);

/**
 * @brief Free UI memory
 *
 * @param type UI memory type
 * @param ptr Pointer to the allocated memory
 *
 * @retval N/A
 */
void ui_mem_free(uint8_t type, void *ptr);

/**
 * @brief Query if memory is allocated from a specific pool.
 *
 * @param type UI memory type
 * @param ptr Pointer to an allocated memory
 *
 * @retval true if of the specific type else false
 */
bool ui_mem_is_type(uint8_t type, const void * ptr);

/**
 * @brief Dump UI memory allocation detail.
 *
 * @param type UI memory type
 *
 * @retval N/A
 */
void ui_mem_dump(uint8_t type);

/**
 * @brief Dump All UI memory allocation detail.
 *
 * @param type UI memory type
 *
 * @retval N/A
 */
void ui_mem_dump_all(void);

/**
 * @brief ui memory safe check
 *
 * @param view_id id of view witch want to check
 *
 * @retval N/A
 */
void ui_mem_safe_check(uint16_t view_id);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* FRAMEWORK_DISPLAY_INCLUDE_UI_MEM_H_ */
