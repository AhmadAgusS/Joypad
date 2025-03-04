/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(te_test, LOG_LEVEL_INF);

#include <string.h>
#include <zephyr.h>
#include <spicache.h>
#include <device.h>
#include <drivers/display.h>
#include <drivers/display/display_engine.h>
#include <board_cfg.h>

static K_SEM_DEFINE(display_start_sem, 0, 1);
static K_SEM_DEFINE(display_complete_sem, 0, 1);

#define X_RES       (CONFIG_PANEL_TIMING_HACTIVE)
#define Y_RES       (CONFIG_PANEL_TIMING_VACTIVE)
#define VDB_SIZE    (X_RES * Y_RES * 2)
#define NUM_LAYERS  (1)

static __attribute__((aligned(4))) char bg_vdb[VDB_SIZE];
static __attribute__((aligned(4))) char fg_vdb[VDB_SIZE];

static display_buffer_t bufs[] = {
	{
		.desc = {
			.pixel_format = PIXEL_FORMAT_RGB_565,
			.width = X_RES,
			.height = Y_RES,
			.pitch = X_RES,
		},
		.addr = (uint32_t)bg_vdb,
	},

	{
		.desc = {
			.pixel_format = PIXEL_FORMAT_RGB_565,
			.width = X_RES,
			.height = Y_RES,
			.pitch = X_RES,
		},
		.addr = (uint32_t)fg_vdb,
	},
};

static display_layer_t layers[] = {
	{
		.buffer = &bufs[0],
		.frame = { .x = 0, .y = 0, .w = X_RES, .h = Y_RES, },
		.blending = DISPLAY_BLENDING_NONE,
	},

#if NUM_LAYERS > 1
	{
		.buffer = &bufs[1],
		.frame = { .x = 0, .y = 0, .w = X_RES, .h = Y_RES, },
		.blending = DISPLAY_BLENDING_COVERAGE,
		.color = { .a = 128, },
	},
#endif
};

static void display_vsync_handler(const struct display_callback *callback, uint32_t timestamp)
{
	static uint32_t prev_timestamp = 0;
	static uint32_t count = 0;

	if (++count == 256) {
		LOG_INF("vsync %u us\n", k_cyc_to_us_near32(timestamp - prev_timestamp));
		count = 0;
	}

	prev_timestamp = timestamp;

	k_sem_give(&display_start_sem);
}

static void display_complete_handler(const struct display_callback *callback)
{
	k_sem_give(&display_complete_sem);
}

static void de_complete_handler(int status, uint16_t cmd_seq, void *user_data)
{
	k_sem_give(&display_complete_sem);
}

static void display_post_normal(const struct device *display_dev,
		const struct device *de_dev, int de_inst)
{
	display_buffer_t *bg_buf = &bufs[0];
	display_buffer_t *fg_buf = &bufs[NUM_LAYERS - 1];
	const display_color_t test_color[] = {
		{ .full = 0x80000000, }, /* black */
		{ .full = 0x80FF0000, }, /* red */
		{ .full = 0x8000FF00, }, /* green */
		{ .full = 0x800000FF, }, /* blue */
		{ .full = 0x80FFFFFF, }, /* white */
	};
	unsigned int frame_cnt = 0;
	unsigned int color_cnt = 0;
	uint32_t start_cycle;
	uint32_t display_cycles = 0;

	/* register display te/vsync and complete callback */
	const struct display_callback callback  = {
		.vsync = display_vsync_handler,
		.complete = display_complete_handler,
	};
	display_register_callback(display_dev, &callback);

	/* initial background color */
	display_buffer_fill_color(bg_buf, test_color[0]);

	while (1) {
		if ((frame_cnt % 50) == 0) {
			/* change foreground color */
			display_buffer_fill_color(fg_buf, test_color[color_cnt]);
			if (++color_cnt >= ARRAY_SIZE(test_color))
				color_cnt = 0;
		}

		/* wait te signal */
		k_sem_reset(&display_start_sem);
		k_sem_take(&display_start_sem, K_FOREVER);

		start_cycle = k_cycle_get_32();

		/* post current frame */
		if (NUM_LAYERS > 1) {
			display_engine_compose(de_dev, de_inst, NULL, layers, ARRAY_SIZE(layers), false);
		} else {
			display_write(display_dev, 0, 0, &fg_buf->desc, (void *)fg_buf->addr);
		}

		/* wait current frame completed */
		k_sem_take(&display_complete_sem, K_FOREVER);

		display_cycles += k_cycle_get_32() - start_cycle;

		if (++frame_cnt % 100 == 0) {
			LOG_INF("frame %u, time %u us\n", frame_cnt, k_cyc_to_us_near32(display_cycles) / 100);
			display_cycles = 0;
		}
	}
}

static void display_post_sync(const struct device *display_dev,
		const struct device *de_dev, int de_inst)
{
	const display_color_t test_color[] = {
		{ .full = 0x80000000, }, /* black */
		{ .full = 0x80FF0000, }, /* red */
		{ .full = 0x8000FF00, }, /* green */
		{ .full = 0x800000FF, }, /* blue */
		{ .full = 0x80FFFFFF, }, /* white */
	};
	unsigned int frame_cnt = 0;
	unsigned int color_cnt = 0;
	bool first_frame = true;
	int fg_idx = 0;

	/* register display te/vsync callback */
	const struct display_callback callback  = {
		.vsync = display_vsync_handler,
		.complete = NULL,
	};
	display_register_callback(display_dev, &callback);

	/* register de post complete callback */
	display_engine_register_callback(de_dev, de_inst, de_complete_handler, NULL);

	while (1) {
		if ((frame_cnt % 50) == 0) {
			if (++color_cnt >= ARRAY_SIZE(test_color))
				color_cnt = 0;
		}

		/* change color */
		display_buffer_fill_color(&bufs[fg_idx], test_color[color_cnt]);

		/* wait te signal */
		k_sem_reset(&display_start_sem);
		k_sem_take(&display_start_sem, K_FOREVER);

		/* post current frame */
		layers[0].buffer = &bufs[fg_idx];
		display_engine_compose(de_dev, de_inst, NULL, layers, 1, false);

		/* wait previous frame completed */
		if (first_frame) {
			first_frame = false;
		} else {
			k_sem_take(&display_complete_sem, K_FOREVER);
		}

		fg_idx ^= 1;
		frame_cnt++;
	}
}

static bool display_has_ram(const struct device *display_dev)
{
	struct display_capabilities cap;

	display_get_capabilities(display_dev, &cap);

	return (cap.screen_info & SCREEN_INFO_ZERO_BUFFER) ? false : true;
}

int main(void)
{
	const struct device *display_dev = device_get_binding(CONFIG_LCD_DISPLAY_DEV_NAME);
	if (display_dev == NULL) {
		LOG_ERR(CONFIG_LCD_DISPLAY_DEV_NAME " device not found\n");
		return -ENODEV;
	}

	const struct device *de_dev = device_get_binding(CONFIG_DISPLAY_ENGINE_DEV_NAME);
	if (de_dev == NULL) {
		LOG_ERR("DE device not found\n");
		return -ENODEV;
	}

	int de_inst = display_engine_open(de_dev, DISPLAY_ENGINE_FLAG_POST);
	if (de_inst < 0) {
		LOG_ERR("DE device open failed\n");
		return -ENODEV;
	}

	if (display_has_ram(display_dev)) {
		display_post_normal(display_dev, de_dev, de_inst);
	} else {
		display_post_sync(display_dev, de_dev, de_inst);
	}

	return 0;
}
