/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <ui_manager.h>
#include <view_manager.h>
#include <ui_surface.h>
#include <ui_service.h>
#include "gui.h"

/*******************************************************************************
 * Input glue
 ******************************************************************************/
#define CONFIG_GUI_INPUT_POINTER_MSGQ_COUNT 10

OS_MSGQ_DEFINE(pointer_scan_msgq, sizeof(input_dev_data_t),
		CONFIG_GUI_INPUT_POINTER_MSGQ_COUNT, 4);

static int _gui_pointer_put(const input_dev_data_t *data, void *_indev)
{
	gui_indev_t *indev = _indev;

	if (indev->disp == NULL || indev->enabled == 0) {
		return 0;
	}

	if (os_msgq_put(&pointer_scan_msgq, data, OS_NO_WAIT) != 0) {
		SYS_LOG_WRN("Could put input data into queue");
		return -ENOBUFS;
	}

	return 0;
}

static bool _gui_pointer_read(gui_indev_t *indev, input_dev_data_t *data)
{
	static input_dev_data_t prev = {
		.point.x = 0,
		.point.y = 0,
		.state = INPUT_DEV_STATE_REL,
	};

	input_dev_data_t curr;

	if (os_msgq_get(&pointer_scan_msgq, &curr, OS_NO_WAIT) == 0) {
		prev = curr;
	}

	*data = prev;

	return (os_msgq_num_used_get(&pointer_scan_msgq) > 0) ? true : false;
}

static void _gui_indev_enable(bool en, void *_indev)
{
	gui_indev_t *indev = _indev;

	indev->enabled = en;
}

static void _gui_indev_wait_release(void *_indev)
{
	gui_indev_t *indev = _indev;

	os_msgq_purge(&pointer_scan_msgq);
	indev->wait_release = 1;
}

static const ui_input_gui_callback_t pointer_callback = {
	.enable = _gui_indev_enable,
	.wait_release = _gui_indev_wait_release,
	.put_data = _gui_pointer_put,
};

/*******************************************************************************
 * Draw/Refresh glue
 ******************************************************************************/
static void * _gui_view_init(surface_t *surface)
{
	gui_disp_t * disp;

	disp = gui_disp_create(surface);
	if (disp == NULL) {
		SYS_LOG_ERR("Failed to create display");
	}

	return disp;
}

static void _gui_view_deinit(void * display)
{
	gui_disp_destroy(display);
}

static int _gui_view_resume(void * display, bool full_invalidate)
{
	gui_disp_t *disp = display;

	disp->paused = 0;
	return 0;
}

static int _gui_view_pause(void * display)
{
	gui_disp_t *disp = display;

	disp->paused = 1;
	return 0;
}

static int _gui_view_focus(void * display, bool focus)
{
	if (focus) {
		gui_disp_set_focus(display);
	}

	return 0;
}

static int _gui_view_refresh(void * display, bool full_refresh)
{
	gui_disp_t *disp = display;

	if (disp->paused == 0) {
		/* refresh display now */
		return 0;
	}

	return -EPERM;
}

static void _gui_view_task_handler(void)
{
	gui_task_handler();
}

static const ui_view_gui_callback_t view_callback = {
	.init = _gui_view_init,
	.deinit = _gui_view_deinit,
	.resume = _gui_view_resume,
	.pause = _gui_view_pause,
	.focus = _gui_view_focus,
	.refresh = _gui_view_refresh,
	.task_handler = _gui_view_task_handler,
};

int gui_view_system_init(void)
{
	static gui_indev_t pointer_indev;

	/* register input callback */
	pointer_indev.enabled = 1;
	pointer_indev.read_cb = _gui_pointer_read;
	gui_indev_register(&pointer_indev);
	ui_service_register_gui_input_callback(INPUT_DEV_TYPE_POINTER, &pointer_callback, &pointer_indev);

	/* register view callback */
	ui_service_register_gui_view_callback(UI_VIEW_USER, PIXEL_FORMAT_RGB_565, &view_callback);

	/* initialize dma2d */
	gui_dma2d_init();
	return 0;
}
