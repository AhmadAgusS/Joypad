/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file launcher_app.c
 */
#include <assert.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <power_manager.h>
#include <ui_manager.h>
#include <view_manager.h>
#include <thread_timer.h>
#include <lvgl/lvgl_res_loader.h>
#include <lvgl/lvgl_bitmap_font.h>
#include <view_stack.h>
#include <msgbox_cache.h>
#include "app_defines.h"
#include "system_app.h"
#include "launcher_app.h"
#include "main_view.h"
#include "clock_view.h"
#include "heart_view.h"
#include "applist/applist.h"
#include "health/health.h"
#include "clock_selector/clock_selector.h"
#include "alipay/alipay_ui.h"
#include "app_ui.h"
#ifndef CONFIG_SIMULATOR
#include <soc.h>
#include <drivers/power_supply.h>
#else
#include <native_window.h>
#endif

#ifdef CONFIG_BT_MANAGER
#include <bt_manager.h>
#endif

#ifdef CONFIG_ALARM_MANAGER
#include <alarm_manager.h>
#endif
#ifdef CONFIG_ALARM_APP
#include "../alarm/alarm.h"
#endif
#ifdef CONFIG_SYS_WAKELOCK
#include <sys_wakelock.h>
#endif

#ifdef CONFIG_BT_PLAYER
extern void bt_player_bt_event_proc(struct app_msg *msg);
extern void btmusic_view_input_event_proc(struct app_msg *msg);
#endif /* CONFIG_BT_PLAYER */
#ifdef CONFIG_BT_CALL_APP
extern void btcall_input_event_proc(struct app_msg *msg);
extern void btcall_bt_event_proc(struct app_msg *msg);
extern bool btcall_key_event_proc(uint32_t event);
extern void btcall_view_resume(void);
#endif/* CONFIG_BT_CALL_APP */
#ifdef CONFIG_ALARM_APP
extern void _alarm_callback(void);
#endif/* CONFIG_ALARM_APP */
extern void system_create_low_power_view(void);
extern void system_delete_low_power_view(void);
extern void lcmusic_restore_music_player_mode(launcher_app_t *app);

extern bool aod_clock_view_is_inflated(void);

static void _launcher_key_event_handle(uint16_t view_id, uint32_t event);

/*******************************************************************************
 * View cache definition
 ******************************************************************************/
static void _main_view_get_time(struct rtc_time *time);
static uint8_t _main_get_battery_percent(void);

static const main_view_presenter_t main_view_presenter = {
	.get_time = _main_view_get_time,
	.get_battery_percent = _main_get_battery_percent,
};

static void _clock_view_get_time(struct rtc_time *time);
static uint8_t _clock_get_battery_percent(void);
static uint8_t _clock_get_temperature(void);
static uint8_t _clock_get_weather(void);
static uint32_t _clock_view_get_step_count(void);
static uint32_t _clock_view_get_heart_rate(void);
static uint32_t _clock_view_get_calories(void);
static uint32_t _clock_get_sleep_time(void);
static uint32_t _clock_get_distance(void);
static uint32_t _clock_view_get_clock_id(void);
static int _clock_view_invoke_preview(void);

static const clock_view_presenter_t clock_view_presenter = {
	.get_time = _clock_view_get_time,
	.get_battery_percent = _clock_get_battery_percent,
	.get_temperature = _clock_get_temperature,
	.get_weather = _clock_get_weather,
	.get_step_count = _clock_view_get_step_count,
	.get_heart_rate = _clock_view_get_heart_rate,
	.get_calories = _clock_view_get_calories,
	.get_sleep_time = _clock_get_sleep_time,
	.get_distance = _clock_get_distance,
	.get_clock_id = _clock_view_get_clock_id,
	.invoke_preview = _clock_view_invoke_preview,
};

static uint32_t _heart_view_get_rate(void);

static const heart_view_presenter_t heart_view_presenter = {
	.get_heart_rate = _heart_view_get_rate,
};

