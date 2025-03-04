/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/util.h>
#include <zephyr/types.h>
#include <drivers/display.h>

#ifndef GUI_DMA2D_H_
#define GUI_DMA2D_H_

int gui_dma2d_init(void);

int gui_dma2d_poll(void);

int gui_dma2d_fill_color(void * dest, uint32_t dest_cf, int16_t dest_w,
		int16_t fill_w, int16_t fill_h, display_color_t color32);

int gui_dma2d_fill_mask(void * dest, uint32_t dest_cf, int16_t dest_w,
		const void * mask, uint32_t mask_cf, int16_t mask_w,
		int16_t fill_w, int16_t fill_h, display_color_t color32);

int gui_dma2d_copy(void * dest, uint32_t dest_cf, int16_t dest_w,
		const void * src, uint32_t src_cf, int16_t src_w,
		int16_t copy_w, int16_t copy_h);

int gui_dma2d_blend(void * dest, uint32_t dest_cf, int16_t dest_w,
		const void * src, uint32_t src_cf, int16_t src_w,
		int16_t copy_w, int16_t copy_h, uint8_t opa);

#endif /* GUI_DMA2D_H_ */
