/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file app define
 */

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#define APP_ID_MAIN				"main"
#define APP_ID_BTCALL			"btcall"
#define APP_ID_OTA				"ota"
#define APP_ID_RECORDER			"recorder"
#define APP_ID_USOUND			"usound"
#define APP_ID_ALARM			"alarm"
#define APP_ID_ATT				"att"
#define APP_ID_OTA				"ota"
#define APP_ID_WATCH_APPLIST	"watch_applist"
#define APP_ID_MESSAGE			"message"
#define APP_ID_LAUNCHER			"launcher"
#define APP_ID_VENDOR			"vendor"
#define APP_ID_ASET				"aset"
#define APP_ID_DEFAULT			APP_ID_LAUNCHER

/*
 * app id switch list
 */
#define app_id_list {\
						APP_ID_BTCALL,\
						APP_ID_ALARM,\
						APP_ID_ATT,\
						APP_ID_OTA,\
						APP_ID_WATCH_APPLIST,\
						APP_ID_MESSAGE,\
						APP_ID_LAUNCHER,\
					}


extern char share_stack_area[CONFIG_APP_STACKSIZE];

#endif  // _APP_DEFINES_H
