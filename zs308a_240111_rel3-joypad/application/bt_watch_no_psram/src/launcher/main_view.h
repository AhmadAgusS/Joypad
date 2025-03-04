/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BT_WATCH_SRC_LAUNCHER_MAIN_VIEW_H_
#define BT_WATCH_SRC_LAUNCHER_MAIN_VIEW_H_
#ifdef CONFIG_RTC_ACTS
#include <drivers/rtc.h>
#endif
#include "app_ui.h"

typedef struct main_view_presenter {
	void (*get_time)(struct rtc_time *time);
	uint8_t (*get_battery_percent)(void);
} main_view_presenter_t;

#endif /* BT_WATCH_SRC_LAUNCHER_MAIN_VIEW_H_ */
