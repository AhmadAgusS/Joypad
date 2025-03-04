/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file app ui define
 */

#ifndef _APP_UI_H_
#define _APP_UI_H_

#include <sys_event.h>
#include <msg_manager.h>
#include <ui_manager.h>
#include <board_cfg.h>

/* default ui resolution */
#define DEF_UI_WIDTH	CONFIG_PANEL_HOR_RES
#define DEF_UI_HEIGHT	CONFIG_PANEL_VER_RES

/* default ui view resolution */
#define DEF_UI_VIEW_WIDTH		CONFIG_PANEL_HOR_RES
#define DEF_UI_VIEW_HEIGHT		CONFIG_PANEL_VER_RES

// default ui file
#define DEF_UI_DISK				CONFIG_APP_UI_DISK
#define DEF_FONT_DISK			CONFIG_APP_FONT_DISK
#define DEF_STY_FILE			DEF_UI_DISK"/bt_watch.sty"
#define DEF_RES_FILE			DEF_UI_DISK"/bt_watch.res"
#define DEF_STR_FILE			DEF_UI_DISK"/bt_watch.zhC"
#define DEF_FONT22_FILE			DEF_FONT_DISK"/sans22.font"
#define DEF_FONT24_FILE			DEF_FONT_DISK"/sans24.font"
#define DEF_FONT28_FILE			DEF_FONT_DISK"/sans28.font"
#define DEF_FONT32_FILE			DEF_FONT_DISK"/sans32.font"
#define DEF_FONT28_EMOJI		DEF_FONT_DISK"/emoji28.font"

#define WELCOME_UI_DISK			CONFIG_WELCOME_UI_DISK
#define WELCOME_STY_FILE		WELCOME_UI_DISK"/logo.sty"
#define WELCOME_RES_FILE		WELCOME_UI_DISK"/logo.res"

enum APP_UI_VIEW_ORDER {
	HIGHEST_ORDER,
	HIGH_ORDER,
	NORMAL_ORDER,
};

enum APP_UI_VIEW_ID {
	TEST_VIEW_1 = VIEW_ID_USER_OFFSET, /* 1 */
	TEST_VIEW_2,
};

#endif /* _APP_UI_H_ */
