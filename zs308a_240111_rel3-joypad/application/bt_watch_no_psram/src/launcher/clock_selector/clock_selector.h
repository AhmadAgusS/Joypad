/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BT_WATCH_SRC_LAUNCHER_CLOCK_SELECTOR_CLOCK_SELECTOR_H_
#define BT_WATCH_SRC_LAUNCHER_CLOCK_SELECTOR_CLOCK_SELECTOR_H_

typedef struct clock_dsc {
	const char *name;
	uint32_t scene; /* resource scene id */
	uint16_t period; /* time update period */
} clock_dsc_t;

int clocksel_ui_enter(void);

const clock_dsc_t * clocksel_get_clock_dsc(uint8_t index);

const clock_dsc_t * clocksel_get_aod_clock_dsc(void);

#endif /* BT_WATCH_SRC_LAUNCHER_CLOCK_SELECTOR_CLOCK_SELECTOR_H_ */
