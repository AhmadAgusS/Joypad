/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_GUI_TEXT_CACHE_H_
#define ZEPHYR_LIB_GUI_TEXT_CACHE_H_

#include <stdlib.h>
#include <ui_mem.h>

#ifdef __cplusplus
extern "C" {
#endif

#define gui_text_cache_malloc(size) ui_mem_alloc(MEM_RES, size, __func__)

#define gui_text_cache_free(ptr) ui_mem_free(MEM_RES, ptr)

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_LIB_GUI_TEXT_CACHE_H_ */
