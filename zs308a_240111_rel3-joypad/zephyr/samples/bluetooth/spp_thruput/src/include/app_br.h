/*
 * Copyright (c) 2018 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_BR_H_
#define _APP_BR_H_

#include <zephyr.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* br event */
enum {
	BR_EVENT_NULL,
	BR_START_DISCOVER,
	BR_ACL_CONNECTED_IND,
	BR_ACL_DISCONNECTED_IND,
	BR_SPP_CONNECTED_IND,
	BR_SPP_DISCONNECTED_IND,
};


void app_br_start_discover(void);
void app_br_stop_discover(void);
bool app_br_init(void);
void app_br_event_handle(u8_t event_code, void *event_data);
void app_br_create_conn_timeout_handler(void);
void app_br_tx(u8_t *buf, u16_t buf_len);

#endif //_APP_BR_H_