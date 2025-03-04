/*
 * Copyright (c) 2018 Actions Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __USB_HANDLER_H__
#define __USB_HANDLER_H__

enum {
	MSG_USBHID_STARTOTA = 150,
};

typedef void (*system_call_status_flag)(u32_t status_flag);
int usbhid_dev_init(void);
#endif /* __USB_HANDLER_H__ */
