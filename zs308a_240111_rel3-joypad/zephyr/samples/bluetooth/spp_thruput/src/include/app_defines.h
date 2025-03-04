/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_DEFINES_H
#define _APP_DEFINES_H

#include <msg_manager.h>
#include <stream.h>

#define APP_ID_MAIN		"main"

enum
{
	MSG_BR_EVENT = MSG_APP_MESSAGE_START,
	MSG_APP_TIMER,
};

#endif  // _APP_DEFINES_H
