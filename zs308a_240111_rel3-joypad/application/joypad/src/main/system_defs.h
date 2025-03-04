/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SYSTEM_DEFS_H__
#define __SYSTEM_DEFS_H__

#include <drivers/power_supply.h>
#include <usb/class/usb_audio.h>

int system_app_launch(const char *default_app);
void system_audio_policy_init(void);
#endif /* __SYSTEM_DEFS_H__ */