launcher_app_t g_launcher_app;

static const uint16_t app_view_id_list[] = {
	SPORT_VIEW, CLOCK_VIEW, HEART_VIEW,
	MUSIC_VIEW, ALIPAY_MAIN_VIEW, WXPAY_MAIN_VIEW,
};

static const void * app_view_presenter_list[] = {
	NULL,
	&clock_view_presenter,
	&heart_view_presenter,
	NULL,
	&alipay_view_presenter,
	&wxpay_view_presenter,
};

static void _view_cache_focus_proc(uint16_t view_id, bool focus);
static void _view_cache_event_proc(uint8_t event);

static const view_cache_dsc_t app_view_cache_dsc = {
	.type = LANDSCAPE,
	.serial_load = 1,
	.num = ARRAY_SIZE(app_view_id_list),
	.vlist = app_view_id_list,
	.plist = app_view_presenter_list,
	.cross_attached_view = VIEW_INVALID_ID,
	.cross_vlist = { MAIN_VIEW, MSG_VIEW },
	.cross_plist = { &main_view_presenter, NULL },

	.focus_cb = _view_cache_focus_proc,
	.event_cb = _view_cache_event_proc,
};

#ifdef CONFIG_RTC_ACTS
static void _launcher_app_update_clock_view(os_work *work);

static OS_WORK_DEFINE(clock_update_work, _launcher_app_update_clock_view);

static void _launcher_app_update_clock_view(os_work *work)
{
	launcher_app_t *app = launcher_app_get();

	ui_view_paint(app->in_aod_view ? AOD_CLOCK_VIEW : CLOCK_VIEW);
}

static void _rtc_alarm_period_handler(const void *cb_data)
{
	os_work_submit(&clock_update_work);
}

static void _clock_view_focus_proc(bool focus)
{
	launcher_app_t *app = launcher_app_get();
	SYS_LOG_INF("focus %d\n", focus);
	rtc_set_period_alarm(app->rtc_dev, (void *)&app->rtc_config, focus);
}

#elif defined(CONFIG_SIMULATOR)

static void _rtc_timer_period_handler(void * user_data)
{
	ui_view_paint(CLOCK_VIEW);
}

static void _clock_view_focus_proc(bool focus)
{
	launcher_app_t *app = launcher_app_get();
	SYS_LOG_INF("focus %d\n", focus);

	if (focus) {
		native_window_register_callback(NWIN_CB_CLOCK, _rtc_timer_period_handler, NULL);
	} else {
		native_window_unregister_callback(NWIN_CB_CLOCK);
	}
}
#else
static void _clock_view_focus_proc(bool focus) { }
#endif /* CONFIG_RTC_ACTS*/

static void _view_cache_focus_proc(uint16_t view_id, bool focus)
{
	SYS_LOG_DBG("view %d focus %d\n", view_id, focus);

	if (view_id == CLOCK_VIEW) {
		_clock_view_focus_proc(focus);
	}
}

static void _view_cache_event_proc(uint8_t event)
{
	switch (event) {
	case VIEW_CACHE_EVT_LOAD_BEGIN:
		//msgbox_cache_set_en(false);
		break;
	case VIEW_CACHE_EVT_LOAD_END:
	case VIEW_CACHE_EVT_LOAD_CANCEL:
	default:
		msgbox_cache_set_en(true);
		break;
	}
}

static void _main_view_get_time(struct rtc_time *time)
{
	_clock_view_get_time(time);
}

static uint8_t _main_get_battery_percent(void)
{
#ifdef CONFIG_POWER
	return power_manager_get_battery_capacity();
#else
	return 100;
#endif
}

static void _clock_view_get_time(struct rtc_time *time)
{
	launcher_app_t *app = launcher_app_get();

#ifdef CONFIG_RTC_ACTS
	if (app->rtc_dev) {
		rtc_get_time(app->rtc_dev, &app->rtc_time);
	}
#elif defined(CONFIG_SIMULATOR)
	native_window_get_local_time(&app->rtc_time);
#endif /* CONFIG_RTC_ACTS */

	memcpy(time, &app->rtc_time, sizeof(*time));
}

