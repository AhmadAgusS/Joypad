/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <msg_manager.h>
#include <mem_manager.h>
#include <app_utils.h>
#include <string.h>

bool app_send_async_msg(char *appid, int type, int cmd, int value)
{
	struct app_msg msg = {
		.type = type,
		.cmd = cmd,
		.value = value,
	};

	return send_async_msg((char *)appid, &msg);
}

static void send_async_msg_callback(struct app_msg *msg, int result, void *data)
{
	if (msg && msg->ptr)
		mem_free(msg->ptr);
}

bool app_send_async_msg2(char *appid, int type, int cmd, void *bufptr, size_t bufsz)
{
	struct app_msg msg = {
		.type = type,
		.cmd = cmd,
	};

	if (bufsz == 0) {
		msg.ptr = bufptr;
		return send_async_msg((char *)appid, &msg);
	}

	msg.ptr = mem_malloc(bufsz);
	if (!msg.ptr)
		return false;

	memcpy(msg.ptr, bufptr, bufsz);
	msg.callback = send_async_msg_callback;

	if (false == send_async_msg((char *)appid, &msg)) {
		mem_free(msg.ptr);
		return false;
	}

	return true;
}
