/*
 * Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stddef.h>
#include <string.h>
#include <zephyr.h>
#include "msg_manager.h"
#include "rmc_timer.h"
#include "app_utils.h"
#ifdef CONFIG_BT_BREDR
#include "app_br.h"
#endif //CONFIG_BT_BREDR


#if defined CONFIG_BT_BREDR

static struct rmc_timer_t rmc_timer[ID_NUM_TIMEOUTS];

static u32_t rmcTimeoutValue[ID_NUM_TIMEOUTS] =
{
	3000,  /* 3s */
};


static void check_rmc_timer(struct k_work *work)
{
	struct rmc_timer_t * timer = CONTAINER_OF(work, struct rmc_timer_t, rmc_timer_work);
	app_send_async_msg(APP_ID_MAIN, MSG_APP_TIMER, 0, timer->timer_id);
}

void rmc_timer_stop(int8_t timer_id)
{
	k_delayed_work_cancel(&(rmc_timer[timer_id].rmc_timer_work));
}

void rmc_timer_start(int8_t timer_id)
{
	k_delayed_work_submit(&(rmc_timer[timer_id].rmc_timer_work), K_MSEC(rmcTimeoutValue[timer_id]));
}

int8_t rmc_timer_init(void)
{
	int i = 0;
	for (i = 0; i < ID_NUM_TIMEOUTS; i++) {
		rmc_timer[i].timer_id = i;
		k_delayed_work_init(&(rmc_timer[i].rmc_timer_work), check_rmc_timer);
	}
	return 0;
}

void rmc_timer_event_handle(u32_t timer_event)
{
	switch (timer_event) {
	#ifdef CONFIG_BT_BREDR
	case ID_BR_CONNECT_TIMEOUT:
		SYS_LOG_ERR("connect to");
		app_br_create_conn_timeout_handler();
		break;
	#endif //CONFIG_BT_BREDR
	default:
		SYS_LOG_WRN("error: %d", timer_event);
		break;
	}
}

#endif //defined CONFIG_BT_BREDR
