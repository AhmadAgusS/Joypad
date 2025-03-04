/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __APP_UTILS__
#define __APP_UTILS__

#include <stdbool.h>
#include <app_defines.h>

bool app_send_async_msg(char *appid, int type, int cmd, int value);
bool app_send_async_msg2(char *appid, int type, int cmd, void *bufptr, size_t bufsz);

#endif /* __APP_UTILS__ */
