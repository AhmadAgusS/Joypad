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
#include <ui_manager.h>
#include <view_manager.h>
#include <thread_timer.h>
#include <view_cache.h>
#include "app_defines.h"
#include "system_app.h"
#include "launcher_app.h"
#include "app_ui.h"
#include <logging/log.h>

static void _launcher_key_event_handle(uint16_t view_id, uint32_t event);

/*******************************************************************************
 * View cache definition
 ******************************************************************************/
static const uint16_t test_view_list[] = {
	TEST_VIEW_1, TEST_VIEW_2,
};

static const view_cache_dsc_t test_view_cache_dsc = {
	.type = LANDSCAPE,
	.num = ARRAY_SIZE(test_view_list),
	.vlist = test_view_list,
	.plist = NULL,
	.focus_cb = NULL,
};

static int _launcher_app_init(void)
{
	ui_manager_set_keyevent_callback(_launcher_key_event_handle);

	view_cache_init(&test_view_cache_dsc, TEST_VIEW_1);
	return 0;
}

static void _launcher_app_exit(void)
{
	app_manager_thread_exit(APP_ID_LAUNCHER);
}

static void _launcher_app_suspend(void)
{
	LOG_INF("launcher suspend\n");
}

static void _launcher_app_resume(void)
{
	LOG_INF("launcher resume\n");
}

static void _launcher_key_event_handle(uint16_t view_id, uint32_t event)
{
	os_strace_u32(SYS_TRACE_ID_KEY_READ, event);
	
	if (event == (KEY_POWER | KEY_TYPE_LONG_DOWN)) {
		LOG_INF("ONOFF BUTTON LONG PRESSED\n");
		/* do some reset things here */
	} else if (event == (KEY_POWER | KEY_TYPE_SHORT_UP)) {
		LOG_INF("ONOFF BUTTON SHORT PRESSED\n");
		/* do some reset things here */
	}
}

static void _launcher_app_loop(void *p1, void *p2, void *p3)
{
	struct app_msg msg = {0};
	bool terminated = false;
	bool suspended = false;
	int timeout;

	SYS_LOG_INF(APP_ID_LAUNCHER " enter");

	if (_launcher_app_init()) {
		SYS_LOG_ERR(APP_ID_LAUNCHER " init failed");
		_launcher_app_exit();
		goto out_exit;
	}

	while (!terminated) {
		timeout = suspended ? OS_FOREVER : thread_timer_next_timeout();

		if (receive_msg(&msg, timeout)) {
			switch (msg.type) {
			case MSG_EXIT_APP:
				_launcher_app_exit();
				terminated = true;
				break;
			case MSG_SUSPEND_APP:
				_launcher_app_suspend();
				suspended = true;
				break;
			case MSG_RESUME_APP:
				_launcher_app_resume();
				suspended = false;
				break;
			case MSG_KEY_INPUT:
				_launcher_key_event_handle(VIEW_INVALID_ID, msg.value);
				break;
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
