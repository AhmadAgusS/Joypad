/*
 * Copyright (c) 2015 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <drivers/audio/audio_common.h>

#define BOARD_BATTERY_CAP_MAPPINGS      \
	{0, 3200},  \
	{5, 3300},  \
	{10, 3400}, \
	{20, 3550}, \
	{30, 3650}, \
	{40, 3750}, \
	{50, 3800}, \
	{60, 3850}, \
	{70, 3900}, \
	{80, 3950}, \
	{90, 4000}, \
	{100, 4050},

#define BOARD_ADCKEY_KEY_MAPPINGS	\
	{KEY_ADFU,          0x05}, \
	{KEY_PREVIOUSSONG,	0x376},	\
	{KEY_MENU,		    0x532},	\
	{KEY_TBD,	        0x69C},	\
	{KEY_VOLUMEDOWN,    0x86D}, \
	{KEY_VOLUMEUP,	    0x9EE}, \
	{KEY_NEXTSONG,	    0xB87},


/* @brief The macro to define an audio device */
#define AUDIO_LINE_IN0 (AUDIO_DEV_TYPE_LINEIN | 0)
#define AUDIO_LINE_IN1 (AUDIO_DEV_TYPE_LINEIN | 1)
#define AUDIO_LINE_IN2 (AUDIO_DEV_TYPE_LINEIN | 2)
#define AUDIO_ANALOG_MIC0 (AUDIO_DEV_TYPE_AMIC | 0)
#define AUDIO_ANALOG_MIC1 (AUDIO_DEV_TYPE_AMIC | 1)
#define AUDIO_ANALOG_MIC2 (AUDIO_DEV_TYPE_AMIC | 2)
#define AUDIO_ANALOG_FM0 (AUDIO_DEV_TYPE_FM | 0)
#define AUDIO_DIGITAL_MIC0 (AUDIO_DEV_TYPE_DMIC | 0)
#define AUDIO_DIGITAL_MIC1 (AUDIO_DEV_TYPE_DMIC | 1)

/* @brief Get the mapping relationship between the hardware inputs and audio devices */
int board_audio_device_mapping(audio_input_map_t *input_map);

#endif /* __INC_BOARD_H */
