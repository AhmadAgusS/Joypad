/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <sys/slist.h>
#include <ui_surface.h>

#ifndef GUI_DISPLAY_H_
#define GUI_DISPLAY_H_

typedef struct gui_disp {
	uint8_t paused : 1;

	/* draw */
	void (*draw_cb)(struct gui_disp *disp);

	surface_t * surface;
	void * user_data;

	sys_snode_t node;
} gui_disp_t;

gui_disp_t * gui_disp_create(surface_t * surface);

void gui_disp_destroy(gui_disp_t * disp);

int gui_disp_set_focus(gui_disp_t * disp);

gui_disp_t * gui_disp_get_next(gui_disp_t * disp);

#endif /* GUI_DISPLAY_H_ */
