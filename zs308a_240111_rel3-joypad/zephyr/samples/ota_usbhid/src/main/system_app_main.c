/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <os_common_api.h>
#include <msg_manager.h>

#include "ota_app.h"	// ota_app_init()
#include <ota_breakpoint.h>

#include "system_defs.h"
#include "app_defines.h"
#include "usb_hid_handler.h"
#include "usbhid_service.h"

/*share stack for app thread */
char __aligned(ARCH_STACK_PTR_ALIGN) share_stack_area[CONFIG_APP_STACKSIZE];

int main(void)
{
	// SYS_LOG_INF("OTA UPGRADE SUCCESSFULLY :3");

	int ret = usbhid_dev_init();
	if (ret) {
		SYS_LOG_ERR("%s, USB HID init failed, ret=%d", __func__, ret);
		return ret;
	}
	
	msg_manager_init();

#ifdef CONFIG_OTA_APP
	struct ota_breakpoint bp;
	ota_breakpoint_load(&bp);
	if (bp.state == OTA_BP_STATE_UPGRADE_WRITING) {
		ota_breakpoint_update_state(&bp, OTA_BP_STATE_WRITING_IMG_FAIL);
	}
	SYS_LOG_INF("%s, init ota app", __func__);
	ota_app_init();
#endif

	system_app_launch(APP_ID_OTA);
	usbhid_service_start();

	while (1) {
		k_sleep(K_MSEC(1000));
	}
	return 0;
}

