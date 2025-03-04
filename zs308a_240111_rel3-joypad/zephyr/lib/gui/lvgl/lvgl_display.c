/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "lvgl_display.h"
#include "lvgl_pointer.h"

#include <board_cfg.h>
#if defined(CONFIG_LV_USE_GPU) && defined(CONFIG_ACTIONS_FRAMEWORK)
#  include <lvgl/lvgl_gpu.h>
#endif

#include <logging/log.h>
LOG_MODULE_DECLARE(lvgl, CONFIG_LV_LOG_LEVEL);

#define NBR_PIXELS_IN_BUFFER CONFIG_LVGL_VDB_SIZE
#define BUFFER_SIZE (CONFIG_LVGL_VDB_SIZE * CONFIG_LV_COLOR_DEPTH / 8)

/* NOTE: depending on chosen color depth buffer may be accessed using uint8_t *,
 * uint16_t * or uint32_t *, therefore buffer needs to be aligned accordingly to
 * prevent unaligned memory accesses.
 */
static uint8_t vdb_buf0[BUFFER_SIZE] __aligned(4) __in_section_unique(lvgl.noinit.vdb.0);
#ifdef CONFIG_LVGL_DOUBLE_VDB
static uint8_t vdb_buf1[BUFFER_SIZE] __aligned(4) __in_section_unique(lvgl.noinit.vdb.1);
#endif

static lv_disp_drv_t lv_disp_drv;
static lv_disp_draw_buf_t lv_disp_buf;
static lvgl_disp_user_data_t disp_user_data;

static void lvgl_set_rendering_buffers(lv_disp_drv_t *disp_drv)
{
	struct display_capabilities cap;
	const struct device *display_dev = lvgl_get_display_dev(disp_drv);

	display_get_capabilities(display_dev, &cap);

	disp_drv->hor_res = cap.x_resolution;
	disp_drv->ver_res = cap.y_resolution;
	disp_drv->draw_buf = &lv_disp_buf;

#ifdef CONFIG_LVGL_DOUBLE_VDB
	lv_disp_draw_buf_init(disp_drv->draw_buf, &vdb_buf0, &vdb_buf1, NBR_PIXELS_IN_BUFFER);
#else
	lv_disp_draw_buf_init(disp_drv->draw_buf, &vdb_buf0, NULL, NBR_PIXELS_IN_BUFFER);
#endif /* CONFIG_LVGL_DOUBLE_VDB  */
}

static void _display_flush(lvgl_disp_user_data_t *disp_data,
		const lv_area_t *area, const lv_color_t *color_p)
{
	struct display_buffer_descriptor desc;

	desc.width = lv_area_get_width(area);
	desc.height = lv_area_get_height(area);
	desc.pitch = desc.width;
#ifdef CONFIG_LV_COLOR_DEPTH_16
	desc.pixel_format = PIXEL_FORMAT_RGB_565;
	desc.buf_size = desc.pitch * desc.height * 2;
#else
	desc.pixel_format = PIXEL_FORMAT_ARGB_8888;
	desc.buf_size = desc.pitch * desc.height * 4;
#endif

	display_write(disp_data->disp_dev, area->x1, area->y1, &desc, (void *)color_p);
}

static void _display_vsync_cb(const struct display_callback *callback, uint32_t timestamp)
{
	lvgl_disp_user_data_t *disp_data =
			CONTAINER_OF(callback, lvgl_disp_user_data_t, callback);

	if (disp_data->flush_data) {
		_display_flush(disp_data, &disp_data->flush_area, disp_data->flush_data);
		disp_data->flush_data = NULL;
	}

	lvgl_pointer_scan();
}

static void _display_complete_cb(const struct display_callback *callback)
{
	lvgl_disp_user_data_t *disp_data =
			CONTAINER_OF(callback, lvgl_disp_user_data_t, callback);

	disp_data->flushing = false;
	k_sem_give(&disp_data->complete_sem);
}

