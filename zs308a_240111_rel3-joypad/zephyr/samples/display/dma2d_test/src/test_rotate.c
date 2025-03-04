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

#define SRC_USE_ARGB_8888 0

extern const uint8_t compass_360x360_map[];
extern const uint8_t compass_454x454_map[];
extern const uint8_t compass_454x454_rot180_map[];

#define COMPASS_W 454
#define COMPASS_H 454

#if COMPASS_W == 454
#  define COMPASS_HOLE 300
#  define COMPASS_MAP compass_454x454_map
#  define COMPASS_ROT180_MAP compass_454x454_rot180_map
#else
#  define COMPASS_HOLE 240
#  define COMPASS_MAP compass_360x360_map
#  define COMPASS_ROT180_MAP compass_360x360_map
#endif

static K_SEM_DEFINE(display_start_sem, 0, 1);
static K_SEM_DEFINE(display_complete_sem, 0, 1);

static __aligned(4) uint8_t src_psram[COMPASS_W * COMPASS_H * 4];
#if SRC_USE_ARGB_8888
static __aligned(4) uint8_t dst_psram[COMPASS_W * COMPASS_H * 4];
#else
static __aligned(4) __in_section_unique(ram.noinit) uint8_t dst_sram[2 * COMPASS_W * (COMPASS_H + 9) / 10];
#endif

static display_buffer_t src_buf = {
	.desc = {
#if SRC_USE_ARGB_8888
		.pixel_format = PIXEL_FORMAT_ARGB_8888,
#else
		.pixel_format = PIXEL_FORMAT_RGB_565,
#endif
		.pitch = COMPASS_W,
		.width = COMPASS_W,
		.height = COMPASS_H,
	},

	.addr = (uint32_t)src_psram,
};

static display_buffer_t dst_buf = {
	.desc = {
#if SRC_USE_ARGB_8888
		.pixel_format = PIXEL_FORMAT_ARGB_8888,
		.height = COMPASS_H,
#else
		.pixel_format = PIXEL_FORMAT_RGB_565,
		.height = sizeof(dst_sram) / (COMPASS_W * 2),
#endif
		.width = COMPASS_W,
		.pitch = COMPASS_W,
	},

#if SRC_USE_ARGB_8888
	.addr = (uint32_t)dst_psram,
#else
	.addr = (uint32_t)dst_sram,
#endif
};

static display_layer_t layer = {
	.buffer = &dst_buf,
	.frame = { .x = 0, .y = 0, .w = COMPASS_W, .h = COMPASS_H, },
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

void test_rotate(void)
{
	static hal_dma2d_handle_t dma2d;

	display_color_t test_color[] = {
		{ .full = 0x80000000, }, /* black */
		{ .full = 0x80FF0000, }, /* red */
		{ .full = 0x8000FF00, }, /* green */
		{ .full = 0x800000FF, }, /* blue */
		{ .full = 0x80FFFFFF, }, /* white */
		{ .full = 0x80505050, }, /* grey */
	};
	unsigned int frame_cnt = 0;
	unsigned int color_cnt = 0;

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

	dma2d.rotation_cfg.input_offset = 0;
	dma2d.rotation_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
#if SRC_USE_ARGB_8888
	dma2d.rotation_cfg.color_mode = HAL_DMA2D_ARGB8888;
#else
	dma2d.rotation_cfg.color_mode = HAL_DMA2D_RGB565;
#endif
	dma2d.rotation_cfg.outer_diameter = COMPASS_W - 1;
	dma2d.rotation_cfg.inner_diameter = COMPASS_HOLE;
	dma2d.rotation_cfg.angle = 0;
	dma2d.rotation_cfg.fill_enable = 1;
	dma2d.rotation_cfg.fill_color = 0;
	hal_dma2d_config_rotation(&dma2d);

	dma2d.output_cfg.mode = HAL_DMA2D_M2M_ROTATE;
	dma2d.output_cfg.output_offset = 0;
	dma2d.output_cfg.rb_swap = HAL_DMA2D_RB_REGULAR;
#if SRC_USE_ARGB_8888
	dma2d.output_cfg.color_mode = HAL_DMA2D_ARGB8888;
#else
	dma2d.output_cfg.color_mode = HAL_DMA2D_RGB565;
#endif
	hal_dma2d_config_output(&dma2d);

	uint32_t max_cycles = 0;
	uint32_t display_cycles = 0;
	uint32_t draw_cycles = 0;
	uint16_t max_angle = 0;
	uint16_t angle = 0;;

	while (1) {
		if ((frame_cnt % 2) == 0) {
			dma2d.rotation_cfg.fill_color = test_color[color_cnt].full;
			if (++color_cnt >= ARRAY_SIZE(test_color))
				color_cnt = 0;
		}

		angle += 150;
		if (angle >= 3600)
			angle = 0;

		printk("angle=%d\n", angle);

		/* FIXME: initial different src image for 454x454 due to hardware bug */
		if (angle > 900 && angle < 2700) {
			display_buffer_fill_image(&src_buf, COMPASS_ROT180_MAP);
			dma2d.rotation_cfg.angle = (angle >= 1800) ? (angle - 1800) : (angle + 1800);
		} else {
			display_buffer_fill_image(&src_buf, COMPASS_MAP);
			dma2d.rotation_cfg.angle = angle;
		}

		hal_dma2d_config_rotation(&dma2d);

		uint16_t row_start = 0;
		uint16_t row_count = dst_buf.desc.height;
		uint16_t tmp_dst_height = dst_buf.desc.height;
		uint32_t rotate_cycles = 0;
		uint32_t start_cycle;

		while (row_start < src_buf.desc.height) {
			if (row_count > src_buf.desc.height - row_start) {
				row_count = src_buf.desc.height - row_start;
				dst_buf.desc.height = row_count;
			}

			start_cycle = k_cycle_get_32();

			hal_dma2d_rotation_start(&dma2d, src_buf.addr,
					dst_buf.addr, row_start, row_count);
			hal_dma2d_poll_transfer(&dma2d, -1);

			rotate_cycles += k_cycle_get_32() - start_cycle;

			if (row_start == 0) {
				k_sem_reset(&display_start_sem);
				k_sem_take(&display_start_sem, K_FOREVER);
			}

			start_cycle = k_cycle_get_32();

			layer.frame.y = row_start;
			layer.frame.h = row_count;
			display_engine_compose(de_dev, de_inst, NULL, &layer, 1, false);
			k_sem_take(&display_complete_sem, K_FOREVER);

			display_cycles += k_cycle_get_32() - start_cycle;

			row_start += row_count;
		}

		dst_buf.desc.height = tmp_dst_height;

		if (rotate_cycles > max_cycles) {
			max_cycles = rotate_cycles;
			max_angle = angle;
		}

		draw_cycles += rotate_cycles;

		frame_cnt++;
		if ((frame_cnt % 24) == 0) {
			printk("draw avr %u us, max %u us (angle=%u); display %u us\n",
					k_cyc_to_us_floor32(draw_cycles / 24),
					k_cyc_to_us_floor32(max_cycles), max_angle,
					k_cyc_to_us_floor32(display_cycles / 24));
			draw_cycles = 0;
			display_cycles = 0;
		}
	}
}
