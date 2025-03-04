/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief bt player.h
 */


#ifndef _MPLAYER_H
#define _MPLAYER_H

#ifdef CONFIG_SYS_LOG
#define SYS_LOG_NO_NEWLINE
#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN "btplayer"
#ifdef SYS_LOG_LEVEL
#undef SYS_LOG_LEVEL
#endif
#define SYS_LOG_LEVEL 4		/* 4, debug */
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stream.h>
#include <file_stream.h>
#include <loop_fstream.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <volume_manager.h>
#include <media_player.h>
#include <audio_system.h>
#include <audio_policy.h>
#include <thread_timer.h>
#include "app_defines.h"
#include "sys_manager.h"
#include "app_ui.h"
#include <fs/fs.h>

#ifdef CONFIG_BLUETOOTH
#include "bt_manager.h"
#endif

enum {
    /* bt play key message */
	MSG_BT_PLAY_PAUSE_RESUME = MSG_BT_APP_MESSAGE_START,
	MSG_BT_PLAY_NEXT,
	MSG_BT_PLAY_PREVIOUS,
	MSG_BT_PLAY_VOLUP,
	MSG_BT_PLAY_VOLDOWN,
	MSG_VIEW_VOLSYNC,
};

struct bt_player_t {
	media_player_t *player;
	io_stream_t bt_stream;
};

struct bt_player_t *bt_player_start_play(void);

void bt_player_stop_play(struct bt_player_t *player);

int bt_player_pause(struct bt_player_t *player);

int bt_player_resume(struct bt_player_t *player);

void bt_player_bt_event_proc(struct app_msg *msg);

#endif  /* _MPLAYER_H */
