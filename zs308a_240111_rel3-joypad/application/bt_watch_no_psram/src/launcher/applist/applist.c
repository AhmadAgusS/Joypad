/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONFIG_SIMULATOR
#include <soc.h>
#endif
#include <os_common_api.h>
#include <ui_manager.h>
#include <input_manager.h>
#include <property_manager.h>
#include <view_stack.h>
#include <app_ui.h>
#include "applist.h"
#include "applist_view.h"
#include "compass_view.h"
#include "launcher_app.h"
#include "../clock_selector/clock_selector.h"
#include "../alipay/alipay_ui.h"

static uint16_t _compass_view_get_bearing(void);

static const compass_view_presenter_t compass_view_presenter = {
	.get_bearing = _compass_view_get_bearing,
};

static bool _applist_view_phone_is_on(void);
static void _applist_view_toggle_phone(void);
static bool _applist_view_vibrator_is_on(void);
static void _applist_view_toggle_vibrator(void);
static bool _applist_view_aod_mode_is_on(void);
static void _applist_view_toggle_aod_mode(void);
static void _applist_view_open_stopwatch(void);
static void _applist_view_open_alarm(void);
static void _applist_view_open_compass(void);
static void _applist_view_open_longview(void);

static const applist_view_presenter_t applist_view_presenter = {
	.phone_is_on = _applist_view_phone_is_on,
	.toggle_phone = _applist_view_toggle_phone,
	.vibrator_is_on = _applist_view_vibrator_is_on,
	.toggle_vibrator = _applist_view_toggle_vibrator,
	.aod_mode_is_on = _applist_view_aod_mode_is_on,
	.toggle_aod_mode = _applist_view_toggle_aod_mode,
	.open_stopwatch = _applist_view_open_stopwatch,
	.open_alarmclock = _applist_view_open_alarm,
	.open_compass = _applist_view_open_compass,
	.open_longview = _applist_view_open_longview,
	.open_alipay = alipay_ui_enter,
};

static bool _applist_view_phone_is_on(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->phone_en;
}

static void _applist_view_toggle_phone(void)
{
	launcher_app_t *app = launcher_app_get();

	app->phone_en = !app->phone_en;
}

static bool _applist_view_vibrator_is_on(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->vibrator_en;
}

static void _applist_view_toggle_vibrator(void)
{
	launcher_app_t *app = launcher_app_get();

	app->vibrator_en = !app->vibrator_en;
}

static bool _applist_view_aod_mode_is_on(void)
{
#ifndef CONFIG_SIMULATOR
	return soc_get_aod_mode();
#else
	return 0;
#endif
}

static void _applist_view_toggle_aod_mode(void)
{
#ifndef CONFIG_SIMULATOR
	/* AOD mode supported */
	if (clocksel_get_aod_clock_dsc() != NULL) {
		soc_set_aod_mode(!soc_get_aod_mode());
#ifdef CONFIG_PROPERTY
		property_set_int(CFG_AOD_MODE, soc_get_aod_mode());
		property_flush(CFG_AOD_MODE);
#endif
	}
#endif /* CONFIG_SIMULATOR */
}

static void _applist_view_open_stopwatch(void)
{
	view_stack_push_view(STOPWATCH_VIEW, NULL);
}

static void _applist_view_open_alarm(void)
{
#ifdef CONFIG_ALARM_MANAGER
	view_stack_push_view(ALARM_SET_VIEW, NULL);
#endif
}

static uint16_t _compass_view_get_bearing(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->bearing;
}

static void _applist_view_open_compass(void)
{
	view_stack_push_view(COMPASS_VIEW, &compass_view_presenter);
}

static void _applist_view_open_longview(void)
{
	view_stack_push_view(TEST_LONG_VIEW, NULL);
}

int applist_ui_enter(void)
{
	view_stack_push_view(APPLIST_VIEW, &applist_view_presenter);
	return 0;
}
