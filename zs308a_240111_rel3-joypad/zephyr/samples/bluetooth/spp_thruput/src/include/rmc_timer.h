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

#ifndef _RMC_TIMER_H_
#define _RMC_TIMER_H_

struct rmc_timer_t {
	struct k_delayed_work rmc_timer_work;
	int8_t timer_id;
};

enum {
	ID_BR_CONNECT_TIMEOUT,
	ID_NUM_TIMEOUTS,
};

#define CONFIG_BR_CONNECT_TIMEOUT		15 //3s

void rmc_timer_stop(int8_t timer_id);

void rmc_timer_start(int8_t timer_id);

int8_t rmc_timer_init(void);

void rmc_timer_event_handle(u32_t timer_event);

#endif /* _RMC_TIMER_H_ */
