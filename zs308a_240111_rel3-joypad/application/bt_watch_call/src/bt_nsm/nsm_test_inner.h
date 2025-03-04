/*
 * Copyright (c) 2019 Actions Semiconductor Co, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef __NSM_TEST_INNER_H__
#define __NSM_TEST_INNER_H__

#include <kernel.h>
#include <device.h>
#include <thread_timer.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <app_defines.h>
#include <bt_manager.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <sys_manager.h>
#include <sys_event.h>
#include <spp_test_backend.h>
#include "tool_app.h"
#include "usp_protocol.h"
#include "../tool_app_inner.h"
#include "app_config.h"

#ifdef CONFIG_PROPERTY
#include <property_manager.h>
#endif
#ifdef CONFIG_DVFS
#include <dvfs.h>
#endif
#ifdef CONFIG_BLUETOOTH
#include "mem_manager.h"
#endif

void nsm_init(void);
void nsm_deinit(void);

extern bool nsm_test_backend_load_bt_init(spp_test_notify_cb_t cb, struct ota_backend_bt_init_param *param);
#endif  /* __NSM_TEST_INNER_H__ */

