﻿/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system app main
 */
#include <mem_manager.h>
#include <msg_manager.h>
#include <fw_version.h>
#include <sys_event.h>
#include <hotplug_manager.h>
#include <input_manager.h>
#include <thread_timer.h>
#include <stream.h>
#include <property_manager.h>
#include <fs_manager.h>
#include <usb/usb_device.h>
#include <usb/class/usb_msc.h>
#include <soc.h>
#include <sys_wakelock.h>
#include <drivers/usb/usb_phy.h>

struct cardreader_app_t {
	struct thread_timer monitor_timer;
	u32_t card_plugin:1;
	u32_t card_write_flag:1;
	u32_t card_state_change_cnt:8;
};

static void card_reader_moditor_handle(struct thread_timer *ttimer, void *expiry_fn_arg)
{
	struct cardreader_app_t *card_reader = (struct cardreader_app_t *)expiry_fn_arg;
	/**stub mode */
	if (usb_mass_storage_stub_mode()) {
		struct app_msg  msg = {0};
		msg.type = MSG_HOTPLUG_EVENT;
		msg.cmd = HOTPLUG_USB_STUB;
		msg.value = HOTPLUG_IN;
		send_async_msg("main", &msg);
	}

	if (!usb_hotplug_device_mode() || usb_mass_storage_ejected()) {
		struct app_msg  msg = {0};
		msg.type = MSG_HOTPLUG_EVENT;
		msg.cmd = HOTPLUG_USB_DEVICE;
		msg.value = HOTPLUG_OUT;
		send_async_msg("main", &msg);
	}

	if (usb_mass_storage_working()) {
		card_reader->card_write_flag = 1;
		card_reader->card_state_change_cnt = 0;
	} else {
		card_reader->card_state_change_cnt ++;
		if(card_reader->card_state_change_cnt >= 5) {
			card_reader->card_state_change_cnt = 0;
			card_reader->card_write_flag = 0;
			if(card_reader->card_plugin) {

			}
		}
	}
}

#ifdef CONFIG_USB_MASS_STORAGE
#define PAGE_SIZE	CONFIG_MASS_STORAGE_BUF_SIZE
static u8_t data_buf[PAGE_SIZE*2] __aligned(4) ;
#endif

int card_reader_init(void)
{
#ifdef CONFIG_HOTPLUG
#ifdef CONFIG_USB_MASS_STORAGE
	usb_disable();
	usb_phy_init();
	usb_phy_enter_b_idle();
	printk("%s -%d \n",__FUNCTION__,__LINE__);
	usb_mass_storage_init(NULL, data_buf, PAGE_SIZE*2);
#endif

#ifdef CONFIG_FS_MANAGER
	fs_manager_sdcard_enter_high_speed();
#endif
#endif

	return 0;
}

int card_reader_deinit(void)
{
#ifdef CONFIG_USB_MASS_STORAGE
	usb_mass_storage_exit();
#endif
#ifdef CONFIG_FS_MANAGER
	//fs_manager_disk_init("/SD:");
#endif

#ifdef CONFIG_FS_MANAGER
	fs_manager_sdcard_exit_high_speed();
#endif
	return 0;
}

int card_reader_mode_check(void)
{
	struct app_msg msg = {0};
	int result = 0;
	int res = 0;
	bool card_read_init_ok = false;
	bool terminaltion = false;
	bool exit_by_stub = false;
	struct cardreader_app_t *card_reader = NULL;

	if (!usb_hotplug_device_mode()) {
		return 0;
	}

	card_reader = app_mem_malloc(sizeof(struct cardreader_app_t));
	if (!card_reader) {
		return -ENOMEM;
	}

#ifdef CONFIG_THREAD_TIMER
	thread_timer_init(&card_reader->monitor_timer, card_reader_moditor_handle, card_reader);
	thread_timer_start(&card_reader->monitor_timer, 400, 400);
#endif

#ifdef CONFIG_SYS_WAKELOCK
	sys_wake_lock(FULL_WAKE_LOCK);
#endif

	if (!card_reader_init()) {
		card_read_init_ok = true;
	}
#ifdef CONFIG_HOTPLUG
	if (hotplug_manager_get_state(HOTPLUG_SDCARD) == HOTPLUG_IN) {
		card_reader->card_plugin = 1;
	}
#endif

	while (!terminaltion) {
		if (receive_msg(&msg, thread_timer_next_timeout())) {
			switch (msg.type) {
            case MSG_KEY_INPUT:
				if (msg.value == (KEY_POWER | KEY_TYPE_SHORT_UP)
					|| msg.value == (KEY_MENU | KEY_TYPE_SHORT_UP)) {
					if (!card_reader->card_write_flag) {
						terminaltion = true;
					}
				}
				break;
			case MSG_HOTPLUG_EVENT:
				if (msg.value == HOTPLUG_OUT) {
					switch (msg.cmd) {
						case HOTPLUG_USB_DEVICE:
							terminaltion = true;
							break;
						case HOTPLUG_SDCARD:
						#ifdef CONFIG_USB_MASS_STORAGE
                           usb_mass_storage_remove_lun();
                  		#endif
						#ifdef CONFIG_SEG_LED_MANAGER
							if (card_read_init_ok) {
								seg_led_display_icon(SLED_SD, false);
							}
						#endif
							card_reader->card_plugin = 0;
							break;
					}
				} else if (msg.value == HOTPLUG_IN) {
					if (msg.cmd == HOTPLUG_SDCARD) {
					#ifdef CONFIG_SEG_LED_MANAGER
						if (card_read_init_ok) {
							seg_led_display_icon(SLED_SD, true);
						}
					#endif
					#ifdef CONFIG_USB_MASS_STORAGE
						usb_mass_storage_add_lun(MSC_DISK_SD);
					#endif
						card_reader->card_plugin = 1;
					} else if (msg.cmd == HOTPLUG_USB_STUB) {
						terminaltion = true;
						exit_by_stub = true;
					}
				}
				break;
			default:
				SYS_LOG_ERR("error type: 0x%x! \n", msg.type);
				continue;
			}
			if (msg.callback != NULL)
				msg.callback(&msg, result, NULL);
		}
		thread_timer_handle_expired();
	}

	if (card_read_init_ok) {
		card_reader_deinit();
	}

	thread_timer_stop(&card_reader->monitor_timer);


	app_mem_free(card_reader);

#ifdef CONFIG_SYS_WAKELOCK
	sys_wake_unlock(FULL_WAKE_LOCK);
#endif
	return res;

}