static uint8_t _clock_get_battery_percent(void)
{
	return 100;
}

static uint8_t _clock_get_temperature(void)
{
	return 30;
}

static uint8_t _clock_get_weather(void)
{
	return SUNNY;
}

static uint32_t _clock_get_sleep_time(void)
{
	return 310;
}

static uint32_t _clock_get_distance(void)
{
	return 12345;
}

static uint32_t _clock_view_get_step_count(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->step_count;
}

static uint32_t _clock_view_get_heart_rate(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->heart_rate;
}

static uint32_t _clock_view_get_calories(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->calories;
}

static uint32_t _clock_view_get_clock_id(void)
{
	launcher_app_t *app = launcher_app_get();
	const clock_dsc_t *clock_dsc = clocksel_get_clock_dsc(app->clock_id);

	return clock_dsc->scene;
}

static int _clock_view_invoke_preview(void)
{
	return clocksel_ui_enter();
}

static uint32_t _heart_view_get_rate(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->heart_rate;
}

#ifdef CONFIG_SENSOR_MANAGER
static int _sensor_res_init(void)
{
	launcher_app_t *app = launcher_app_get();
	sensor_res_t *res = &app->sensor_res;
	int ret;

	ret = sensor_manager_get_result(res);
	if (ret == 0) {
		app->heart_rate = (uint32_t)res->heart_rate;
		app->step_count = (uint32_t)res->pedometer.total_steps;
		SYS_LOG_INF("heart_rate: %d", app->heart_rate);
		SYS_LOG_INF("step_count: %d", app->step_count);
	}

	return 0;
}

static int _sensor_res_callback(int evt_id, sensor_res_t *res)
{
	launcher_app_t *app = launcher_app_get();

	if (app->heart_rate != (uint32_t)res->heart_rate) {
		app->heart_rate = (uint32_t)res->heart_rate;
		SYS_LOG_INF("heart_rate: %d", app->heart_rate);
		ui_view_paint(HEART_VIEW);
	}

	if (app->step_count != (uint32_t)res->pedometer.total_steps) {
		app->step_count = (uint32_t)res->pedometer.total_steps;
		SYS_LOG_INF("step_count: %d", app->step_count);
		ui_view_paint(CLOCK_VIEW);
	}

	if (app->bearing != (uint32_t)res->orientation) {
		app->bearing = (uint32_t)res->orientation;
		SYS_LOG_INF("bearing: %d", app->bearing);
		ui_view_paint(COMPASS_VIEW);
	}

	return 0;
}
#endif /* CONFIG_SENSOR_MANAGER */

void launcher_apply_clock_id(uint8_t clock_id)
{
	launcher_app_t *app = launcher_app_get();

	const clock_dsc_t *clock_dsc = clocksel_get_clock_dsc(clock_id);
	if (clock_dsc == NULL) {
		clock_dsc = clocksel_get_clock_dsc(0);
		clock_id = 0;
	}

#ifdef CONFIG_RTC_ACTS
	app->rtc_config.tm_msec = clock_dsc->period % 1000;
	app->rtc_config.tm_sec = clock_dsc->period / 1000;
#endif

	app->clock_id = clock_id;
}

