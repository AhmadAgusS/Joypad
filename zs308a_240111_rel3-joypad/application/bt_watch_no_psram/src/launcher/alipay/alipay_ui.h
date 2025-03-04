/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BT_WATCH_SRC_LAUNCHER_ALIPAY_UI_H_
#define BT_WATCH_SRC_LAUNCHER_ALIPAY_UI_H_

#include <msg_manager.h>

enum alipay_bind_state {
	STATE_UNBINDED = 0,
	STATE_WAIT_SCAN,
	STATE_BINDING,
	STATE_BINDING_FAIL,
	STATE_BINDING_OK,
};

typedef struct alipay_view_presenter_s {
	void (*bind_init)(void);
	void (*bind_deinit)(void);
	void (*do_unbind)(void);

	int (*get_binding_status)(void);
	int (*get_binding_string)(uint8_t *buf, uint32_t *out_len);
	int (*get_paycode_string)(uint8_t *buf, uint32_t *out_len);
	int (*get_logon_id)(uint8_t *buf, uint32_t *out_len);
	int (*get_logon_name)(uint8_t *buf, uint32_t *out_len);

	bool (*is_barcode_on)(void);
	void (*toggle_barcode)(void);

	void (*lock_screen)(void);
	void (*unlock_screen)(void);
} alipay_view_presenter_t;

extern const alipay_view_presenter_t alipay_view_presenter;

void alipay_ui_init(void);
void alipay_ui_enter(void);
void alipay_ui_update(void);

#endif /* BT_WATCH_SRC_LAUNCHER_ALIPAY_UI_H_ */

