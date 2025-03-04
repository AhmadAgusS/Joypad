/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <ui_service.h>
#include <sys/util.h>
#include "gui_indev.h"

static sys_slist_t indev_list = SYS_SLIST_STATIC_INIT(&indev_list);

int gui_indev_register(gui_indev_t * indev)
{
	sys_slist_append(&indev_list, &indev->node);
	return 0;
}

gui_indev_t * gui_indev_get_next(gui_indev_t * indev)
{
	sys_snode_t * node;

	if (indev == NULL) {
		node = sys_slist_peek_head(&indev_list);
	} else {
		node = sys_slist_peek_next(&indev->node);
	}

	return node ? CONTAINER_OF(node, gui_indev_t, node) : NULL;
}
