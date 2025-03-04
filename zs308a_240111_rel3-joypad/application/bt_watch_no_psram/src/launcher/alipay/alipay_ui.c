/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONFIG_SIMULATOR
#include <soc.h>
#endif
#include <os_common_api.h>
#include <ui_manager.h>
#include <input_manager.h>
#include <property_manager.h>
#include <view_stack.h>
#include <sys_wakelock.h>
#include <app_ui.h>
#include "alipay_ui.h"

#ifdef CONFIG_ALIPAY_LIB
#include <alipay.h>
#include <alipay_vendor.h>
#endif

static bool is_bind_inited = false;
static bool is_barcode_mode = false;
static bool is_screen_locked = false;

#ifndef CONFIG_ALIPAY_LIB
// status counter for simulator
static int status_cnt = 0;
#endif

static void _alipay_bind_init(void)
{
	if (!is_bind_inited) {
#ifdef CONFIG_ALIPAY_LIB
		alipay_env_init();
#endif
		is_bind_inited = true;
	}
}

static void _alipay_bind_deinit(void)
{
	if (is_bind_inited) {
#ifdef CONFIG_ALIPAY_LIB
		alipay_env_deinit();
#endif
		is_bind_inited = false;
	}
}

static void _alipay_do_unbind(void)
{
#ifdef CONFIG_ALIPAY_LIB
	alipay_unbinding();
#else
	status_cnt = 0;
#endif
}

static int _alipay_get_binding_status(void)
{
	int status = STATE_BINDING_OK;

#ifdef CONFIG_ALIPAY_LIB
	int binding_status = alipay_get_binding_status();

	switch (binding_status) {
		case STATUS_UNKNOWN:
		case STATUS_UNBINDED:
			status = STATE_UNBINDED;
			break;

		case STATUS_START_BINDING:
			status = STATE_WAIT_SCAN;
			break;

		case STATUS_GETTING_PROFILE:	//进度10%，进度>0时说明BLE连接已经OK，可以关闭绑定码，显示进度
		case STATUS_SAVING_DATA:		//进度30%
		case STATUS_SAVING_DATA_OK:		//进度70%
		case STATUS_FINISH_BINDING:		//进度90%
		case STATUS_FINISH_BINDING_OK:
			status = STATE_BINDING;
			break;

		case STATUS_BINDING_FAIL:
			status = STATE_BINDING_FAIL;
			break;

		case STATUS_BINDING_OK:
			status = STATE_BINDING_OK;
			break;
	}
#else
	// fake status for simulator
	status_cnt ++;
	if (status_cnt < 10) {
		status = STATE_WAIT_SCAN;
	} else if (status_cnt < 15) {
		status = STATE_BINDING;
	} else {
		status = STATE_BINDING_OK;
	}
#endif
	SYS_LOG_INF("ui binding status: %d", status);

	return status;
}

static int _alipay_get_binding_string(uint8_t *buf, uint32_t *out_len)
{
	int ret = 0;

#ifdef CONFIG_ALIPAY_LIB
	ret = alipay_get_binding_string(buf, out_len);
#else
	strcpy(buf, "https://lvgl.io");
	*out_len = strlen(buf);
#endif
	return ret;
}

static int _alipay_get_paycode_string(uint8_t *buf, uint32_t *out_len)
{
	int ret = 0;

#ifdef CONFIG_ALIPAY_LIB
	ret = alipay_get_paycode(buf, out_len);
#else
	strcpy(buf, "286605383181667759");
	*out_len = strlen(buf);
#endif
	return ret;
}

static int _alipay_get_logon_id(uint8_t *buf, uint32_t *out_len)
{
	int ret = 0;

#ifdef CONFIG_ALIPAY_LIB
	ret = alipay_get_logon_ID(buf, out_len);
#else
	strcpy(buf, "alipay");
	*out_len = strlen(buf);
#endif
	return ret;
}

static int _alipay_get_logon_name(uint8_t *buf, uint32_t *out_len)
{
	int ret = 0;

#ifdef CONFIG_ALIPAY_LIB
	ret = alipay_get_nick_name(buf, out_len);
#else
	strcpy(buf, "alipay@163.com");
	*out_len = strlen(buf);
#endif
	return ret;
}

static bool _alipay_is_barcode_on(void)
{
	return is_barcode_mode;
}

static void _alipay_toggle_barcode(void)
{
	is_barcode_mode = !is_barcode_mode;
}

static void _alipay_lock_screen(void)
{
	if (!is_screen_locked) {
#ifdef CONFIG_SYS_WAKELOCK
		sys_wake_lock(FULL_WAKE_LOCK);
#endif
		is_screen_locked = true;
	}
}

static void _alipay_unlock_screen(void)
{
	if (is_screen_locked) {
#ifdef CONFIG_SYS_WAKELOCK
		sys_wake_unlock(FULL_WAKE_LOCK);
#endif
		is_screen_locked = false;
	}
}

const alipay_view_presenter_t alipay_view_presenter = {
	.bind_init = _alipay_bind_init,
	.bind_deinit = _alipay_bind_deinit,
	.do_unbind = _alipay_do_unbind,
	.get_binding_status = _alipay_get_binding_status,
	.get_binding_string = _alipay_get_binding_string,
	.get_paycode_string = _alipay_get_paycode_string,
	.get_logon_id = _alipay_get_logon_id,
	.get_logon_name = _alipay_get_logon_name,
	.is_barcode_on = _alipay_is_barcode_on,
	.toggle_barcode = _alipay_toggle_barcode,
	.lock_screen = _alipay_lock_screen,
	.unlock_screen = _alipay_unlock_screen,
};

static const uint16_t pay_view_list[] = {
	ALIPAY_PAY_VIEW, ALIPAY_UNBIND_VIEW,
};

static const void *pay_view_presenter_list[] = {
	&alipay_view_presenter, &alipay_view_presenter,
};

static const view_cache_dsc_t pay_view_cache_dsc = {
	.type = PORTRAIT,
	.serial_load = 1,
	.num = ARRAY_SIZE(pay_view_list),
	.vlist = pay_view_list,
	.plist = pay_view_presenter_list,
	.cross_vlist = { VIEW_INVALID_ID, VIEW_INVALID_ID },
	.cross_plist = { NULL, NULL },
};

void alipay_ui_init(void)
{
	int status = _alipay_get_binding_status();

	if (status != STATE_BINDING_OK) {
		_alipay_bind_init();
	}
}

void alipay_ui_enter(void)
{
	int status = _alipay_get_binding_status();

	switch(status) {
		case STATE_BINDING_OK:
			view_stack_push_cache(&pay_view_cache_dsc, ALIPAY_PAY_VIEW);
			break;

		default:
			view_stack_push_view(ALIPAY_BIND_VIEW, &alipay_view_presenter);
			break;
	}
}

void alipay_ui_update(void)
{
	int status = _alipay_get_binding_status();

	switch(status) {
		case STATE_BINDING_OK:
			view_stack_jump_cache(&pay_view_cache_dsc, ALIPAY_PAY_VIEW);
			break;

		default:
			view_stack_jump_view(ALIPAY_BIND_VIEW, &alipay_view_presenter);
			break;
	}
}

