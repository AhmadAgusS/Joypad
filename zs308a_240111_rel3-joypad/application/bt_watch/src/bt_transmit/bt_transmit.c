﻿/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief lccal player
 */

#define LOG_MODULE_CUSTOMER

#include <audio_system.h>
#include <audio_policy.h>
#include <ringbuff_stream.h>
#include <os_common_api.h>
#include <media_mem.h>

#include "bt_transmit.h"

LOG_MODULE_REGISTER(bt_transmit, LOG_LEVEL_INF);

typedef struct {
	media_player_t *recorder;
	io_stream_t input_stream;
	io_stream_t output_stream;

	io_stream_t src_stream;
	uint16_t src_sample_rate;
	uint8_t src_channels;

	uint8_t sync_vol_cnt;
	os_delayed_work sync_vol_work;

	uint8_t ready : 1;
} bt_transmit_media_t;

static bt_transmit_media_t bt_transmit;

void bt_transmit_capture_set_ready(bool ready)
{
	bt_transmit.ready = ready;
}

static io_stream_t _bt_transmit_catpure_create_inputstream(void)
{
	int ret = 0;
	io_stream_t input_stream = ringbuff_stream_create_ext(
		media_mem_get_cache_pool(BT_TRANSMIT_INPUT, AUDIO_STREAM_LOCAL_MUSIC),
		media_mem_get_cache_pool_size(BT_TRANSMIT_INPUT, AUDIO_STREAM_LOCAL_MUSIC));

	if (!input_stream) {
		return NULL;
	}

	ret = stream_open(input_stream, MODE_IN_OUT);
	if (ret) {
		stream_destroy(input_stream);
		input_stream = NULL;
	}

	return input_stream;
}

static io_stream_t _bt_transmit_catpure_create_outputstream(void)
{
	int ret = 0;
	io_stream_t output_stream = ringbuff_stream_create_ext(
		media_mem_get_cache_pool(BT_TRANSMIT_OUTPUT, AUDIO_STREAM_LOCAL_MUSIC),
		media_mem_get_cache_pool_size(BT_TRANSMIT_OUTPUT, AUDIO_STREAM_LOCAL_MUSIC));

	if (!output_stream) {
		return NULL;
	}

	ret = stream_open(output_stream, MODE_IN_OUT);
	if (ret) {
		stream_destroy(output_stream);
		output_stream = NULL;
	}

	return 	output_stream;
}

int bt_transmit_sync_vol_to_remote(void)
{
	int max_volume = audio_policy_get_volume_level();
	int volume = audio_system_get_stream_volume(AUDIO_STREAM_LOCAL_MUSIC);
	uint8_t avrcp_vol; /* percentage */

	if (!bt_manager_trs_get_connected_dev_num()) {
		return -ENODEV;
	}

	if (volume <= 0) {
		avrcp_vol = 0;
	} else if (volume >= max_volume) {
		avrcp_vol = 127;
	} else {
		avrcp_vol = (uint8_t)(volume * 127 / max_volume);
	}

	SYS_LOG_INF("volume %d -> remote %d\n", volume, avrcp_vol);

	return bt_manager_avrcp_set_absolute_volume(BTSRV_DEVICE_PLAYER, &avrcp_vol, 1);
}

static void _sync_vol_work_handler(struct k_work *work)
{
	bt_transmit_sync_vol_to_remote();

	if (--bt_transmit.sync_vol_cnt > 0) {
		os_delayed_work_submit(&bt_transmit.sync_vol_work, 1000);
	}
}

