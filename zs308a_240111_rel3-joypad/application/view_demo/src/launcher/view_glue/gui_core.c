/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gui.h"

void gui_task_handler(void)
{
	gui_indev_t *indev = NULL;
	gui_disp_t *disp = NULL;

	do {
		input_dev_data_t input_data;
		bool more = false;

		indev = gui_indev_get_next(indev);
		if (indev == NULL)
			break;

		if (indev->enabled == 0 || indev->disp == NULL)
			continue;

		do {
			more = indev->read_cb(indev, &input_data);

			/*TODO: handle input */
		} while (more == true);
	} while (1);

	do {
		disp = gui_disp_get_next(disp);
		if (disp == NULL)
			break;

		if (disp->paused || disp->draw_cb == NULL)
			continue;

		disp->draw_cb(disp);
	} while (1);
}
