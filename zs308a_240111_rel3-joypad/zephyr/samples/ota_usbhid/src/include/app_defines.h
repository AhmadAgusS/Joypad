/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file app define
 */

#ifndef __APP_DEFINES_H__
#define __APP_DEFINES_H__

#define APP_ID_MAIN				"main"
#define APP_ID_OTA				"ota"
#define APP_ID_USBHID_SERVICE	"usbhid"

#define APP_ID_DEFAULT		APP_ID_OTA

/*
 * app id switch list
 */
#define APP_ID_LIST {\
						APP_ID_OTA, \
					}

extern char share_stack_area[CONFIG_APP_STACKSIZE];

#endif  // __APP_DEFINES_H__
