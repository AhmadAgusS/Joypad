/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BT_WATCH_SRC_LAUNCHER_CLOCK_SELECTOR_SUBVIEW_H_
#define BT_WATCH_SRC_LAUNCHER_CLOCK_SELECTOR_SUBVIEW_H_

#include "app_ui.h"

typedef struct clocksel_subview_presenter {
	int8_t (*get_clock_id)(uint16_t view_id);
	const char * (*get_clock_name)(uint8_t id);
	void (*set_clock_id)(uint8_t id);
} clocksel_subview_presenter_t;

#endif /* BT_WATCH_SRC_LAUNCHER_CLOCK_SELECTOR_SUBVIEW_H_ */
