/*
 * Copyright (c) 2020 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <os_common_api.h>
#include <os_common_api.h> // #include <logging/sys_log.h> #2855
#include <app_switch.h>
#include "app_defines.h"
#include "system_defs.h"


int system_app_launch(const char *default_app)
{
	const char *app_id_list_array[] = APP_ID_LIST;

	if (app_switch_init(app_id_list_array, ARRAY_SIZE(app_id_list_array)))
		return -EINVAL;

#ifdef CONFIG_BT_HID_APP
	app_switch_add_app(APP_ID_BTHID);
#endif

#ifdef CONFIG_BT_SPP_APP
	app_switch_add_app(APP_ID_BTSPP);
#endif

#ifdef CONFIG_USB_HID_APP
	app_switch_add_app(APP_ID_USBHID);
#endif

#ifdef CONFIG_ACTIONS_ATT
	app_switch_add_app(APP_ID_ATT);
#endif

	if (default_app) {
		SYS_LOG_INF("default_app: %s\n", default_app);
		app_switch((void *)default_app, APP_SWITCH_CURR, false);
	}

	return 0;
}
