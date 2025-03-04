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

static const uint16_t alipay_view_list[] = {
	ALIPAY_PAY_VIEW, ALIPAY_UNBIND_VIEW,
};

static const void *alipay_view_presenter_list[] = {
	&alipay_view_presenter, &alipay_view_presenter,
};

static const view_cache_dsc_t alipay_view_cache_dsc = {
	.type = PORTRAIT,
	.serial_load = 1,
	.num = ARRAY_SIZE(alipay_view_list),
	.vlist = alipay_view_list,
	.plist = alipay_view_presenter_list,
	.cross_vlist = { VIEW_INVALID_ID, VIEW_INVALID_ID },
	.cross_plist = { NULL, NULL },
};

void alipay_ui_sync_time(void)
{
	alipay_view_presenter.sync_time();
}

void alipay_ui_init(void)
{
	alipay_view_presenter.init();
}

void alipay_ui_enter(void)
{
	int status = alipay_view_presenter.get_binding_status();

	switch(status) {
		case STATE_BINDING_OK:
			view_stack_push_cache(&alipay_view_cache_dsc, ALIPAY_PAY_VIEW);
			break;

		default:
			view_stack_push_view(ALIPAY_BIND_VIEW, &alipay_view_presenter);
			break;
	}
}

void alipay_ui_update(void)
{
	int status = alipay_view_presenter.get_binding_status();

	switch(status) {
		case STATE_BINDING_OK:
			view_stack_jump_cache(&alipay_view_cache_dsc, ALIPAY_PAY_VIEW);
			break;

		default:
			view_stack_jump_view(ALIPAY_BIND_VIEW, &alipay_view_presenter);
			break;
	}
}

static const uint16_t wxpay_view_list[] = {
	WXPAY_PAY_VIEW, WXPAY_UNBIND_VIEW,
};

static const void *wxpay_view_presenter_list[] = {
	&wxpay_view_presenter, &wxpay_view_presenter,
};

static const view_cache_dsc_t wxpay_view_cache_dsc = {
	.type = PORTRAIT,
	.serial_load = 1,
	.num = ARRAY_SIZE(wxpay_view_list),
	.vlist = wxpay_view_list,
	.plist = wxpay_view_presenter_list,
	.cross_vlist = { VIEW_INVALID_ID, VIEW_INVALID_ID },
	.cross_plist = { NULL, NULL },
};

void wxpay_ui_init(void)
{
	int status;

	wxpay_view_presenter.init();

	status = wxpay_view_presenter.get_binding_status();
	if (status != STATE_BINDING_OK) {
		wxpay_view_presenter.bind_init();
	}
}

void wxpay_ui_enter(void)
{
	int status = wxpay_view_presenter.get_binding_status();

	switch(status) {
		case STATE_BINDING_OK:
			view_stack_push_cache(&wxpay_view_cache_dsc, WXPAY_PAY_VIEW);
			break;

		default:
			view_stack_push_view(WXPAY_BIND_VIEW, &wxpay_view_presenter);
			break;
	}
}

void wxpay_ui_update(void)
{
	int status = wxpay_view_presenter.get_binding_status();

	switch(status) {
		case STATE_BINDING_OK:
			view_stack_jump_cache(&wxpay_view_cache_dsc, WXPAY_PAY_VIEW);
			break;

		default:
			view_stack_jump_view(WXPAY_BIND_VIEW, &wxpay_view_presenter);
			break;
	}
}

