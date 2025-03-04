/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system audio policy
 */

#include <os_common_api.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <string.h>
#include <audio_system.h>
#include <audio_policy.h>

#define MAX_AUDIO_VOL_LEVEL 32

#ifdef CONFIG_DVFS
#include <dvfs.h>
#endif

#ifdef CONFIG_MEDIA
#if (MAX_AUDIO_VOL_LEVEL == 32)
/* unit: 0.1 dB */
const short music_da_table[MAX_AUDIO_VOL_LEVEL] = {
	-80, -80, -80, -80, -80, -80, -80, -80,
	-80, -80, -80, -80, -80, -80, -80, -80,
	-80, -80, -80, -72, -66, -60, -54, -48,
	-42, -36, -30, -24, -18, -12, -6, 0
};

/* unit: 0.001 dB */
const int music_pa_table[MAX_AUDIO_VOL_LEVEL] = {
	-60000, -52125, -46125, -40875, -36000, -31125, -28125, -25125,
	-22125, -19875, -18000, -16125, -14625, -13125, -11625, -10125,
	-8625, -7125, -5625, -4875, -4500, -4125, -3750, -3375,
	-2625, -2250, -1875, -1500, -1125, -750, -375, 0
};

static const struct audio_policy_t system_audio_policy = {
	.audio_out_channel = AUDIO_CHANNEL_DAC,
	.audio_in_linein_gain = 0, // 0db
	.audio_in_fm_gain = 0, // 0db
	.audio_in_mic_gain = 365, // 36.5db
	.tts_fixed_volume = 1,
	.volume_saveto_nvram = 1,

	.audio_out_volume_level = MAX_AUDIO_VOL_LEVEL - 1,
	.music_da_table = music_da_table,
	.music_pa_table = music_pa_table,
};
#endif
#endif /* CONFIG_MEDIA */

void system_audio_policy_init(void)
{
#ifdef CONFIG_MEDIA
	audio_policy_register(&system_audio_policy);
#endif
}