static int _launcher_app_init(void)
{
	launcher_app_t *app = launcher_app_get();

	memset(app, 0, sizeof(*app));
	app->step_count = 3456;
	app->heart_rate = 120;
	app->calories = 789;
	app->bearing = 200;

#ifdef CONFIG_RTC_ACTS
	app->rtc_dev = device_get_binding(CONFIG_RTC_0_NAME);
	assert(app->rtc_dev != NULL);
	app->rtc_config.cb_fn = _rtc_alarm_period_handler;
	app->rtc_config.cb_data = NULL;
#endif

#ifdef CONFIG_PROPERTY
	if (clocksel_get_aod_clock_dsc() != NULL) {
		soc_set_aod_mode((uint8_t)property_get_int(CFG_AOD_MODE, 0));
	}

	app->clock_id = (uint8_t)property_get_int(CFG_CLOCK_ID, 0);
#endif

	launcher_apply_clock_id(app->clock_id);

#ifdef CONFIG_SENSOR_MANAGER
	_sensor_res_init();
	sensor_manager_add_callback(_sensor_res_callback);
#endif

	ui_manager_set_keyevent_callback(_launcher_key_event_handle);

	view_stack_init();
	view_stack_push_cache(&app_view_cache_dsc, CLOCK_VIEW);

#ifdef CONFIG_ALARM_MANAGER
#ifdef CONFIG_ALARM_APP
	system_registry_alarm_callback(_alarm_callback);
#endif
	alarm_manager_init();
	find_and_set_alarm();
#endif
	lcmusic_restore_music_player_mode(app);
	return 0;
}

static void _music_player_exit(void)
{
	struct app_msg msg = { 0 };

	/*stop btmusic player*/
#ifdef CONFIG_BT_PLAYER
	msg.cmd = BT_A2DP_STREAM_SUSPEND_EVENT;
	bt_player_bt_event_proc(&msg);
#endif
#ifdef CONFIG_ALARM_APP
	alarm_exit(false);
#endif
#ifdef CONFIG_BT_CALL_APP
	msg.cmd = BT_HFP_CALL_STATE_EXIT;
	btcall_bt_event_proc(&msg);
#endif
}

static void _launcher_app_exit(void)
{
#ifdef CONFIG_SENSOR_MANAGER
	sensor_manager_remove_callback(_sensor_res_callback);
#endif
	_music_player_exit();

	view_stack_deinit();
	app_manager_thread_exit(APP_ID_LAUNCHER);
}

#ifndef CONFIG_SIMULATOR

static void _aod_clock_view_enter(void)
{
	launcher_app_t *app = launcher_app_get();

	SYS_LOG_INF("enter\n");

	app->in_aod_view = 1;

	//ui_view_create(AOD_CLOCK_VIEW, &clock_view_presenter, UI_CREATE_FLAG_SHOW);
	view_stack_push_view(AOD_CLOCK_VIEW, &clock_view_presenter);

#ifdef CONFIG_RTC_ACTS
	const clock_dsc_t *clock_dsc = clocksel_get_aod_clock_dsc();
	struct rtc_alarm_period_config rtc_config = {
		.tm_msec = clock_dsc->period % 1000,
		.tm_sec = clock_dsc->period / 1000,
		.cb_fn = _rtc_alarm_period_handler,
		.cb_data = NULL,
	};

	rtc_set_period_alarm(app->rtc_dev, (void *)&rtc_config, true);
#endif

	while (aod_clock_view_is_inflated() == false) {
		os_sleep(2);
	}
}

static void _aod_clock_view_exit(void)
{
	launcher_app_t *app = launcher_app_get();

#ifdef CONFIG_RTC_ACTS
	rtc_set_period_alarm(app->rtc_dev, NULL, false);
#endif

	app->in_aod_view = 0;

	//ui_view_delete(AOD_CLOCK_VIEW);
	view_stack_pop();

	SYS_LOG_INF("exit\n");
}

#endif /* CONFIG_SIMULATOR */

static void _launcher_app_early_suspend(void)
{
	launcher_app_t *app = launcher_app_get();

#ifndef CONFIG_SIMULATOR
	if (soc_get_aod_mode()) {
		_aod_clock_view_enter();
	}
#endif /* CONFIG_SIMULATOR */

	app->suspended = 1;

	SYS_LOG_INF("launcher early-suspend\n");
}

