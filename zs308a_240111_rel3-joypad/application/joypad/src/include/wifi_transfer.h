/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __WIFI_TRANSFER_H__
#define __WIFI_TRANSFER_H__

#include <stdbool.h>

struct wifi_connect {
	const char *ssid;
	const char *passwd;
};

struct wifi_transfer {
	char *url;
	char *file;
	bool repeat;
	bool upload;
};

int syswifi_init(void);
void syswifi_deinit(void);
int syswifi_connect(struct wifi_connect *info);
void syswifi_disconnect(void);
bool syswifi_is_connected(void);
int syswifi_start_transfer(struct wifi_transfer *transfer);
void syswifi_stop_transfer(void);

#endif /* __WIFI_TRANSFER_H__ */
