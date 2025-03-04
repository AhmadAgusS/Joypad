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
#include <ui_mem.h>
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
extern void btcall_tts_event_proc(struct app_msg *msg);
extern bool btcall_key_event_proc(uint32_t event);
extern void btcall_view_resume(void);
#endif/* CONFIG_BT_CALL_APP */
#ifdef CONFIG_ALARM_APP
extern void _alarm_callback(void);
#endif/* CONFIG_ALARM_APP */
//extern void system_create_low_power_view(void);
extern void system_delete_low_power_view(void);


extern bool aod_clock_view_is_inflated(void);

static void _launcher_key_event_handle(uint16_t view_id, uint32_t event);

extern int clock_view_create(void);
extern int clock_view_suspend(void);
extern int clock_view_resume(void);
extern int clock_view_repaint(void);
extern int clock_view_delete(void);
/*******************************************************************************
 * View cache definition
 ******************************************************************************/
#if 0
static void _main_view_get_time(struct rtc_time *time);
static uint8_t _main_get_battery_percent(void);

static const main_view_presenter_t main_view_presenter = {
	.get_time = _main_view_get_time,
	.get_battery_percent = _main_get_battery_percent,
};
#endif
#if 1
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
//static int _clock_view_invoke_preview(void);

const clock_view_presenter_t clock_view_presenter = {
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
	//.invoke_preview = _clock_view_invoke_preview,
};
#endif
launcher_app_t g_launcher_app;

#ifdef CONFIG_RTC_ACTS
static void _launcher_app_update_clock_view(os_work *work);

static OS_WORK_DEFINE(clock_update_work, _launcher_app_update_clock_view);

static void _launcher_app_update_clock_view(os_work *work)
{

}

static void _rtc_alarm_period_handler(const void *cb_data)
{
	os_work_submit(&clock_update_work);
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
#if 1
#if 0
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
#endif
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
	//launcher_app_t *app = launcher_app_get();
	//const clock_dsc_t *clock_dsc = clocksel_get_clock_dsc(app->clock_id);

	//return clock_dsc->scene;
	return 0;
}
#if 0
static int _clock_view_invoke_preview(void)
{
	return clocksel_ui_enter();
}

static uint32_t _heart_view_get_rate(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->heart_rate;
}
#endif
#endif


void launcher_apply_clock_id(uint8_t clock_id)
{
	launcher_app_t *app = launcher_app_get();
#if 0
	const clock_dsc_t *clock_dsc = clocksel_get_clock_dsc(clock_id);
	if (clock_dsc == NULL) {
		clock_dsc = clocksel_get_clock_dsc(0);
		clock_id = 0;
	}
#ifdef CONFIG_RTC_ACTS
	app->rtc_config.tm_msec = clock_dsc->period % 1000;
	app->rtc_config.tm_sec = clock_dsc->period / 1000;
#endif
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
	app->rtc_config.tm_msec = CONFIG_CLOCK_DEF_REFR_PERIOD % 1000,
	app->rtc_config.tm_sec = CONFIG_CLOCK_DEF_REFR_PERIOD / 1000,
#endif

#ifdef CONFIG_PROPERTY
	app->clock_id = (uint8_t)property_get_int(CFG_CLOCK_ID, 0);
#endif

	launcher_apply_clock_id(app->clock_id);

	rtc_set_period_alarm(app->rtc_dev, (void *)&app->rtc_config, true);

	//ui_manager_set_keyevent_callback(_launcher_key_event_handle);
	extern int lvgl_init(void);
	lvgl_init();

	extern int clock_view_create(void);
	clock_view_create();

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
	_music_player_exit();

	extern int clock_view_delete(void);
	clock_view_delete();

	app_manager_thread_exit(APP_ID_LAUNCHER);
}


static void _launcher_app_early_suspend(void)
{
	launcher_app_t *app = launcher_app_get();

	app->suspended = 1;
	clock_view_suspend();
	SYS_LOG_INF("launcher early-suspend\n");
}

static void _launcher_app_late_resume(void)
{
	launcher_app_t *app = launcher_app_get();

	app->suspended = 0;

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

	clock_view_resume();

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
		#ifdef CONFIG_ALARM_APP
			alarm_snooze(false);
		#endif
			app->cur_player = BTCALL_PLAYER;
			clock_view_delete();
			btcall_bt_event_proc(msg);
		} else if (app->cur_player == LCMUSIC_PLAYER || app->cur_player == BTMUSIC_PLAYER){
			SYS_LOG_INF("switch player %d -> btcall\n", app->cur_player);
			_music_player_exit();
			app->prev_player = app->cur_player;
			app->cur_player = BTCALL_PLAYER;
			btcall_bt_event_proc(msg);
		}
	} else if (msg->cmd == BT_HFP_CALL_STATE_EXIT) {
		clock_view_create();
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
	//if (app->cur_player == BTMUSIC_PLAYER) {
		bt_player_bt_event_proc(msg);
		return;
	//}
#endif
}

static void _launcher_bt_mgr_event_handle(struct app_msg *msg)
{

	switch (msg->cmd) {
#ifdef CONFIG_ALARM_MANAGER
	case BT_MAP_SET_TIME_EVENT:
		int ret;
		ret = alarm_manager_set_time((struct rtc_time *)msg->ptr);
		if (ret) {
			SYS_LOG_ERR("set time error ret=%d\n", ret);
		} else {
			find_and_set_alarm();
		}
		ui_view_paint(MAIN_VIEW);
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
#ifdef CONFIG_ALARM_APP
	launcher_app_t *app = launcher_app_get();
#endif
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

	if (event == (KEY_POWER | KEY_TYPE_LONG_DOWN)) {/* enter power view */
		return;
	}

	if (event == (KEY_POWER | KEY_TYPE_SHORT_UP)) {
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

			break;
		}
	}
}

static void _launcher_input_event_handle(struct app_msg *msg)
{
#ifdef CONFIG_BT_CALL_APP
	launcher_app_t *app = launcher_app_get();

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
static void _launcher_tts_event_handle(struct app_msg *msg)
{
#ifdef CONFIG_BT_CALL_APP
	launcher_app_t *app = launcher_app_get();

	if (app->cur_player == BTCALL_PLAYER) {
		btcall_tts_event_proc(msg);
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
	int timeout = 0;

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
		timeout = thread_timer_next_timeout();
		if (timeout > CONFIG_LV_DISP_DEF_REFR_PERIOD || timeout == -1) {
			timeout = CONFIG_LV_DISP_DEF_REFR_PERIOD;
		} else {
			timeout = thread_timer_next_timeout();
		}
		timeout = (app->suspended == 2) ?
				OS_FOREVER : timeout;
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
			case MSG_TTS_EVENT:
				_launcher_tts_event_handle(&msg);
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
					//system_create_low_power_view();
				}
				break;
			case MSG_BAT_CHARGE_EVENT:
				if (msg.cmd == BAT_CHG_EVENT_DC5V_IN) {
					//system_delete_low_power_view();
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

		if (!app->suspended) {
			clock_view_repaint();
		}

		lv_task_handler();
		thread_timer_handle_expired();
	}

out_exit:
	SYS_LOG_INF(APP_ID_LAUNCHER " exit");
}

APP_DEFINE(launcher, share_stack_area, sizeof(share_stack_area),
		CONFIG_APP_PRIORITY, DEFAULT_APP | FOREGROUND_APP, NULL, NULL, NULL,
	   _launcher_app_loop, NULL);