static void _launcher_app_late_resume(void)
{
	launcher_app_t *app = launcher_app_get();

	app->suspended = 0;

#ifndef CONFIG_SIMULATOR
	if (app->in_aod_view) {
		_aod_clock_view_exit();
	}
#endif /* CONFIG_SIMULATOR */

	if (view_stack_get_num() == 0) {
		view_stack_push_cache(&app_view_cache_dsc, CLOCK_VIEW);
	}

#ifdef CONFIG_BT_CALL_APP
	if (app->cur_player == BTCALL_PLAYER) {
		btcall_view_resume();
	}
#endif

#ifdef CONFIG_ALARM_APP
	if (app->alarm_en) {
		alarm_start_msg_send();
		app->alarm_en = 0;
	}
#endif

	SYS_LOG_INF("launcher late-resume\n");
}

static void _launcher_app_suspend(void)
{
	launcher_app_t *app = launcher_app_get();

#ifdef CONFIG_SYS_WAKELOCK
	if (sys_wakelocks_check(PARTIAL_WAKE_LOCK)
		|| sys_wakelocks_check(FULL_WAKE_LOCK)) {
		SYS_LOG_INF("wake locked\n");
		return;
	}
#endif

	app->suspended = 2;

	SYS_LOG_INF("launcher suspend\n");
}

static void _launcher_app_resume(void)
{
	launcher_app_t *app = launcher_app_get();

	app->suspended = 1;

	SYS_LOG_INF("launcher resume\n");
}

#ifdef CONFIG_BT_MANAGER

#ifdef CONFIG_BT_CALL_APP
static void btcall_start_exit_event(struct app_msg *msg)
{
	launcher_app_t *app = launcher_app_get();

	if (msg->cmd == BT_HFP_CALL_STATE_START) {
		if (app->cur_player == BTCALL_PLAYER) {
			SYS_LOG_INF("btcall is cur player\n");
		} else if (app->cur_player == ALARM_PLAYER) {
			SYS_LOG_INF("switch player alarm -> btcall\n");
			alarm_snooze(false);
			app->cur_player = BTCALL_PLAYER;
			btcall_bt_event_proc(msg);
		} else if (app->cur_player == LCMUSIC_PLAYER || app->cur_player == BTMUSIC_PLAYER){
			SYS_LOG_INF("switch player %d -> btcall\n", app->cur_player);
			_music_player_exit();
			app->prev_player = app->cur_player;
			app->cur_player = BTCALL_PLAYER;
			btcall_bt_event_proc(msg);
		}
	} else if (msg->cmd == BT_HFP_CALL_STATE_EXIT) {
		btcall_bt_event_proc(msg);
	} else {
		btcall_bt_event_proc(msg);
	}
}
#endif

static void _launcher_bt_event_handle(struct app_msg *msg)
{
	launcher_app_t *app = launcher_app_get();

#ifdef CONFIG_BT_CALL_APP
	if (msg->cmd == BT_HFP_CALL_STATE_START || msg->cmd == BT_HFP_CALL_STATE_EXIT || app->cur_player == BTCALL_PLAYER) {
		btcall_start_exit_event(msg);
		return;
	}
#endif

#ifdef CONFIG_BT_PLAYER
	if (app->cur_player == BTMUSIC_PLAYER) {
		bt_player_bt_event_proc(msg);
		return;
	}
#endif
}

static void _launcher_bt_mgr_event_handle(struct app_msg *msg)
{
	int ret;

	switch (msg->cmd) {
#ifdef CONFIG_ALARM_MANAGER
	case BT_MAP_SET_TIME_EVENT:
		ret = alarm_manager_set_time((struct rtc_time *)msg->ptr);
		if (ret) {
			SYS_LOG_ERR("set time error ret=%d\n", ret);
		} else {
			find_and_set_alarm();
		}
		ui_view_paint(MAIN_VIEW);
		// sync time for alipay
		alipay_ui_sync_time();
		break;
#endif

	default:
		SYS_LOG_ERR("error: 0x%x!\n", msg->cmd);
		break;
	}
}
#endif /* CONFIG_BT_MANAGER */

