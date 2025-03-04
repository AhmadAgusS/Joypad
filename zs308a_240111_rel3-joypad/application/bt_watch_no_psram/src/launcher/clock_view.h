/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BT_WATCH_SRC_LAUNCHER_CLOCK_VIEW_H_
#define BT_WATCH_SRC_LAUNCHER_CLOCK_VIEW_H_

#include <drivers/rtc.h>
#include "app_ui.h"

/* weather */
enum {
	SUNNY = 0,
	CLOUDY,
	HAIL,
	SANDSTORM,
	FOG,
	HEAVY_RAIN,
	HEAVY_SNOW,
	THUNDERSTORM,
	CLEAR_TO_OVERCAST,

	NUM_WEATHER_STA,
};

typedef struct clock_view_presenter {
	void (*get_time)(struct rtc_time *time);
	uint32_t (*get_step_count)(void);
	uint32_t (*get_heart_rate)(void);
	uint32_t (*get_calories)(void);
	uint32_t (*get_sleep_time)(void); /* in minutes */
	uint32_t (*get_distance)(void);   /* in meters */
	uint8_t (*get_battery_percent)(void);
	uint8_t (*get_temperature)(void);
	uint8_t (*get_weather)(void);

	uint32_t (*get_clock_id)(void); /* clock scene id */
	int (*invoke_preview)(void);
} clock_view_presenter_t;

#endif /* BT_WATCH_SRC_LAUNCHER_CLOCK_VIEW_H_ */
