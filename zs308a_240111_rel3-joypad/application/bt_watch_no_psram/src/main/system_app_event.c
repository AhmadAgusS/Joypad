/*
 * Copyright (c) 2019 Actions Semi Co., Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief system app event.
 */

#include <mem_manager.h>
#include <msg_manager.h>
#include <sys_event.h>

#include "app_ui.h"
#include "system_app.h"
#ifdef CONFIG_SYS_WAKELOCK
#include <sys_wakelock.h>
#endif
#ifdef CONFIG_USB_MASS_STORAGE

#ifdef CONFIG_FS_MANAGER
#include <fs_manager.h>
#endif
#include <usb/class/usb_msc.h>
#endif
#ifdef CONFIG_BLUETOOTH
#include "bt_manager.h"
#include "btservice_api.h"
#endif

#ifdef CONFIG_ALARM_MANAGER
#include <alarm_manager.h>
#endif

extern void recorder_service_start_stop(uint8_t is_stop);
extern void recorder_service_check_disk(const char *disk);
extern void alarm_entry_exit(uint8_t force_exit);
extern int alarm_input_event_proc(struct app_msg *msg);
#ifdef CONFIG_GMA_APP
extern uint8_t gma_dev_sent_record_start_stop(uint8_t cmd);
#endif
void system_key_event_handle(struct app_msg *msg)
{
	SYS_LOG_INF("msg->cmd 0x%x\n", msg->cmd);
	switch (msg->cmd) {
	case MSG_KEY_POWER_OFF:
		sys_event_notify(SYS_EVENT_POWER_OFF);
		break;
	case MSG_FACTORY_DEFAULT:
		sys_event_notify(SYS_EVENT_FACTORY_DEFAULT);
		system_restore_factory_config();
		break;
	case MSG_SWITCH_APP:
		app_switch((char *)msg->ptr, APP_SWITCH_NEXT, false);
		break;
	case MSG_ENTER_PAIRING_MODE:
		/* Design for entering pairing mode, wait to connect by phone */
		break;
#ifdef CONFIG_TWS
	case MSG_BT_PLAY_TWS_PAIR:
		bt_manager_tws_wait_pair();
		break;
	case MSG_BT_PLAY_DISCONNECT_TWS_PAIR:
		bt_manager_tws_disconnect_and_wait_pair();
		break;
	case MSG_BT_TWS_SWITCH_MODE:
		bt_manager_tws_channel_mode_switch();
		break;
#endif
#ifdef CONFIG_BT_MANAGER
	case MSG_BT_PLAY_CLEAR_LIST:
		bt_manager_clear_list(0);
		break;
	case MSG_BT_SIRI_STOP:
		bt_manager_hfp_stop_siri();
		break;
	case MSG_BT_SIRI_START:
		bt_manager_hfp_start_siri();
		break;
	case MSG_BT_CALL_LAST_NO:
		bt_manager_hfp_dial_last_number();
		break;
#endif
#ifdef CONFIG_BT_HID
	case MSG_BT_HID_START:
#ifdef CONFIG_BT_MANAGER
		bt_manager_hid_take_photo();
		/* bt_manager_hid_key_func(KEY_FUNC_HID_CUSTOM_KEY); */
#endif
		break;
#endif
	case MSG_RECORDER_START_STOP:
	#ifdef CONFIG_RECORD_SERVICE
		recorder_service_start_stop(msg->reserve);
	#endif
		break;
#ifdef CONFIG_GMA_APP
	case MSG_GMA_RECORDER_START:
		gma_dev_sent_record_start_stop(0);
#endif
		break;
#ifdef CONFIG_ALARM_APP
	case MSG_ALARM_ENTRY_EXIT:
		alarm_entry_exit(msg->reserve);
		break;
#endif

	default:
	#ifdef CONFIG_ALARM_APP
		if (alarm_input_event_proc(msg))
			SYS_LOG_ERR("error: 0x%x!\n", msg->cmd);
	#endif
		break;
	}
}

void system_hotplug_event_handle(struct app_msg *msg)
{
#ifdef CONFIG_HOTPLUG_MANAGER
	switch (msg->cmd) {
	case HOTPLUG_LINEIN:
		break;
	case HOTPLUG_SDCARD:
		break;
	case HOTPLUG_USB_DEVICE:
		break;
	case HOTPLUG_USB_HOST:
		break;
	default:
		SYS_LOG_ERR("error: 0x%x!\n", msg->cmd);
		break;
	}
#endif
}

void system_bluetooth_event_handle(struct app_msg *msg)
{
	switch (msg->cmd) {
#ifdef CONFIG_BT_MANAGER
	case SYS_EVENT_BT_CONNECTED:
	case SYS_EVENT_BT_DISCONNECTED:
	#ifdef CONFIG_SYS_WAKELOCK
		sys_wake_lock(FULL_WAKE_LOCK);
		sys_wake_unlock(FULL_WAKE_LOCK);
	#endif
		break;
#endif
    default:
		//SYS_LOG_ERR("error: 0x%x!\n", msg->cmd);
        break;
	}
}

