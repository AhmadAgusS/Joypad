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
// #include <global_mem.h>
#include <dvfs.h>
#include <app_defines.h>
#include "system_defs.h"
#include <bt_manager.h>
#include <btdrv_api.h>

#ifdef CONFIG_PROPERTY
#include "property_manager.h"
#endif

enum {
	TEST_SPP_MODE_IDEL,
	TEST_SPP_MODE_WAIT_CONNECT,
	TEST_SPP_MODE_ACTIVE_CONNECT,
};

static const u8_t test_spp_uuid[16] = {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,	\
										0x00, 0x10, 0x00, 0x00, 0x66, 0x66, 0x00, 0x00};
static u8_t test_spp_mode = TEST_SPP_MODE_IDEL;
static u8_t test_spp_channel;

#define SPP_TEST_SEND_SIZE			600
#define SPP_TEST_WORK_INTERVAL		1		/* 1ms */

static void test_spp_connect_failed_cb(u8_t channel)
{
	SYS_LOG_INF("channel:%d\n", channel);
	if (test_spp_channel == channel) {
		test_spp_channel = 0;
		test_spp_mode = TEST_SPP_MODE_IDEL;
		SYS_LOG_INF("Back to TEST_SPP_MODE_IDEL\n");
	}
}

static void test_spp_connected_cb(u8_t channel, u8_t *uuid)
{
	SYS_LOG_INF("channel:%d\n", channel);
	test_spp_channel = channel;
	test_spp_mode = TEST_SPP_MODE_ACTIVE_CONNECT;
}

static void test_spp_disconnected_cb(u8_t channel)
{
	SYS_LOG_INF("channel:%d\n", channel);
	if (test_spp_channel == channel) {
		test_spp_channel = 0;
		if (test_spp_mode == TEST_SPP_MODE_ACTIVE_CONNECT) {
			test_spp_mode = TEST_SPP_MODE_IDEL;
			SYS_LOG_INF("Back to TEST_SPP_MODE_IDEL\n");
		}
	}
}

static void test_spp_receive_data_cb(u8_t channel, u8_t *data, u32_t len)
{

	printk("Rx: channel:%d ,len %d byte\n", channel, len);

}

static const struct btmgr_spp_cb test_spp_cb = {
	.connect_failed = test_spp_connect_failed_cb,
	.connected = test_spp_connected_cb,
	.disconnected = test_spp_disconnected_cb,
	.receive_data = test_spp_receive_data_cb,
};

static int bt_spp_send_data(void *data, u16_t len)
{
	if (test_spp_channel == 0) {
		SYS_LOG_INF("SPP not connected\n");
	} else {
		bt_manager_spp_send_data(test_spp_channel, data, len);
		SYS_LOG_INF("Send data len %d\n", len);
	}
	return 0;
}

static int _bt_spp_init(void)
{
	if (test_spp_mode != TEST_SPP_MODE_IDEL) {
		SYS_LOG_INF("Not in TEST_SPP_MODE_IDEL mode %d\n", test_spp_mode);
		return 0;
	}


	bt_manager_spp_reg_uuid((u8_t *)test_spp_uuid, (struct btmgr_spp_cb *)&test_spp_cb);
	test_spp_mode = TEST_SPP_MODE_WAIT_CONNECT;
	return 0;
}

static void _bt_spp_exit()
{
	if (test_spp_channel == 0) {
		SYS_LOG_INF("SPP not connected\n");
	} else {
		bt_manager_spp_disconnect(test_spp_channel);
	}

#ifdef CONFIG_PROPERTY
	property_flush(NULL);
#endif

	app_manager_thread_exit(APP_ID_BTSPP);
}

static void bt_spp_main_loop(void *p1, void *p2, void *p3)
{
	struct app_msg msg = { 0 };
	bool terminated = false;

	SYS_LOG_INF("BT SPP: Enter loop");

	if (_bt_spp_init()) {
		SYS_LOG_ERR("bt spp init err\n");
		_bt_spp_exit();
	}

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_set_level(DVFS_LEVEL_HIGH_PERFORMANCE, APP_ID_BTSPP);
#endif

	while (!terminated) {
		if (!receive_msg(&msg, OS_FOREVER))
			continue;

		switch (msg.type) {
		case MSG_KEY_INPUT:
			printk("MSG_KEY_INPUT: %x \n",msg.value);

			bt_spp_send_data(&msg.value, sizeof(msg.value));
			break;
		case MSG_EXIT_APP:
			terminated = true;
			_bt_spp_exit();
			break;

		default:
			break;
		}

		if (msg.callback)
			msg.callback(&msg, 0, NULL);
	}

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_unset_level(DVFS_LEVEL_HIGH_PERFORMANCE, APP_ID_BTSPP);
#endif

	SYS_LOG_INF("BT SPP: Exit APP");
}

APP_DEFINE(btspp, share_stack_area, sizeof(share_stack_area),
	   APP_PRIORITY, FOREGROUND_APP, NULL, NULL, NULL,
	   bt_spp_main_loop, NULL);

