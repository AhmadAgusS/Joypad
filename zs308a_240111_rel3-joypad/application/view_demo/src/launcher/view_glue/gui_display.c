/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <mem_manager.h>
#include <sys/util.h>
#include "gui_display.h"
#include "gui_indev.h"

static sys_slist_t disp_list = SYS_SLIST_STATIC_INIT(&disp_list);
static gui_disp_t *act_disp = NULL;

static void surface_draw_handler(uint32_t event, void *data, void *user_data)
{
	switch (event) {
	case SURFACE_EVT_DRAW_READY:
		break;
	case SURFACE_EVT_DRAW_COVER_CHECK: {
		surface_cover_check_data_t *cover_check = data;
		cover_check->covered = true; /* alway skip swapbuf copy */
		break;
	}
	}
}

gui_disp_t * gui_disp_create(surface_t * surface)
{
	gui_disp_t * disp = mem_malloc(sizeof(*disp));

	if (disp == NULL)
		return NULL;

	memset(disp, 0, sizeof(*disp));
	disp->surface = surface;
	surface_register_callback(surface, SURFACE_CB_DRAW, surface_draw_handler, disp);

	sys_slist_append(&disp_list, &disp->node);
	return disp;
}

void gui_disp_destroy(gui_disp_t * disp)
{
	if (act_disp == disp) {
		gui_disp_set_focus(NULL);
	}

	sys_slist_find_and_remove(&disp_list, &disp->node);
	mem_free(disp);
}

int gui_disp_set_focus(gui_disp_t * disp)
{
	if (disp == act_disp) {
		return 0;
	}

	/* Retach the input devices */
	gui_indev_t *indev = gui_indev_get_next(NULL);
	while (indev) {
		indev->disp = disp;
		indev = gui_indev_get_next(indev);
	}

	act_disp = disp;
	return 0;
}

gui_disp_t * gui_disp_get_next(gui_disp_t * disp)
{
	sys_snode_t * node;

	if (disp == NULL) {
		node = sys_slist_peek_head(&disp_list);
	} else {
		node = sys_slist_peek_next(&disp->node);
	}

	return node ? CONTAINER_OF(node, gui_disp_t, node) : NULL;
}
