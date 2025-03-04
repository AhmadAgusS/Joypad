/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system app
 */

#ifndef _SYSTEM_APP_H_
#define _SYSTEM_APP_H_
#include <zephyr.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <srv_manager.h>
#include <app_manager.h>
#include <hotplug_manager.h>
#include <power_manager.h>
#include <input_manager.h>
#include <property_manager.h>
#include <sys_monitor.h>
#include "app_defines.h"
#include <sys_manager.h>
#include <thread_timer.h>
#ifdef CONFIG_UI_MANAGER
#include <ui_manager.h>
#endif
#include <list.h>
#include "app_switch.h"
#include "app_defines.h"
#include "system_util.h"

#define SYS_LOG_NO_NEWLINE
#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN "system_app"
#ifdef SYS_LOG_LEVEL
#undef SYS_LOG_LEVEL
#endif

#ifdef CONFIG_BLUETOOTH
#include "mem_manager.h"
#include "btservice_api.h"
#endif


#endif //_SYSTEM_APP_H_
