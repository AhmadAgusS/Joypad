/*
 * Copyright (c) 2020 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <os_common_api.h>
#include <app_switch.h>
#include "app_defines.h"
#include "system_defs.h"


int system_app_launch(const char *default_app)
{
	const char *app_id_list_array[] = APP_ID_LIST;

	if (app_switch_init(app_id_list_array, ARRAY_SIZE(app_id_list_array)))
		return -EINVAL;

#ifdef CONFIG_OTA_APP
	app_switch_add_app(APP_ID_OTA);
#endif

	if (default_app) {
		SYS_LOG_INF("default_app: %s\n", default_app);
		app_switch((void *)default_app, APP_SWITCH_CURR, false);
	}

	return 0;
}
