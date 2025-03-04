/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <os_common_api.h> // #include <logging/sys_log.h> #2855
#include <input_manager.h>
#include <msg_manager.h>
#include <app_manager.h>
#include <srv_manager.h>
// #include <global_mem.h> // 2855
#include <dvfs.h>
#include <app_defines.h>
#include "system_defs.h"
#include <string.h>
#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>
#include <usb/class/usb_audio.h>
#include <sys/byteorder.h>
#include <drivers/usb/usb_phy.h>
#include "usb_hid_report_desc.h"
#include "usb_hid_descriptor.h"
#include "key_definition.h"

#ifdef CONFIG_PROPERTY
#include "property_manager.h"
#endif

static u8_t usb_hid_out_buf[64];

typedef enum
{
	USB_HID_KEY_TYPE_KEYBOARD = 0,
	USB_HID_KEY_TYPE_MEIDA,
	USB_HID_KEY_TYPE_CUSTOM,
} usb_hid_key_type_e;

struct usb_hid_key_map_t {
	u32_t key_value;
	u8_t usb_hid_value;
	u8_t usb_key_type;
} usb_hid_key_map[] = {
	{KEY_NEXTSONG | KEY_TYPE_SHORT_UP, Num_Lock_code, USB_HID_KEY_TYPE_KEYBOARD},
	{KEY_PREVIOUSSONG | KEY_TYPE_SHORT_UP, Caps_Lock_code, USB_HID_KEY_TYPE_KEYBOARD},
	{KEY_UP_ACTION_BLACK | KEY_TYPE_SHORT_UP, PLAYING_VOLUME_INC, USB_HID_KEY_TYPE_MEIDA},
	{KEY_LEFT_ACTION_WHITE | KEY_TYPE_SHORT_UP, PLAYING_VOLUME_DEC, USB_HID_KEY_TYPE_MEIDA},
};


/* read output report data*/
static void _usb_hid_int_out_ready(void)
{
	u32_t bytes_to_read;
	int ret;

	/* Get all bytes were received */
	ret = hid_int_ep_read(usb_hid_out_buf, sizeof(usb_hid_out_buf),
			&bytes_to_read);
	if (!ret) {
		SYS_LOG_DBG("bytes_to_read:%d", bytes_to_read);
		for (u8_t i = 0; i < bytes_to_read; i++) {
			SYS_LOG_DBG("usb_hid_out_buf[%d] = 0x%02x", i, usb_hid_out_buf[i]);
		}
		return;
	} else {
		SYS_LOG_ERR("read output report error");
		return;
	}
}

static const struct hid_ops ops = {
	.get_report = NULL,
	.get_idle = NULL,
	.get_protocol = NULL,
	.set_report = NULL,
	.set_idle = NULL,
	.set_protocol = NULL,
	.int_in_ready = NULL,
	.int_out_ready = _usb_hid_int_out_ready,
};

static int usb_hid_tx(const u8_t *buf, u16_t len)
{
	u32_t wrote, interval = 0;
	int ret, count = 0;
	u8_t speed_mode = usb_device_speed();

	/* wait one interval at most, unit: 125us */
	if (speed_mode == USB_SPEED_HIGH) {
		count = 1 << (CONFIG_HID_INTERRUPT_EP_INTERVAL_HS - 1);
	} else if (speed_mode == USB_SPEED_FULL || speed_mode == USB_SPEED_LOW) {
		interval = CONFIG_HID_INTERRUPT_EP_INTERVAL_FS;
		count = interval * 20;
	}

	do {
		ret = hid_int_ep_write(buf, len, &wrote);
		if (ret == -EAGAIN) {
			k_busy_wait(125);
		}
	} while ((ret == -EAGAIN) && (--count > 0));

	if (ret) {
		SYS_LOG_ERR("ret: %d", ret);
	} else if (!ret && wrote != len) {
		SYS_LOG_ERR("wrote: %d, len: %d", wrote, len);
	}

	return ret;
}

static int usb_hid_tx_command(u8_t key_code, u8_t key_type)
{
	int len = 0;
	int ret = 0;
	u8_t data[HID_SIZE_KEYBOARD] = {0};

	if (key_type == USB_HID_KEY_TYPE_KEYBOARD) {
		len = HID_SIZE_KEYBOARD;
		data[0] = REPORT_ID_KBD;
		data[3] = key_code;
	} else if (key_type == USB_HID_KEY_TYPE_MEIDA) {
		len = REPORT_ID_MEDIA;
		data[0] = REPORT_ID_MEDIA;
		data[1] = key_code;
	}

	ret = usb_hid_tx(data, len);
	if (ret == 0) {
		memset(data, 0, sizeof(data));
		data[0] = key_type == USB_HID_KEY_TYPE_KEYBOARD? REPORT_ID_KBD:REPORT_ID_MEDIA;
		ret = usb_hid_tx(data, len);
		SYS_LOG_DBG("test_ret: %d\n", ret);
	}

	return ret;
}

static void _usb_hid_upload_num(int key_value)
{
	for(int i = 0; i < sizeof(usb_hid_key_map) / sizeof(struct usb_hid_key_map_t); i++) {
		if (key_value == usb_hid_key_map[i].key_value) {
			usb_hid_tx_command(usb_hid_key_map[i].usb_hid_value, usb_hid_key_map[i].usb_key_type);
			return;
		}
	}
	SYS_LOG_INF("not find key value!\n");
	return;
}

static int _usb_hid_init(void)
{
	int ret = 0;
	usb_phy_init();
	usb_phy_enter_b_idle();

	/* register hid report descriptor */
	usb_hid_register_device(hid_report_desc, sizeof(hid_report_desc), &ops);

	/* Register device descriptors */
	usb_device_register_descriptors(usb_hid_fs_desc, usb_hid_hs_desc);

	/* USB HID initialize */
	ret = usb_hid_init();
	return ret;

}

static int _usb_hid_exit()
{
	usb_hid_exit();

#ifdef CONFIG_PROPERTY
	property_flush(NULL);
#endif

	app_manager_thread_exit(APP_ID_USBHID);
	return 0;
}

static void usb_hid_main_loop(void *p1, void *p2, void *p3)
{
	struct app_msg msg = { 0 };
	bool terminated = false;

	SYS_LOG_INF("USB HID: Enter Loop");

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_set_level(DVFS_LEVEL_HIGH_PERFORMANCE, APP_ID_USBHID);
#endif

	if (_usb_hid_init()) {
		SYS_LOG_ERR("usb hid init err!");
		_usb_hid_exit();
	}

	while (!terminated) {
		if (!receive_msg(&msg, OS_FOREVER))
			continue;

		switch (msg.type) {
		case MSG_KEY_INPUT:
			SYS_LOG_INF("MSG_KEY_INPUT: %x \n",msg.value);
			_usb_hid_upload_num(msg.value);
			break;
		case MSG_EXIT_APP:
			terminated = true;
			_usb_hid_exit();
			break;
		default:
			break;
		}

		if (msg.callback)
			msg.callback(&msg, 0, NULL);
	}

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_unset_level(DVFS_LEVEL_HIGH_PERFORMANCE, APP_ID_USBHID);
#endif

	SYS_LOG_INF("USB HID: Exit APP");
}

APP_DEFINE(usbhid, share_stack_area, sizeof(share_stack_area),
	   APP_PRIORITY, FOREGROUND_APP, NULL, NULL, NULL,
	   usb_hid_main_loop, NULL);
