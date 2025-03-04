/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BT_WATCH_SRC_LAUNCHER_APPLIST_VIEW_H_
#define BT_WATCH_SRC_LAUNCHER_APPLIST_VIEW_H_

#include "app_ui.h"

typedef struct applist_view_presenter {
	bool (*phone_is_on)(void);
	void (*toggle_phone)(void);

	bool (*vibrator_is_on)(void);
	void (*toggle_vibrator)(void);

	bool (*aod_mode_is_on)(void);
	void (*toggle_aod_mode)(void);

	void (*open_stopwatch)(void);
	void (*open_alarmclock)(void);
	void (*open_compass)(void);
	void (*open_longview)(void);
	void (*open_alipay)(void);
	void (*open_wxpay)(void);
} applist_view_presenter_t;

#endif /* BT_WATCH_SRC_LAUNCHER_APPLIST_VIEW_H_ */