void bt_transmit_capture_start_inner(void)
{
	media_init_param_t init_param;
	int ret = 0;

	if (!bt_transmit.ready) {
		SYS_LOG_INF("bt trs not ready!");
		return;
	}

	if (bt_transmit.recorder != NULL) {
		SYS_LOG_WRN("catpure exist!");
		return;
	}

	if (bt_transmit.src_stream == NULL) {
		SYS_LOG_WRN("source stream not exist!");
		return;
	}

	bt_transmit.input_stream = _bt_transmit_catpure_create_inputstream();
	if (!bt_transmit.input_stream) {
		SYS_LOG_ERR("input_stream create failed!");
		return;
	}

	ret = stream_attach(bt_transmit.src_stream, bt_transmit.input_stream, MODE_IN);
	if (ret) {
		SYS_LOG_ERR("input_stream attach failed!");
		goto err_close_input_stream;
	}

	bt_transmit.output_stream = _bt_transmit_catpure_create_outputstream();
	if (!bt_transmit.output_stream) {
		SYS_LOG_ERR("output_stream create failed!");
		stream_detach(bt_transmit.src_stream, bt_transmit.input_stream);
		goto err_detach_input_stream;
	}

	memset(&init_param, 0, sizeof(media_init_param_t));
	init_param.type = MEDIA_SRV_TYPE_CAPTURE;
	init_param.stream_type = AUDIO_STREAM_DEFAULT;
	init_param.capture_format = SBC_TYPE;
	init_param.capture_sample_rate_input = bt_transmit.src_sample_rate;
	init_param.capture_sample_rate_output =
		bt_manager_trs_a2dp_get_sample_rate() == 0 ? 44 : bt_manager_trs_a2dp_get_sample_rate();
	init_param.capture_channels_input = bt_transmit.src_channels;
	init_param.capture_channels_output = 2;
	init_param.capture_bit_rate = bt_manager_a2dp_get_max_bitpool(BTSRV_DEVICE_PLAYER);	//现在dsp里面写死成53了，后面应该由此设置sbc的bitpool
	init_param.capture_input_stream = bt_transmit.input_stream;
	init_param.capture_output_stream = bt_transmit.output_stream;
	init_param.capture_input_src = CAPTURE_INPUT_FROM_DSP_DEC_OUTPUT;

	SYS_LOG_INF("bitpool %d\n", init_param.capture_bit_rate);

	bt_transmit.recorder = media_player_open(&init_param);
	if (!bt_transmit.recorder) {
		SYS_LOG_ERR("media_player_open failed!");
		goto err_close_output_stream;
	}

	bt_manager_trs_set_stream(bt_transmit.output_stream);
	bt_manager_trs_stream_enable(true);

	audio_system_set_stream_mute(AUDIO_STREAM_LOCAL_MUSIC, 1);
	media_player_play(bt_transmit.recorder);

	/* sync first time */
	bt_transmit_sync_vol_to_remote();
	/* sync every 1 second in the last 3 seconds */
	bt_transmit.sync_vol_cnt = 3;
	os_delayed_work_init(&bt_transmit.sync_vol_work, _sync_vol_work_handler);
	os_delayed_work_submit(&bt_transmit.sync_vol_work, 1000);

	SYS_LOG_INF("capture start");
	return;
err_close_output_stream:
	stream_close(bt_transmit.output_stream);
	stream_destroy(bt_transmit.output_stream);
	bt_transmit.output_stream = NULL;
err_detach_input_stream:
	stream_detach(bt_transmit.src_stream, bt_transmit.input_stream);
err_close_input_stream:
	stream_close(bt_transmit.input_stream);
	stream_destroy(bt_transmit.input_stream);
	bt_transmit.input_stream = NULL;
}

static void bt_transmit_catpure_send_mute_tail(void)
{
	char buf[32];

	memset(buf, 0, sizeof(buf));

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 2048 / sizeof(buf); j++) {
			stream_write(bt_transmit.input_stream, buf, sizeof(buf));
		}

		os_sleep(10);
	}

	os_sleep(50);
}

void bt_transmit_capture_stop_inner(void)
{
	if (!bt_transmit.recorder) {
		SYS_LOG_WRN("catpure not exist");
		return;
	}

	stream_detach(bt_transmit.src_stream, bt_transmit.input_stream);

	bt_transmit_catpure_send_mute_tail();
	bt_manager_trs_stream_enable(false);
	bt_manager_trs_set_stream(NULL);

	os_delayed_work_cancel(&bt_transmit.sync_vol_work);
	audio_system_set_stream_mute(AUDIO_STREAM_LOCAL_MUSIC, 0);

	stream_close(bt_transmit.input_stream);
	stream_close(bt_transmit.output_stream);

	media_player_stop(bt_transmit.recorder);
	media_player_close(bt_transmit.recorder);
	bt_transmit.recorder = NULL;

	stream_destroy(bt_transmit.input_stream);
	bt_transmit.input_stream = NULL;

	stream_destroy(bt_transmit.output_stream);
	bt_transmit.output_stream = NULL;

	SYS_LOG_INF("capture stop");
}

void bt_transmit_catpure_start(io_stream_t input_stream, uint16_t sample_rate, uint8_t channels)
{
	bt_transmit.src_stream = input_stream;
	bt_transmit.src_sample_rate = sample_rate;
	bt_transmit.src_channels = channels;

	bt_transmit_capture_start_inner();
}

void bt_transmit_catpure_stop(void)
{
	bt_transmit_capture_stop_inner();
	bt_transmit.src_stream = NULL;
}
