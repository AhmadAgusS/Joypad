/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <input_manager.h>
#include <msg_manager.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <bt_manager.h>
// #include <global_mem.h>
#include <dvfs.h>
#include "app_defines.h"
#include "system_defs.h"
#include "key_definition.h"

enum{
	BTHID_KEY_VOLUMEDOWN =0,
	BTHID_KEY_VOLUMEUP,
	BTHID_KEY_MUTE ,
	BTHID_KEY_PLAYPAUSE,
};

int bthid_send_key(int key) {
	SYS_LOG_INF("%s, send key=%d", __func__, key);
	uint8_t report_id = 0x02;
	uint8_t func_id = (uint8_t)key;
	return bt_manager_hid_send_key(report_id, func_id);
}

static bool bthid_input_event_proc(struct app_msg *msg)
{
	switch (msg->value) {
	case KEY_UP_ACTION_BLACK | KEY_TYPE_SHORT_UP:
		bthid_send_key(BTHID_KEY_VOLUMEUP);
		break;
	case KEY_LEFT_ACTION_WHITE | KEY_TYPE_SHORT_UP:
		bthid_send_key(BTHID_KEY_VOLUMEDOWN);
		break; 
	case KEY_POWER | KEY_TYPE_SHORT_UP:
		bthid_send_key(BTHID_KEY_PLAYPAUSE);
		break; 
	default:
		break;
	}

	return true;
}

static void bt_hid_main_loop(void *p1, void *p2, void *p3)
{
	struct app_msg msg = { 0 };
	bool terminated = false;

	SYS_LOG_INF("BT HID: Enter loop");

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_set_level(DVFS_LEVEL_HIGH_PERFORMANCE, APP_ID_BTHID);
#endif

	while (!terminated) {
		if (!receive_msg(&msg, OS_FOREVER))
			continue;

		switch (msg.type) {
		case MSG_KEY_INPUT:
			printk("MSG_KEY_INPUT: %x \n", msg.value);
			bthid_input_event_proc(&msg);
			break;
		case MSG_EXIT_APP:
			terminated = true;
			app_manager_thread_exit(APP_ID_BTHID);
			break;
		default:
			break;
		}

		if (msg.callback)
			msg.callback(&msg, 0, NULL);
	}

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_unset_level(DVFS_LEVEL_HIGH_PERFORMANCE, APP_ID_BTHID);
#endif

	SYS_LOG_INF("BT HID: Exit APP");
}

APP_DEFINE(bthid, share_stack_area, sizeof(share_stack_area),
	   APP_PRIORITY, FOREGROUND_APP, NULL, NULL, NULL,
	   bt_hid_main_loop, NULL);
