/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief bt player
 */
#include <audio_system.h>
#include <media_mem.h>
#include "bt_player.h"
#include "tts_manager.h"
#include <bt_manager.h>
#include <ringbuff_stream.h>
#ifdef CONFIG_SYS_WAKELOCK
#include <sys_wakelock.h>
#endif

static uint8_t current_bt_player = 0;
static bool rdm_play_status = 0;

extern void display_song_info(char *song_name, char *album, char *total_time);
extern void btplayer_update_playback_pos(uint32_t cur_time);
#ifdef CONFIG_BLUETOOTH
void bt_player_bt_event_proc(struct app_msg *msg)
{
	SYS_LOG_INF("bt event %d\n", msg->cmd);
	switch (msg->cmd) {
	case BT_CONNECTION_EVENT:
	{
	#ifdef CONFIG_SYS_WAKELOCK
		sys_wake_lock(FULL_WAKE_LOCK);
		sys_wake_unlock(FULL_WAKE_LOCK);
	#endif
		break;
	}
	case BT_DISCONNECTION_EVENT:
	{
	#ifdef CONFIG_SYS_WAKELOCK
		sys_wake_lock(FULL_WAKE_LOCK);
		sys_wake_unlock(FULL_WAKE_LOCK);
	#endif
		break;
	}
	case BT_A2DP_STREAM_START_EVENT:
	{
		SYS_LOG_INF("STREAM_START\n");
		//bt_player_start_play();
#ifdef CONFIG_BT_AVRCP
		bt_manager_avrcp_get_id3_info();
#endif
		current_bt_player = 0x01;
		break;
	}
	case BT_A2DP_STREAM_SUSPEND_EVENT:
	{
		SYS_LOG_INF("STREAM_SUSPEND\n");
		current_bt_player = 0x00;
		break;
	}
	case BT_AVRCP_PLAYBACK_STATUS_CHANGED_EVENT:
	{
#ifdef CONFIG_BT_AVRCP
		int param = *(int *)(msg->ptr);
		if (param == BT_STATUS_PAUSED) {
            rdm_play_status = 0;
		} else if (param == BT_STATUS_PLAYING) {
            rdm_play_status = 1;
		}
#endif
		break;
	}
	case BT_AVRCP_TRACK_CHANGED_EVENT:
	{
#ifdef CONFIG_BT_AVRCP
		bt_manager_avrcp_get_id3_info();
#endif
		break;
	}
	case BT_AVRCP_UPDATE_PLAYBACK_POS:
	{
#ifdef CONFIG_BT_AVRCP
		uint32_t pos = *(uint32_t *)msg->ptr;
		btplayer_update_playback_pos(pos);
#endif
		break;
	}
	case BT_AVRCP_UPDATE_ID3_INFO_EVENT:
	{
#ifdef CONFIG_BT_AVRCP
		struct bt_id3_info *pinfo = (struct bt_id3_info *)msg->ptr;

		display_song_info(pinfo->item[0].data, pinfo->item[1].data, pinfo->item[4].data);
#endif
		break;
	}

	case BT_RMT_VOL_SYNC_EVENT:
	{
		audio_system_set_stream_volume(AUDIO_STREAM_MUSIC, msg->value);
	}
	default:
		break;
	}
}
#endif

void btmusic_view_input_event_proc(struct app_msg *msg)
{
	SYS_LOG_INF("view input event %d\n", msg->cmd);

	switch (msg->cmd) {
	case MSG_BT_PLAY_PAUSE_RESUME:
	{
	#ifdef CONFIG_BT_MANAGER
    #ifdef CONFIG_BT_A2DP
    #ifdef CONFIG_BT_AVRCP
		if (current_bt_player) {
			bt_manager_avrcp_pause();
		} else {
			bt_manager_avrcp_play();
		}
    #endif
        break;
    #endif

	#ifdef CONFIG_BT_AVRCP
		if (rdm_play_status) {
			bt_manager_avrcp_pause();
		} else {
			bt_manager_avrcp_play();
		}
        break;
	#endif

	#endif
		break;
	}

	case MSG_BT_PLAY_NEXT:
	{
	#ifdef CONFIG_BT_MANAGER
    #ifdef CONFIG_BT_AVRCP
		bt_manager_avrcp_play_next();
    #endif
	#endif
		break;
	}

	case MSG_BT_PLAY_PREVIOUS:
	{
	#ifdef CONFIG_BT_MANAGER
    #ifdef CONFIG_BT_AVRCP
		bt_manager_avrcp_play_previous();
    #endif
	#endif
		break;
	}

	case MSG_BT_PLAY_VOLUP:
	{
		system_volume_up(AUDIO_STREAM_MUSIC, 1);
		break;
	}

	case MSG_BT_PLAY_VOLDOWN:
	{
		system_volume_down(AUDIO_STREAM_MUSIC, 1);
		break;
	}

	case MSG_VIEW_VOLSYNC:
	{
		system_volume_set(AUDIO_STREAM_MUSIC, msg->value, 0);
		break;
	}
	default:
		break;
	}
}


void btmusic_start_player(void)
{
    SYS_LOG_INF("%x",(uint32_t)current_bt_player);

#ifdef CONFIG_BT_A2DP
	if (current_bt_player)
		return;
#endif

	struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_BT_PLAY_PAUSE_RESUME,
		.reserve = 1,
	};
	send_async_msg(app_manager_get_current_app(), &msg);
}
void btmusic_stop(void)
{
    SYS_LOG_INF("%x",(uint32_t)current_bt_player);

#ifdef CONFIG_BT_A2DP
	if (!current_bt_player)
		return;
#endif

	struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_BT_PLAY_PAUSE_RESUME,
		.reserve = 1,
	};
	send_async_msg(app_manager_get_current_app(), &msg);
}
void btmusic_play_next(void)
{
	SYS_LOG_INF("\n");

	struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_BT_PLAY_NEXT,
		.reserve = 1,
	};
	send_async_msg(app_manager_get_current_app(), &msg);
}
void btmusic_play_prev(void)
{
	SYS_LOG_INF("\n");

	struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_BT_PLAY_PREVIOUS,
		.reserve = 1,
	};
	send_async_msg(app_manager_get_current_app(), &msg);
}

void btmusic_vol_adjust(bool is_add)
{
	SYS_LOG_INF("is_add %d\n", is_add);

	struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_BT_PLAY_VOLDOWN,
		.reserve = 1,
	};

	if (is_add)
		msg.cmd = MSG_BT_PLAY_VOLUP;

	send_async_msg(app_manager_get_current_app(), &msg);

	SYS_LOG_INF("mode:%d\n", is_add);
}
void btmusic_vol_sync(int music_vol)
{
	struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_VIEW_VOLSYNC,
		.reserve = 1,
		.value = music_vol,
	};

	send_async_msg(app_manager_get_current_app(), &msg);

	SYS_LOG_INF("music_vol:%d\n", music_vol);
}
bool btmusic_get_play_state(void)
{
#ifdef CONFIG_BT_A2DP
	if (current_bt_player)
		return true;
	return false;
#else
    if (rdm_play_status)
        return true;
    return false;

#endif
}
