/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <drivers/input/input_dev.h>
#include <board_cfg.h>
#ifdef CONFIG_ACTS_RING_BUFFER
#include <utils/acts_ringbuf.h>
#endif
#include <lvgl.h>
#include "lvgl_pointer.h"

#include <logging/log.h>
LOG_MODULE_DECLARE(lvgl, CONFIG_LV_LOG_LEVEL);

typedef struct lvgl_pointer_user_data {
	const struct device *input_dev;    /* display device */
	struct k_work input_work;

	bool inited;
} lvgl_pointer_user_data_t;


#ifdef CONFIG_ACTS_RING_BUFFER

#define CONFIG_LVGL_POINTER_COUNT 8

static void _pointer_input_work_handler(struct k_work *work);

static lv_indev_data_t pointer_scan_buf_data[CONFIG_LVGL_POINTER_COUNT];
static ACTS_RINGBUF_DEFINE(pointer_scan_buf, pointer_scan_buf_data, sizeof(pointer_scan_buf_data));

static lvgl_pointer_user_data_t pointer_user_data; 
static lv_indev_drv_t pointer_indev_drv;

static void _pointer_input_work_handler(struct k_work *work)
{
	lvgl_pointer_user_data_t *pointer_data =
			CONTAINER_OF(work, lvgl_pointer_user_data_t, input_work);

	static uint8_t put_cnt;
	static uint8_t drop_cnt;

	struct input_value val;
	input_dev_inquiry(pointer_data->input_dev, &val);

	lv_indev_data_t indev_data;
	indev_data.point.x = val.point.loc_x;
	indev_data.point.y = val.point.loc_y;
	indev_data.state = (val.point.pessure_value == 1) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

	if (acts_ringbuf_put(&pointer_scan_buf, &indev_data, sizeof(indev_data)) != sizeof(indev_data)) {
		drop_cnt++;
	}

	if (++put_cnt >= 64) { /* about 1s for LCD refresh-rate 60 Hz */
		put_cnt = 0;

		if (drop_cnt > 0) {
			LOG_WRN("%u pointer dropped\n", drop_cnt);
			drop_cnt = 0;
		}
	}
} 

void lvgl_pointer_scan(void)
{
	lvgl_pointer_user_data_t *pointer_data = &pointer_user_data;

	if (pointer_data->inited) {
		k_work_submit(&pointer_data->input_work);
	}
}

static void _lvgl_pointer_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
	static lv_indev_data_t prev = {
		.point.x = 0,
		.point.y = 0,
		.state = LV_INDEV_STATE_REL,
	};

	lv_indev_data_t curr;

	if (acts_ringbuf_get(&pointer_scan_buf, &curr, sizeof(curr)) == sizeof(curr)) {
		prev = curr;
	}

	*data = prev;
	data->continue_reading = (acts_ringbuf_is_empty(&pointer_scan_buf) == 0);
}

int lvgl_pointer_init(void)
{
	lvgl_pointer_user_data_t *pointer_data = &pointer_user_data;

	pointer_data->input_dev = device_get_binding(CONFIG_TPKEY_DEV_NAME);
	if (pointer_data->input_dev == NULL) {
		return -ENODEV;
	}

	k_work_init(&pointer_data->input_work, _pointer_input_work_handler);

	lv_indev_drv_init(&pointer_indev_drv);
	pointer_indev_drv.type = LV_INDEV_TYPE_POINTER;
	pointer_indev_drv.read_cb = _lvgl_pointer_read_cb;
	pointer_indev_drv.user_data = pointer_data;

	if (lv_indev_drv_register(&pointer_indev_drv) == NULL) {
		LOG_ERR("Failed to register input device.");
		return -EPERM;
	}

	input_dev_enable(pointer_data->input_dev);
	pointer_data->inited = true;

	return 0;
}

#else /* CONFIG_ACTS_RING_BUFFER */

void lvgl_pointer_scan(void) {}
int lvgl_pointer_init(void) { return -ENODEV; }

#endif /* CONFIG_ACTS_RING_BUFFER */
