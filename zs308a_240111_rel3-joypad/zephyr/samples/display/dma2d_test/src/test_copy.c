/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_DECLARE(sample, LOG_LEVEL_INF);

#include <string.h>
#include <zephyr.h>
#include <device.h>
#include <dma2d_hal.h>
#include <drivers/display/display_engine.h>

static K_SEM_DEFINE(display_start_sem, 0, 1);
static K_SEM_DEFINE(display_complete_sem, 0, 1);

#if 1
static __attribute__((aligned(4))) uint8_t dst_psram[360 * 360 * 2];
static __attribute__((aligned(4))) uint8_t src_psram[360 * 360 * 4];
#else
static uint8_t *const dst_psram = (uint8_t *)(CONFIG_PSRAM_BASE_ADDRESS + 0x100000);
static uint8_t *const src_psram = (uint8_t *)(CONFIG_PSRAM_BASE_ADDRESS);
#endif

static display_buffer_t src_buf = {
	.desc = {
		.pixel_format = PIXEL_FORMAT_ARGB_8888,
		.pitch = 360,
		.width = 360,
		.height = 360,
	},

	.addr = (uint32_t)src_psram,
};

static display_buffer_t dst_buf = {
	.desc = {
		.pixel_format = PIXEL_FORMAT_RGB_565,
		.pitch = 360,
		.width = 360,
		.height = 360,
	},

	.addr = (uint32_t)dst_psram,
};

static display_layer_t layer = {
	.buffer = &dst_buf,
	.frame = { .x = 0, .y = 0, .w = 360, .h = 360, },
	.blending = DISPLAY_BLENDING_NONE,
};

static void display_vsync(const struct display_callback *callback,
			  uint32_t timestamp)
{
	k_sem_give(&display_start_sem);
}

static void display_complete(const struct display_callback *callback)
{
	k_sem_give(&display_complete_sem);
}

static const struct display_callback display_callback = {
	.vsync = display_vsync,
	.complete = display_complete,
};

void test_copy(void)
{
	static hal_dma2d_handle_t dma2d;

	display_color_t test_color[] = {
		{ .full = 0x00000000, }, /* black */
		{ .full = 0x00FF0000, }, /* red */
		{ .full = 0x0000FF00, }, /* green */
		{ .full = 0x000000FF, }, /* blue */
		{ .full = 0x00FFFFFF, }, /* white */
		{ .full = 0x00505050, }, /* grey */
	};
	unsigned int color_cnt = 0;
	unsigned int frame_cnt = 0;

	const struct device *display_dev = device_get_binding(CONFIG_LCD_DISPLAY_DEV_NAME);
	if (!display_dev) {
		printk("%s: display " CONFIG_LCD_DISPLAY_DEV_NAME " not found\n", __func__);
		return;
	}

	display_register_callback(display_dev, &display_callback);

	const struct device *de_dev = device_get_binding(CONFIG_DISPLAY_ENGINE_DEV_NAME);
	if (!de_dev) {
		printk("%s: DE not found\n", __func__);
		return;
	}

	int de_inst = display_engine_open(de_dev, 0);
	if (de_inst < 0) {
		printk("%s: de open failed\n", __func__);
		return;
	}

	if (hal_dma2d_init(&dma2d)) {
		printk("hal_dma2d_init failed\n");
		return;
	}

	dma2d.layer_cfg[1].input_offset = 0;
	dma2d.layer_cfg[1].rb_swap = HAL_DMA2D_RB_REGULAR;
	dma2d.layer_cfg[1].color_mode = HAL_DMA2D_ARGB8888;
	dma2d.layer_cfg[1].alpha_mode = HAL_DMA2D_NO_MODIF_ALPHA;
	hal_dma2d_config_layer(&dma2d, 1);

	dma2d.output_cfg.mode = HAL_DMA2D_M2M;
	dma2d.output_cfg.output_offset = 0;
	dma2d.output_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
	dma2d.output_cfg.color_mode = HAL_DMA2D_RGB565;
	hal_dma2d_config_output(&dma2d);

	while (1) {
		if (frame_cnt++ % 2 == 0) {
			display_buffer_fill_color(&src_buf, test_color[color_cnt]);

			hal_dma2d_start(&dma2d, src_buf.addr, dst_buf.addr,
					src_buf.desc.width, src_buf.desc.height);
			hal_dma2d_poll_transfer(&dma2d, -1);

			if (++color_cnt >= ARRAY_SIZE(test_color))
				color_cnt = 0;
		}

		k_sem_reset(&display_start_sem);
		k_sem_take(&display_start_sem, K_FOREVER);

		display_engine_compose(de_dev, de_inst, NULL, &layer, 1, false);
		k_sem_take(&display_complete_sem, K_FOREVER);
	}
}