#ifdef CONFIG_ALARM_APP
static void _launcher_alarm_event_handle(struct app_msg *msg)
{
	launcher_app_t *app = launcher_app_get();

	if (app->cur_player == BTCALL_PLAYER) {
		SYS_LOG_INF("btcall going");
		alarm_snooze(false);
		return;
	}

	if (msg->cmd == MSG_ALARM_PROC_START) {
		if (app->cur_player != ALARM_PLAYER) {
			_music_player_exit();
			app->prev_player = app->cur_player;
			app->cur_player = ALARM_PLAYER;
			SYS_LOG_INF("switch player %d to alarm\n", app->prev_player);
		}

		if (app->suspended != 2) {
			alarm_event_proc(msg);
		} else {
		#ifdef CONFIG_SYS_WAKELOCK
			sys_wake_lock(FULL_WAKE_LOCK);
			sys_wake_unlock(FULL_WAKE_LOCK);
		#endif
			app->alarm_en = 1;
		}
	} else if (app->cur_player == ALARM_PLAYER) {
		alarm_event_proc(msg);
	}
}
#endif /* CONFIG_ALARM_APP */

static void _launcher_key_event_handle(uint16_t view_id, uint32_t event)
{
	launcher_app_t *app = launcher_app_get();

	SYS_LOG_INF("view %u, key 0x%x", view_id, event);

	os_strace_u32(SYS_TRACE_ID_KEY_READ, event);

#ifdef CONFIG_BT_CALL_APP
	/* if (app->cur_player == BTCALL_PLAYER) */ {
		if (btcall_key_event_proc(event) == true)
			return;
	}
#endif

#ifdef CONFIG_ALARM_APP
		if (app->cur_player == ALARM_PLAYER) {
			if (alarm_key_event_proc(event) == true)
				return;
		}
#endif /* CONFIG_ALARM_APP */

	if (msgbox_cache_num_popup_get() > 0) {
		return;
	}

	if (event == (KEY_POWER | KEY_TYPE_LONG_DOWN)) {/* enter power view */
		view_stack_push_view(POWER_VIEW, NULL);
		return;
	}

	if (event == (KEY_POWER | KEY_TYPE_SHORT_UP)) {		
		if (view_stack_get_num() == 1 && view_cache_get_focus_view()==CLOCK_VIEW) { /* in launcher views */
			applist_ui_enter();
			return;
		}
		else if(view_stack_get_num() == 1)
		{
			view_cache_set_focus_view(CLOCK_VIEW);
			return;
		}
		
		if (view_stack_get_top() == APPLIST_VIEW) {
			health_ui_enter();
			return;
		}

		view_stack_pop_until_first(); /* go back to the CLOCK_VIEW */
		return;
	}

#ifdef CONFIG_INPUT_DEV_ACTS_KNOB
	if(event == KEY_KONB_CLOCKWISE) {
		SYS_LOG_INF("KEY_KONB_CLOCKWISE");
	} else if(event == KEY_KONB_ANTICLOCKWISE) {
		SYS_LOG_INF("KEY_KONB_ANTICLOCKWISE");
	}
#endif

	if (KEY_VALUE(event) == KEY_GESTURE_RIGHT) {
		switch (view_id) {
		case BTCALL_VIEW:
		case CLOCK_SELECTOR_SUBVIEW_0:
		case CLOCK_SELECTOR_SUBVIEW_1:
		case CLOCK_SELECTOR_SUBVIEW_2:
		case CLOCK_SELECTOR_SUBVIEW_3:
		case CLOCK_SELECTOR_SUBVIEW_4:
			break;
		default:
			view_stack_pop();
			break;
		}
	}
}

