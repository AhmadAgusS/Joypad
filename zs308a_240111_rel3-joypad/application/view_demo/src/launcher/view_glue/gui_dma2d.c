/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <memory/mem_cache.h>
#include <dma2d_hal.h>
#include "gui_dma2d.h"

static hal_dma2d_handle_t hdma2d;
static bool dma2d_inited = false;

int gui_dma2d_init(void)
{
	int res = hal_dma2d_init(&hdma2d);
	if (res) {
		SYS_LOG_ERR("dma2d initialize failed");
	} else {
		dma2d_inited = true;
	}

	return res;
}

int gui_dma2d_poll(void)
{
	if (dma2d_inited == true) {
		hal_dma2d_poll_transfer(&hdma2d, -1);
		return 0;
	}

	return -ENODEV;
}

int gui_dma2d_fill_color(void * dest, uint32_t dest_cf, int16_t dest_w,
		int16_t fill_w, int16_t fill_h, display_color_t color32)
{
	hal_dma2d_handle_t *dma2d = &hdma2d;
	int res = 0;

	if (dma2d_inited == false) {
		return -ENODEV;
	}

	/* configure the output */
	dma2d->output_cfg.mode = HAL_DMA2D_R2M;
	dma2d->output_cfg.output_offset = dest_w - fill_w;
	dma2d->output_cfg.color_mode = (dest_cf == PIXEL_FORMAT_RGB_565) ?
			HAL_DMA2D_RGB565 : HAL_DMA2D_ARGB8888;
	dma2d->output_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
	res = hal_dma2d_config_output(dma2d);
	if (res < 0) {
		goto out_exit;
	}

	/* start filling */
	res = hal_dma2d_start(dma2d, color32.full, (uint32_t)dest, fill_w, fill_h);
	if (res < 0) {
		goto out_exit;
	}

out_exit:
	return res;
}

int gui_dma2d_fill_mask(void * dest, uint32_t dest_cf, int16_t dest_w,
		const void * mask, uint32_t mask_cf, int16_t mask_w,
		int16_t fill_w, int16_t fill_h, display_color_t color32)
{
	hal_dma2d_handle_t *dma2d = &hdma2d;
	int res = 0;

	if (dma2d_inited == false) {
		return -ENODEV;
	}

	/* configure the output */
	dma2d->output_cfg.mode = HAL_DMA2D_M2M_BLEND;
	dma2d->output_cfg.output_offset = dest_w - fill_w;
	dma2d->output_cfg.color_mode = (dest_cf == PIXEL_FORMAT_RGB_565) ?
			HAL_DMA2D_RGB565 : HAL_DMA2D_ARGB8888;
	dma2d->output_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
	res = hal_dma2d_config_output(dma2d);
	if (res < 0) {
		goto out_exit;
	}

	/* configure the background */
	dma2d->layer_cfg[0].input_offset = dma2d->output_cfg.output_offset;
	dma2d->layer_cfg[0].color_mode = dma2d->output_cfg.color_mode;
	dma2d->layer_cfg[0].rb_swap = dma2d->output_cfg.rb_swap;
	res = hal_dma2d_config_layer(dma2d, HAL_DMA2D_BACKGROUND_LAYER);
	if (res < 0) {
		goto out_exit;
	}

	/* configure the foreground */
	dma2d->layer_cfg[1].input_offset = mask_w - fill_w;
	dma2d->layer_cfg[1].input_alpha = color32.full;
	dma2d->layer_cfg[1].alpha_mode = HAL_DMA2D_COMBINE_ALPHA;
	switch (mask_cf) {
	case PIXEL_FORMAT_A8:
		dma2d->layer_cfg[1].color_mode = HAL_DMA2D_A8;
		break;
	case PIXEL_FORMAT_A4_LE:
		dma2d->layer_cfg[1].color_mode = HAL_DMA2D_A4_LE;
		break;
	case PIXEL_FORMAT_A1_LE:
		dma2d->layer_cfg[1].color_mode = HAL_DMA2D_A1_LE;
		break;
	default:
		return -EINVAL;
	}

	res = hal_dma2d_config_layer(dma2d, HAL_DMA2D_FOREGROUND_LAYER);
	if (res < 0) {
		goto out_exit;
	}

	/* start blending */
	res = hal_dma2d_blending_start(dma2d, (uint32_t)mask,
					(uint32_t)dest, (uint32_t)dest, fill_w, fill_h);
	if (res < 0) {
		goto out_exit;
	}

out_exit:
	return res;
}

