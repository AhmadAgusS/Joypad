/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <input_manager.h>
#include <sys/slist.h>
#include <stdint.h>
#include "gui_display.h"

#ifndef GUI_INDEV_H_
#define GUI_INDEV_H_

typedef struct gui_indev {
	uint8_t enabled : 1;
	uint8_t wait_release : 1;

	bool (*read_cb)(struct gui_indev * indev, input_dev_data_t * data);

	gui_disp_t * disp;
	sys_snode_t node;
} gui_indev_t;

int gui_indev_register(gui_indev_t * indev);

gui_indev_t * gui_indev_get_next(gui_indev_t * indev);

#endif /* GUI_INDEV_H_ */