static void _display_pm_notify_cb(const struct display_callback *callback, uint32_t pm_action)
{
	lvgl_disp_user_data_t *disp_data =
			CONTAINER_OF(callback, lvgl_disp_user_data_t, callback);

	switch (pm_action) {
	case PM_DEVICE_ACTION_EARLY_SUSPEND:
		disp_data->active = false;
		/* wait the whole frame flushed */
		while (/*!disp_data->new_frame || */disp_data->flush_data || disp_data->flushing) {
			k_msleep(2);
		}
		break;
	case PM_DEVICE_ACTION_LATE_RESUME:
		disp_data->active = true;
		break;
	default:
		break;
	}
}

static void lvgl_flush_cb(lv_disp_drv_t *disp_drv,
		const lv_area_t *area, lv_color_t *color_p)
{
	lvgl_disp_user_data_t *disp_data = disp_drv->user_data;

	/* make sure gpu completed */
	if (disp_drv->gpu_wait_cb) disp_drv->gpu_wait_cb(disp_drv);

	/* make sure the frame is complete */
	if (/*disp_data->new_frame && */!disp_data->active) {
		lv_disp_flush_ready(disp_drv);
		goto out_exit;
	}

	disp_data->flushing = true;

	if (disp_data->new_frame) {
		memcpy(&disp_data->flush_area, area, sizeof(*area));
		disp_data->flush_data = color_p;
	} else {
		_display_flush(disp_data, area, color_p);
	}

out_exit:
	disp_data->new_frame = lv_disp_flush_is_last(disp_drv);
}

static void lvgl_wait_cb(lv_disp_drv_t *disp_drv)
{
	lvgl_disp_user_data_t *disp_data = disp_drv->user_data;

	if (k_sem_take(&disp_data->complete_sem, K_MSEC(1000))) {
		LOG_ERR("flush timeout");
	}

	lv_disp_flush_ready(disp_drv);
}

static void lvgl_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
	area->x1 &= ~0x1;
	area->x2 |= 0x1;
	area->y1 &= ~0x1;
	area->y2 |= 0x1;
}

static void set_lvgl_rendering_cb(lv_disp_drv_t *disp_drv)
{
	disp_drv->wait_cb = lvgl_wait_cb;
	disp_drv->rounder_cb = lvgl_rounder_cb;
	disp_drv->flush_cb = lvgl_flush_cb;

#if defined(CONFIG_LV_USE_GPU) && defined(CONFIG_ACTIONS_FRAMEWORK)
	lvgl_gpu_set_render_cb(disp_drv);
	lvgl_gpu_set_sw_render_cb(disp_drv);
#endif
}

int lvgl_display_init(void)
{
	lvgl_disp_user_data_t *disp_data = &disp_user_data;

	disp_data->disp_dev = device_get_binding(CONFIG_LCD_DISPLAY_DEV_NAME);
	if (disp_data->disp_dev == NULL) {
		LV_LOG_ERROR("Display device not found.");
		return -ENODEV;
	}

	k_sem_init(&disp_data->complete_sem, 0, 1);

	disp_data->active = true;
	disp_data->new_frame = true;
	disp_data->callback.vsync = _display_vsync_cb;
	disp_data->callback.complete = _display_complete_cb;
	disp_data->callback.pm_notify = _display_pm_notify_cb,
	display_register_callback(disp_data->disp_dev, &disp_data->callback);
	//display_blanking_off(disp_data->disp_dev);

	lv_disp_drv_init(&lv_disp_drv);
	lv_disp_drv.user_data = (void *) disp_data;

	lvgl_set_rendering_buffers(&lv_disp_drv);
	set_lvgl_rendering_cb(&lv_disp_drv);

	if (lv_disp_drv_register(&lv_disp_drv) == NULL) {
		LV_LOG_ERROR("Failed to register display device.");
		return -EPERM;
	}

	return 0;
}