static void _launcher_input_event_handle(struct app_msg *msg)
{
	launcher_app_t *app = launcher_app_get();

#ifdef CONFIG_BT_CALL_APP
	if (app->cur_player == BTCALL_PLAYER ||
		msg->cmd == MSG_BT_SIRI_START ||
		msg->cmd == MSG_BT_SIRI_STOP) {
		btcall_input_event_proc(msg);
		return;
	}
#endif

#ifdef CONFIG_ALARM_APP
	if (app->cur_player == ALARM_PLAYER) {
		alarm_input_event_proc(msg);
		return;
	}
#endif

#ifdef CONFIG_BT_PLAYER
	if (msg->reserve) { /*(app->cur_player == BTMUSIC_PLAYER)*/
		btmusic_view_input_event_proc(msg);
		return;
	}
#endif
}

void launcher_restore_last_player(void)
{
	launcher_app_t *app = launcher_app_get();

	if (app->cur_player != LCMUSIC_PLAYER && app->cur_player != BTMUSIC_PLAYER) {
		SYS_LOG_INF("restore player %d\n", app->prev_player);
		app->cur_player = app->prev_player;
	}
}

void _launcher_app_loop(void *p1, void *p2, void *p3)
{
	launcher_app_t *app = NULL;
	struct app_msg msg = {0};
	bool terminated = false;
	int timeout;

	SYS_LOG_INF(APP_ID_LAUNCHER " enter");

	if (_launcher_app_init()) {
		SYS_LOG_ERR(APP_ID_LAUNCHER " init failed");
		_launcher_app_exit();
		goto out_exit;
	}

#ifdef CONFIG_ALARM_APP
	if (alarm_wakeup_source_check()) {
		alarm_start_msg_send();
	}
#endif

	app = launcher_app_get();

	while (!terminated) {
		timeout = (app->suspended == 2) ?
				OS_FOREVER : thread_timer_next_timeout();

		if (receive_msg(&msg, timeout)) {
			switch (msg.type) {
			case MSG_EXIT_APP:
				_launcher_app_exit();
				terminated = true;
				break;

			case MSG_EARLY_SUSPEND_APP:
				_launcher_app_early_suspend();
				break;
			case MSG_LATE_RESUME_APP:
				_launcher_app_late_resume();
				break;
			case MSG_SUSPEND_APP:
				_launcher_app_suspend();
				break;
			case MSG_RESUME_APP:
				_launcher_app_resume();
				break;
			case MSG_KEY_INPUT:
				_launcher_key_event_handle(VIEW_INVALID_ID, msg.value);
				break;
			case MSG_INPUT_EVENT:
				_launcher_input_event_handle(&msg);
				break;

			#ifdef CONFIG_ALARM_APP
			case MSG_ALARM_EVENT:
				_launcher_alarm_event_handle(&msg);
				break;
			#endif

			case MSG_SWITCH_LAST_PLAYER:
				launcher_restore_last_player();
				break;

			#ifndef CONFIG_SIMULATOR
			case MSG_SYS_EVENT:
				if (msg.cmd == SYS_EVENT_BATTERY_LOW) {
					system_create_low_power_view();
				}
				break;
			case MSG_BAT_CHARGE_EVENT:
				if (msg.cmd == BAT_CHG_EVENT_DC5V_IN) {
					system_delete_low_power_view();
				}
				break;
			#endif /* CONFIG_SIMULATOR */

			#ifdef CONFIG_BT_MANAGER
			case MSG_BT_EVENT:
				_launcher_bt_event_handle(&msg);
				break;
			case MSG_BT_MGR_EVENT:
				_launcher_bt_mgr_event_handle(&msg);
				break;
			#endif /* CONFIG_BT_MANAGER */

			default:
				break;
			}

			if (msg.callback != NULL)
				msg.callback(&msg, 0, NULL);
		}

		thread_timer_handle_expired();
	}

out_exit:
	SYS_LOG_INF(APP_ID_LAUNCHER " exit");
}

APP_DEFINE(launcher, share_stack_area, sizeof(share_stack_area),
		CONFIG_APP_PRIORITY, DEFAULT_APP | FOREGROUND_APP, NULL, NULL, NULL,
	   _launcher_app_loop, NULL);
