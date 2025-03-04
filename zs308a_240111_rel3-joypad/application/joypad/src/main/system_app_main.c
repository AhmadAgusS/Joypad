/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <thread_timer.h>
#include <input_manager.h>
#include <power_manager.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <sys_manager.h>
#include <volume_manager.h>
#include <bt_manager.h>
#include <app_utils.h>
#include <tts_manager.h>
#include <app_switch.h>
#include <system_defs.h>
#include <soc.h>
#include <hotplug_manager.h>
#include <dvfs.h>
#include "key_definition.h"

#ifdef CONFIG_TOOL
#include "tool_app.h"
#endif

/*share stack for app thread */
char __aligned(ARCH_STACK_PTR_ALIGN) share_stack_area[CONFIG_APP_STACKSIZE];

static void system_input_event_cb(uint32_t key_value, uint16_t type)
{
	if ((type == EV_KEY) && ((key_value & ~KEY_TYPE_ALL) == KEY_POWER)) {
		if (k_uptime_get() < 3000)
			input_manager_filter_key_itself();
	}
}

static void system_key_event_handle(uint32_t key_event)
{
	struct app_msg msg = { 0 };
	bool need_dispatch = false;

	/* system key event handle */
	switch (key_event) {
	case KEY_POWER | KEY_TYPE_LONG_DOWN:
		msg.type = MSG_POWER_OFF;
		break;

	case KEY_SELFDEFINED_MENU | KEY_TYPE_SHORT_UP:
		msg.type = MSG_SWITCH_APP;
		break;

	default:
		need_dispatch = true;
		break;
	}

	if (msg.type > 0) {
		send_async_msg(APP_ID_MAIN, &msg);
	}

	if (need_dispatch) {
		msg.type = MSG_KEY_INPUT;
		msg.value = key_event;
		send_async_msg(app_manager_get_current_app(), &msg);
	}
}

extern int system_enter_FCC_mode(void);
void main(void)
{
	struct app_msg msg;
	bool init_bt_manager = true;

	u16_t reboot_type = 0;
	u8_t reason = 0;

	sys_pm_get_reboot_reason(&reboot_type, &reason);

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_set_level(DVFS_LEVEL_NORMAL, APP_ID_MAIN);
#endif

	system_init();

	system_audio_policy_init();

#ifdef CONFIG_INPUT_MANAGER
	input_manager_init(system_input_event_cb);
	input_manager_unlock();
#endif

#if 0
#ifdef CONFIG_BT_CONTROLER_RF_FCC
	if ((reboot_type == REBOOT_TYPE_GOTO_SYSTEM) && (reason == REBOOT_REASON_GOTO_BQB)) {
		SYS_LOG_INF("Goto BQB TEST");
	} else {
		system_enter_FCC_mode();
	}
#endif
#endif

	system_app_launch(APP_ID_BTHID);

#ifdef CONFIG_BT_MANAGER
	if (init_bt_manager)
		bt_manager_init();

#ifdef CONFIG_BT_BLE_VENDER_GATT_APP
	extern void bt_ble_vnd_gatt_init(void);
	bt_ble_vnd_gatt_init();
#endif //CONFIG_BT_BLE_VENDER_GATT_APP
#endif

#ifdef CONFIG_PLAYTTS
	tts_manager_play_block("welcome.mp3");
#endif

	while (1) {
		if (receive_msg(&msg, thread_timer_next_timeout())) {
			switch (msg.type) {
			case MSG_KEY_INPUT:
				system_key_event_handle(msg.value);
				break;
			case MSG_LOW_POWER:
			#ifdef CONFIG_PLAYTTS
				tts_manager_play_block("batlow.mp3");
			#endif
				break;
			case MSG_NO_POWER:
			#ifdef CONFIG_PLAYTTS
				tts_manager_play_block("battlo.mp3");
			#endif
				break;
			case MSG_POWER_OFF:
			{
				input_manager_lock();
				app_manager_active_app(APP_ID_ILLEGAL);
			#ifdef CONFIG_PLAYTTS
				tts_manager_play_block("poweroff.mp3");
			#endif
				system_power_off();
				break;
			}
			case MSG_REBOOT:
				system_power_reboot(REBOOT_TYPE_SF_RESET);
				break;

#ifdef CONFIG_ACTS_BT
			case MSG_BT_ENGINE_READY:
				/** input init is locked ,so we must unlock*/
				input_manager_unlock();

				/* activate the recorder app after bt engine is ready*/
				app_switch(APP_ID_BTHID, APP_SWITCH_CURR, false);
				break;
#endif

			case MSG_SWITCH_APP:
				app_switch(NULL, APP_SWITCH_NEXT, false);
				break;
			case MSG_BT_EVENT:
			default:
				break;
			}

			if (msg.callback)
				msg.callback(&msg, 0, NULL);
		}

		thread_timer_handle_expired();
	}

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_unset_level(DVFS_LEVEL_NORMAL, APP_ID_MAIN);
#endif
}

extern char z_main_stack[CONFIG_MAIN_STACK_SIZE];

APP_DEFINE(main,
		   z_main_stack, CONFIG_MAIN_STACK_SIZE, \
		   APP_PRIORITY, RESIDENT_APP, \
		   NULL, NULL, NULL, \
		   NULL, NULL);
