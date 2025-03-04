/*
 * Copyright (c) 2019 Actions Semi Co., Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief sensor service interface
 */
#include <mem_manager.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <msg_manager.h>
#include <sys_manager.h>
#include <thread_timer.h>
#ifdef CONFIG_SYS_WAKELOCK
#include <sys_wakelock.h>
#endif
#include <sensor_manager.h>
#include <sensor_algo.h>
#include <hr_algo.h>
#include "sensor_sleep.h"
#include "sensor_port.h"
#include "algo_port.h"
#if defined(CONFIG_PM)
#include <pm/pm.h>
#endif

#define SYS_LOG_DOMAIN "sensor_service"

#define CONFIG_SENSORSRV_STACKSIZE 4096

#define CONFIG_SENSORSRV_PRIORITY 5

#define MAX_SENSOR_CB	8

static sensor_res_cb_t sensor_res_cb[MAX_SENSOR_CB] = { NULL };

static int _sensor_service_cb_t(int evt_id, sensor_res_t *res)
{
	int idx;
	
	// dump result
	switch(evt_id) {
		case ALGO_ACTIVITY_OUTPUT:
			p_sensor_algo_api->dump_result(res);
			break;
		case ALGO_HANDUP:
			SYS_LOG_INF("handup=%d", res->handup);
#ifdef CONFIG_SYS_WAKELOCK
			if (res->handup == 1) {
				sys_wake_lock(FULL_WAKE_LOCK);
				sys_wake_unlock(FULL_WAKE_LOCK);
			} else if (res->handup == 2) {
				system_request_fast_standby();
			}
#endif
			break;
	}

	// callback
	for (idx = 0; idx < MAX_SENSOR_CB; idx ++) {
		if (sensor_res_cb[idx] != NULL) {
			sensor_res_cb[idx](evt_id, res);
		}
	}

	return 0;
}

static void _sensor_service_init(void)
{
	// init sensor
	sensor_init();
	sensor_sleep_init();

	// init algo
	algo_init(_sensor_service_cb_t);

	// enable sensor
	//p_sensor_algo_api->enable(ALGO_ACTIVITY_OUTPUT, 0);
	//p_sensor_algo_api->enable(IN_HEARTRATE, 0);

	SYS_LOG_INF("init");
}

static void _sensor_service_exit(void)
{
	SYS_LOG_INF("exit");
}

static void _sensor_service_proc(struct app_msg *msg)
{
	sensor_dat_t dat;
	sensor_res_t *res;
	int idx;
	
	SYS_LOG_DBG("sensor cmd %d",msg->cmd);
	switch (msg->cmd) {
	case MSG_SENSOR_DATA:
		while (!sensor_data_is_empty()) {
			if (sensor_get_data(&dat) > 0) {
				algo_handler(dat.id, &dat);
			}
		}
		break;
	
	case MSG_SENSOR_GET_RESULT:
		res = p_sensor_algo_api->get_result();
		memcpy(msg->ptr, res, sizeof(sensor_res_t));
		break;
	
	case MSG_SENSOR_ADD_CB:
		for (idx = 0; idx < MAX_SENSOR_CB; idx ++) {
			if (sensor_res_cb[idx] == NULL) {
				sensor_res_cb[idx] = (sensor_res_cb_t)msg->ptr;
				break;
			}
		}
		break;
	
	case MSG_SENSOR_REMOVE_CB:
		for (idx = 0; idx < MAX_SENSOR_CB; idx ++) {
			if (sensor_res_cb[idx] == (sensor_res_cb_t)msg->ptr) {
				sensor_res_cb[idx] = NULL;
				break;
			}
		}
		break;
	
	case MSG_SENSOR_ENABLE:
#ifdef CONFIG_SENSOR_ALGO_MOTION_SILAN
		if ((msg->reserve == ALGO_ACTIVITY_OUTPUT) || (msg->reserve == ALGO_HANDUP)) {
			msg->reserve = IN_ACC;
		}

		sensor_hal_enable(msg->reserve);

		if (msg->reserve == IN_HEARTRATE) {
			p_hr_algo_api->start(HB);
		}
#else
	#ifdef CONFIG_SENSOR_ALGO_MOTION_CYWEE_DML
		sensor_hal_enable(ID_ACC);
	#endif
		p_sensor_algo_api->enable(msg->reserve, (uint32_t)msg->ptr);
#endif
		break;
	
	case MSG_SENSOR_DISABLE:
#ifdef CONFIG_SENSOR_ALGO_MOTION_SILAN
		if ((msg->reserve == ALGO_ACTIVITY_OUTPUT) || (msg->reserve == ALGO_HANDUP)) {
			msg->reserve = IN_ACC;
		}

		sensor_hal_disable(msg->reserve);

		if (msg->reserve == IN_HEARTRATE) {
			p_hr_algo_api->stop();
		}
#else
		p_sensor_algo_api->disable(msg->reserve, (uint32_t)msg->ptr);
#endif
		break;
	
	default:
		break;
	}

	if (msg->sync_sem) {
		os_sem_give((os_sem *)msg->sync_sem);
	}
}

static void _sensor_service_main_loop(void *parama1, void *parama2, void *parama3)
{
	struct app_msg msg = {0};
	bool terminaltion = false;
	bool suspended = false;
	int timeout;
	int result = 0;

	SYS_LOG_INF("sensor_service enter");

	while (!terminaltion) {
		timeout = suspended ? OS_FOREVER : thread_timer_next_timeout();
		if (receive_msg(&msg, timeout)) {
//			SYS_LOG_INF("sensor_service %d %d",msg.type, msg.cmd);
			switch (msg.type) {
			case MSG_INIT_APP:
				_sensor_service_init();
				break;
			case MSG_EXIT_APP:
				_sensor_service_exit();
				terminaltion = true;
				break;
			case MSG_SENSOR_EVENT:
				_sensor_service_proc(&msg);
				break;
			case MSG_SUSPEND_APP:
				SYS_LOG_INF("SUSPEND_APP");
				sensor_sleep_suspend();
				suspended = true;
				break;
			case MSG_RESUME_APP:
				SYS_LOG_INF("RESUME_APP");
				sensor_sleep_resume();
				suspended = false;
				break;
			default:
				break;
			}
			if (msg.callback) {
				msg.callback(&msg, result, NULL);
			}
		}
		thread_timer_handle_expired();
	}
}

char __aligned(ARCH_STACK_PTR_ALIGN) sensorsrv_stack_area[CONFIG_SENSORSRV_STACKSIZE];

SERVICE_DEFINE(sensor_service, \
				sensorsrv_stack_area, sizeof(sensorsrv_stack_area), \
				CONFIG_SENSORSRV_PRIORITY, BACKGROUND_APP, \
				NULL, NULL, NULL, \
				_sensor_service_main_loop);
