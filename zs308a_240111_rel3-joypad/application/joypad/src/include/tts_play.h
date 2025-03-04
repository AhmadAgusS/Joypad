/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TTS_PLAY__
#define __TTS_PLAY__

enum {
	TTS_WELCOME = 0,
	TTS_POWEROFF,
	TTS_WAIT_PAIR,
	TTS_BT_CONNECTED,
	TTS_BT_DISCONNECT,
	TTS_BAT_LOW,
	TTS_BAT_TOOLOW,
	TTS_BAT_CHARGING,
	TTS_BAT_CHARGE_FULL,
};

int tts_init(void);
int tts_play(int id);
int tts_stop(int timeout);

#endif /* __TTS_PLAY__ */
