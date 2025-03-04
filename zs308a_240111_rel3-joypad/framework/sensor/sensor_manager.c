/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file sensor manager interface
 */

#define SYS_LOG_NO_NEWLINE
#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN "sensor_manager"
#include <os_common_api.h>
#include <msg_manager.h>
#include <mem_manager.h>
#include <srv_manager.h>
#include <string.h>
#include <sensor_manager.h>

int sensor_send_msg(uint32_t cmd, uint32_t len, void *ptr, uint8_t notify)
{
	struct app_msg msg = {0};
	os_sem return_notify;
	int ret;

	if (notify) {
		os_sem_init(&return_notify, 0, 1);
	}
	
	msg.type = MSG_SENSOR_EVENT;
	msg.cmd = cmd;
	msg.reserve = len;
	msg.ptr = ptr;
	
	if (notify) {
		msg.sync_sem = &return_notify;
	}

	ret = send_async_msg_discardable(SENSOR_SERVICE_NAME, &msg);
	if (false == ret) {
		return -EBUSY;
	}

	if (notify) {
		if (os_sem_take(&return_notify, OS_FOREVER)) {
			return -ETIME;
		}
	}

	return 0;
}

static int _sensor_service_start(void)
{
	struct app_msg msg = {0};

	if (!srv_manager_check_service_is_actived(SENSOR_SERVICE_NAME)) {
		if (srv_manager_active_service(SENSOR_SERVICE_NAME)) {
			SYS_LOG_DBG("sensor service start ok");
		} else {
			SYS_LOG_ERR("sensor service start failed");
			return -ESRCH;
		}
	}

	msg.type = MSG_INIT_APP;

	return !send_async_msg(SENSOR_SERVICE_NAME, &msg);
}

static int _sensor_service_stop(void)
{
	int ret = 0;

	if (!srv_manager_check_service_is_actived(SENSOR_SERVICE_NAME)) {
		SYS_LOG_ERR("sensor service_stop failed");
		ret = -ESRCH;
		goto exit;
	}

	if (!srv_manager_exit_service(SENSOR_SERVICE_NAME)) {
		ret = -ETIMEDOUT;
		goto exit;
	}

	SYS_LOG_DBG("sensor service_stop success!");
exit:
	return ret;
}

int sensor_manager_enable(uint32_t id, uint32_t func)
{
	return sensor_send_msg(MSG_SENSOR_ENABLE, id, (void*)func, 0);
}

int sensor_manager_disable(uint32_t id, uint32_t func)
{
	return sensor_send_msg(MSG_SENSOR_DISABLE, id, (void*)func, 0);
}

int sensor_manager_add_callback(sensor_res_cb_t cb)
{
	return sensor_send_msg(MSG_SENSOR_ADD_CB, 0, cb, 0);
}

int sensor_manager_remove_callback(sensor_res_cb_t cb)
{
	return sensor_send_msg(MSG_SENSOR_REMOVE_CB, 0, cb, 0);
}

int sensor_manager_get_result(sensor_res_t *res)
{
	return sensor_send_msg(MSG_SENSOR_GET_RESULT, sizeof(sensor_res_t), res, 1);
}

int sensor_manager_init(void)
{
	_sensor_service_start();

	return 0;
}

int sensor_manager_exit(void)
{
	_sensor_service_stop();

	return 0;
}


