/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <os_common_api.h>
#include <stdio.h>
#include <string.h>
#include <file_stream.h>
#include <stream.h>
#include <media_player.h>
#include <audio_system.h>

static io_stream_t open_file_stream(const char *url, int mode)
{
	io_stream_t stream = file_stream_create((void*)url);

	if (!stream)
		return NULL;

	if (stream_open(stream, mode)) {
		stream_destroy(stream);
		return NULL;
	}

	return stream;
}

static void close_stream(io_stream_t stream)
{
	if (stream) {
		stream_close(stream);
		stream_destroy(stream);
	}
}

static io_stream_t mic_in_stream;
static io_stream_t bt_in_stream;
static io_stream_t bt_out_stream;
static io_stream_t spk_out_stream;

void *voice_player_start(const char *mic_in_url, const char *bt_in_url, const char *speaker_out_url, const char *bt_out_url)
{
	media_init_param_t init_param;
	media_player_t *player = NULL;

	memset(&init_param, 0, sizeof(init_param));

	mic_in_stream = open_file_stream(mic_in_url, MODE_IN);
	if (!mic_in_stream) {
		SYS_LOG_ERR("stream open failed (%s)\n", mic_in_url);
		goto err_exit;
	}

	bt_in_stream = open_file_stream(bt_in_url, MODE_IN);
	if (!bt_in_stream) {
		SYS_LOG_ERR("stream open failed (%s)\n", bt_in_url);
		goto err_exit;
	}

	bt_out_stream = open_file_stream(bt_out_url, MODE_OUT);
	if (!bt_out_stream) {
		SYS_LOG_ERR("stream open failed (%s)\n", bt_out_url);
		goto err_exit;
	}

	spk_out_stream = open_file_stream(speaker_out_url, MODE_OUT);
	if (!spk_out_stream) {
		SYS_LOG_ERR("stream open failed (%s)\n", speaker_out_url);
		goto err_exit;
	}

	init_param.type = MEDIA_SRV_TYPE_PLAYBACK_AND_CAPTURE;
	init_param.stream_type = AUDIO_STREAM_VOICE;
	init_param.efx_stream_type = AUDIO_STREAM_VOICE;
	init_param.format = PCM_TYPE;
	init_param.sample_rate = 48;
	init_param.channels = 2;
	init_param.input_stream = bt_in_stream;
	init_param.output_stream = spk_out_stream;
	init_param.capture_format = PCM_TYPE;
	init_param.capture_sample_rate_input = 16;
	init_param.capture_sample_rate_output = 16;
	init_param.capture_channels_input = 1;
	init_param.capture_channels_output = 1;
	init_param.capture_bit_rate = 16;
	init_param.capture_input_stream = mic_in_stream;
	init_param.capture_output_stream = bt_out_stream;
	init_param.dumpable = true;
	init_param.event_notify_handle = NULL;

	SYS_LOG_INF("bt_in_stream %p, spk_out_stream %p, mic_in_stream %p, bt_out_stream %p \n", bt_in_stream, spk_out_stream, mic_in_stream, bt_out_stream);
	player = media_player_open(&init_param);
	if (!player) {
		SYS_LOG_ERR("media_player_open failed\n");
		goto err_exit;
	}

	media_player_play(player);
	return player;
err_exit:
	if (mic_in_stream)
		close_stream(mic_in_stream);
	if (bt_in_stream)
		close_stream(bt_in_stream);
	if (bt_out_stream)
		close_stream(bt_out_stream);
	if (spk_out_stream)
		close_stream(spk_out_stream);
	return NULL;
}

int voice_player_stop(media_player_t *player, bool force)
{
	if (!player || force)
		goto out_exit;

	if (mic_in_stream) {
		int inlen = stream_get_length(bt_in_stream);
		int miclen = stream_get_length(mic_in_stream);
		do {
			os_sleep(1000);
			int outlen = stream_get_length(spk_out_stream);
			int sco_outlen = stream_get_length(bt_out_stream);

			//int newlen = stream_get_length(mic_in_stream);
			SYS_LOG_INF("dec in stream len %d, out stream len %d\n", inlen, outlen);
			SYS_LOG_INF("enc in stream len %d, out stream len %d\n", miclen, sco_outlen);
			if (inlen == outlen && miclen == sco_outlen)
				break;
		} while (1);
	}

out_exit:
	if (player) {
		media_player_stop(player);
		media_player_close(player);
	}

	if (mic_in_stream) {
		close_stream(mic_in_stream);
		mic_in_stream = NULL;
	}
	if (bt_in_stream) {
		close_stream(bt_in_stream);
		bt_in_stream = NULL;
	}
	if (bt_out_stream) {
		close_stream(bt_out_stream);
		bt_out_stream = NULL;
	}
	if (spk_out_stream) {
		close_stream(spk_out_stream);
		spk_out_stream = NULL;
	}
	return 0;
}