int gui_dma2d_copy(void * dest, uint32_t dest_cf, int16_t dest_w,
		const void * src, uint32_t src_cf, int16_t src_w,
		int16_t copy_w, int16_t copy_h)
{
	hal_dma2d_handle_t *dma2d = &hdma2d;
	int res = 0;

	if (dma2d_inited == false) {
		return -ENODEV;
	}

	/* configure the output */
	dma2d->output_cfg.mode = HAL_DMA2D_M2M;
	dma2d->output_cfg.output_offset = dest_w - copy_w;
	dma2d->output_cfg.color_mode = (dest_cf == PIXEL_FORMAT_RGB_565) ?
			HAL_DMA2D_RGB565 : HAL_DMA2D_ARGB8888;
	dma2d->output_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
	res = hal_dma2d_config_output(dma2d);
	if (res < 0) {
		goto out_exit;
	}

	/* configure the foreground */
	dma2d->layer_cfg[1].input_offset = src_w - copy_w;
	dma2d->layer_cfg[1].alpha_mode = HAL_DMA2D_NO_MODIF_ALPHA;
	dma2d->layer_cfg[1].color_mode = (src_cf == PIXEL_FORMAT_RGB_565) ?
			HAL_DMA2D_RGB565 : HAL_DMA2D_ARGB8888;
	dma2d->layer_cfg[1].rb_swap = HAL_DMA2D_RB_REGULAR;
	res = hal_dma2d_config_layer(dma2d, HAL_DMA2D_FOREGROUND_LAYER);
	if (res < 0) {
		goto out_exit;
	}

	res = hal_dma2d_start(dma2d, (uint32_t)src, (uint32_t)dest,
						copy_w, copy_h);
	if (res < 0) {
		goto out_exit;
	}

out_exit:
	return res;
}

int gui_dma2d_blend(void * dest, uint32_t dest_cf, int16_t dest_w,
		const void * src, uint32_t src_cf, int16_t src_w,
		int16_t copy_w, int16_t copy_h, uint8_t opa)
{
	hal_dma2d_handle_t *dma2d = &hdma2d;
	int res = 0;

	if (dma2d_inited == false) {
		return -ENODEV;
	}

	/* configure the output */
	dma2d->output_cfg.mode = HAL_DMA2D_M2M_BLEND;
	dma2d->output_cfg.output_offset = dest_w - copy_w;
	dma2d->output_cfg.color_mode = (dest_cf == PIXEL_FORMAT_RGB_565) ?
			HAL_DMA2D_RGB565 : HAL_DMA2D_ARGB8888;
	dma2d->output_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
	res = hal_dma2d_config_output(dma2d);
	if (res < 0) {
		goto out_exit;
	}

	/* configure the background */
	dma2d->layer_cfg[0].input_offset = dma2d->output_cfg.output_offset;
	dma2d->layer_cfg[0].color_mode = dma2d->output_cfg.color_mode;
	dma2d->layer_cfg[0].rb_swap = dma2d->output_cfg.rb_swap;
	res = hal_dma2d_config_layer(dma2d, HAL_DMA2D_BACKGROUND_LAYER);
	if (res < 0) {
		goto out_exit;
	}

	/* configure the foreground */
	dma2d->layer_cfg[1].input_offset = src_w - copy_w;
	dma2d->layer_cfg[1].input_alpha = (uint32_t)opa << 24;
	dma2d->layer_cfg[1].alpha_mode = HAL_DMA2D_COMBINE_ALPHA;
	dma2d->layer_cfg[1].rb_swap = HAL_DMA2D_RB_REGULAR;
	if (src_cf == PIXEL_FORMAT_ARGB_8888) {
		dma2d->layer_cfg[1].color_mode = HAL_DMA2D_ARGB8888;
	} else if (src_cf == PIXEL_FORMAT_ARGB_6666) {
		dma2d->layer_cfg[1].color_mode = HAL_DMA2D_ARGB6666;
	} else {
		dma2d->layer_cfg[1].rb_swap = PIXEL_FORMAT_RGB_565;
	}

	res = hal_dma2d_config_layer(dma2d, HAL_DMA2D_FOREGROUND_LAYER);
	if (res < 0) {
		goto out_exit;
	}

	res = hal_dma2d_blending_start(dma2d, (uint32_t)src,
					(uint32_t)dest, (uint32_t)dest, copy_w, copy_h);
	if (res < 0) {
		goto out_exit;
	}

out_exit:
	return res;
}
