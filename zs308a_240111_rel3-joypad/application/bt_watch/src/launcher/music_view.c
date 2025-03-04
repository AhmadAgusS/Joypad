/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file lcmusic view
 */
#include <assert.h>
#include <lvgl.h>
#include <lvgl/lvgl_res_loader.h>
#include <view_manager.h>
#include <res_manager_api.h>
#include "app_ui.h"
#include "app_defines.h"
#include "system_app.h"
#include "launcher_app.h"
#include <audio_system.h>
#include <lvgl/lvgl_bitmap_font.h>
#ifdef CONFIG_AUDIO
#include <audio_policy.h>
#endif
#include <volume_manager.h>
#ifdef CONFIG_BT_MANAGER
#include <bt_manager.h>
#ifdef CONFIG_BT_TRANSMIT
#include "../bt_transmit/bt_transmit.h"
#endif
#ifdef CONFIG_BT_PLAYER
#include "../bt_player/bt_player.h"
#endif
#endif
#include "widgets/progress_arc.h"

#define MAX_SUPPORT_CNT	2
#define MUSIC_INFO_UPDATE_PERIOD	300
#define DISPLAY_MORE_VIEW	(1)
enum {
	BTN_PLAY = 0,
	//BTN_STOP,
	BTN_PRE,
	BTN_NEXT,
	BTN_SEQ_PLAY_M,
	//BTN_LIST_LPLAY_M,
	//BTN_RAND_PLAY_M,
	//BTN_SING_LOOP_PLAY_M,
	BTN_VOL,
#if DISPLAY_MORE_VIEW
	BTN_MORE,
#endif
	BTN_BMP_MUSIC_SOURCE,

	NUM_BTNS,
};
enum {
	BMP_BG = 0,
	NUM_IMGS,
};
enum {
	TXT_SONG_NAME = 0,
	TXT_ALBUM,
	NUM_TXTS,
};

enum {
	BTN_VOLUP = 0,
	BTN_VOLDOWN,

	NUM_VOL_BTNS,
};

enum {
	BTN_WATCH= 0,
#if DEF_UI_WIDTH >= 454
	BTN_WATCH_TXT,
	BTN_WATCH_CH,
#endif
	BTN_PHONE,
#if DEF_UI_WIDTH >= 454
	BTN_PHONE_TXT,
	BTN_PHONE_CH,
#endif
	BTN_BT_EARPHONE,
#if DEF_UI_WIDTH >= 454
	BTN_BT_EARPHONE_TXT,
	BTN_BT_EARPHONE_CH,
#endif
	NUM_MORE_BTNS,
};
enum {
	BMP_BT_CONNECTED = 0,
	BMP_BT_DISCONNECT,

	NUM_BT_STATE_IMGS,
};
enum {
	TXT_BT_EARPHONE_LIST_NAME = 0,
	TXT_BT_EARPHONE_NAME1,
	TXT_BT_EARPHONE_NAME1_CNT_STATE,
	TXT_BT_EARPHONE_NAME2,
	TXT_BT_EARPHONE_NAME2_CNT_STATE,

	NUM_BT_EARPHONE_LIST_TXTS,
};

enum {
	BTN_BT_EARPHONE_UPDATE = 0,

	NUM_BT_EARPHONE_LIST_BTNS,
};

const static uint32_t _bmp_ids[] = {
	PIC_BG,
};

#if DISPLAY_MORE_VIEW
const static uint32_t bt_state_bmp_ids[] = {
	PIC_BT_CONNECTED,
	PIC_BT_DISCONNECT,
};
#endif

const static uint32_t _btn_def_ids[] = {
	PIC_BTPL,
	PIC_BTPRE,
	PIC_BTNEXT,
	//PIC_BTSPM,
	PIC_BTLLPM,
	//PIC_BTRPM,
	//PIC_BTSLPM,
	PIC_BTVOL,
#if DISPLAY_MORE_VIEW
	PIC_BTMORE,
#endif
	PIC_WATCH_P,
};

const static uint32_t _btn_sel_ids[] = {
	PIC_BTSP,
	PIC_BTPRE,
	PIC_BTNEXT,
	//PIC_BTSPM,
	//PIC_BTLLPM,
	//PIC_BTRPM,
	PIC_BTSLPM,
	PIC_BTVOL,
#if DISPLAY_MORE_VIEW
	PIC_BTMORE,
#endif
	PIC_PHONE_P,
};

const static uint32_t vol_btn_def_ids[] = {
	PIC_BTN_VOLUP,
	PIC_BTN_VOLDOWN,
};

#if DISPLAY_MORE_VIEW
const static uint32_t more_btn_def_ids[] = {
	PIC_BTN_WATCH,
#if DEF_UI_WIDTH >= 454
	PIC_BTN_WATCH_TXT,
	PIC_BTN_WATCH_UNCH,
#endif
	PIC_BTN_PHONE,
#if DEF_UI_WIDTH >= 454
	PIC_BTN_PHONE_TXT,
	PIC_BTN_PHONE_UNCH,
#endif
	PIC_BTN_BT,
#if DEF_UI_WIDTH >= 454
	PIC_BTN_BT_TXT,
	PIC_BTN_BT_UNCH,
#endif
};
#endif

#ifdef CONFIG_BT_TRANSMIT
const static uint32_t bt_earphone_update_btn_ids[] = {
	PIC_BTN_BT_EARPHONE_UPDATE,
};
#endif

const static uint32_t _txt_ids[] = {
	STR_SN,	STR_ALBUM,
};

#if DISPLAY_MORE_VIEW
const static uint32_t more_btn_sel_ids[] = {
	PIC_BTN_WATCH,
#if DEF_UI_WIDTH >= 454
	PIC_BTN_WATCH_TXT,
	PIC_BTN_WATCH_CH,
#endif
	PIC_BTN_PHONE,
#if DEF_UI_WIDTH >= 454
	PIC_BTN_PHONE_TXT,
	PIC_BTN_PHONE_CH,
#endif
	PIC_BTN_BT,
#if DEF_UI_WIDTH >= 454
	PIC_BTN_BT_TXT,
	PIC_BTN_BT_CH,
#endif
};
#endif

#ifdef CONFIG_BT_TRANSMIT
const static uint32_t bt_earphone_list_txt_ids[] = {
	STR_BT_EARPHONE_LIST_NAME,
	STR_BT_EARPHONE_NAME1,
	STR_BT_EARPHONE_NAME1_CNT_STATE,
	STR_BT_EARPHONE_NAME2,
	STR_BT_EARPHONE_NAME2_CNT_STATE,
};
#endif

const static uint32_t music_resource_id[] = {
	PIC_BG,
	PIC_WATCH_P,
	PIC_BTPL,
	PIC_BTPRE,
	PIC_BTNEXT,
	PIC_BTLLPM,
	//PIC_BTSPM,
	PIC_BTVOL,
	PIC_BTMORE,
	STR_SN,
	STR_ALBUM,
};

static int32_t music_preload_inited = 0;

typedef struct lcmusic_view_data {
	/* lvgl object */
	lv_obj_t *btn[NUM_BTNS];
	lv_obj_t *bmp[NUM_IMGS];
	lv_obj_t *lbl[NUM_TXTS];
	lv_obj_t *bt_state;
	/*volume adjust*/
	lv_obj_t *vol_btn[NUM_VOL_BTNS];
	lv_obj_t *vol_bmp;
	lv_obj_t *vol_slider;
	lv_obj_t *progress_arc;
	/*more set*/
	lv_obj_t *more_btn[NUM_MORE_BTNS];
	lv_obj_t *more_bmp;
	/*bt  list*/
#ifdef CONFIG_BT_TRANSMIT
	lv_obj_t *bt_earphone_name[NUM_BT_EARPHONE_LIST_TXTS];
	lv_obj_t *bt_earphone_update_btn[NUM_BT_EARPHONE_LIST_BTNS];
	lv_obj_t *bt_earphone_list_bg_img;
#endif
	/* ui-editor resource */
	lvgl_res_scene_t res_scene;
	lvgl_res_string_t res_txt[NUM_TXTS];
#ifdef CONFIG_BT_TRANSMIT
	lvgl_res_string_t res_txt_bt_earphone_list[NUM_BT_EARPHONE_LIST_TXTS];
#endif
	/* lvgl resource */
	lv_point_t pt_def[NUM_BTNS];
	lv_point_t pt_sel[NUM_BTNS];
	lv_point_t pt_bmp[NUM_IMGS];
	lv_point_t pt_txt[NUM_TXTS];
	lv_point_t pt_vol_btn[NUM_VOL_BTNS];
	lv_point_t pt_more_btn[NUM_MORE_BTNS];
	lv_point_t pt_bt_state_bmp[NUM_BT_STATE_IMGS];
#ifdef CONFIG_BT_TRANSMIT
	lv_point_t pt_bt_earphone_list_txt[NUM_BT_EARPHONE_LIST_TXTS];
	lv_point_t pt_bt_earphone_update_btn[NUM_BT_EARPHONE_LIST_BTNS];
#endif
	lv_img_dsc_t img_dsc_def[NUM_BTNS];
	lv_img_dsc_t img_dsc_sel[NUM_BTNS];
	lv_img_dsc_t img_dsc_bmp[NUM_IMGS];
	lv_img_dsc_t img_dsc_vol_btn[NUM_VOL_BTNS];
	lv_img_dsc_t img_dsc_more_btn[NUM_MORE_BTNS];
	lv_img_dsc_t img_dsc_more_btn_sel[NUM_MORE_BTNS];
	lv_img_dsc_t img_dsc_bt_state_bmp[NUM_BT_STATE_IMGS];
#ifdef CONFIG_BT_TRANSMIT
	lv_img_dsc_t img_dsc_bt_earphone_update_btn[NUM_BT_EARPHONE_LIST_BTNS];
#endif
	lv_style_t style_txt[NUM_TXTS];
#ifdef CONFIG_BT_TRANSMIT
	lv_style_t style_txt_bt_earphone_list[NUM_BT_EARPHONE_LIST_TXTS];
#endif
	/*slider style*/
	lv_style_t style_slider_bg;
	lv_style_t style_slider_indic;
	lv_style_t style_slider_knob;
	/*arc style*/
	lv_style_t style_arc_bg;
	lv_style_t style_arc_red;

	lv_font_t font;
	/* user data */
	uint32_t update_song_info : 1;/*0--don't need update,1--need to update*/
	uint32_t late_update_song_info;
	uint32_t filter_update_pstate : 3;/*0--update play state,other--filter update play state one time*/
	uint32_t get_playback_pos : 8;/*0--get music playback pos */
#ifdef CONFIG_BT_TRANSMIT
	uint32_t update_bt_earphone_list : 1;/*0--don't need update,1--need to update*/
#endif
	uint32_t volume_need_sync : 1; /* volume changed in the other place, need sync to view */
	uint32_t song_time;/*unit :ms*/
	uint32_t song_cur_time;/*unit :ms*/
	char *song_name;
	char *album;
	bool is_switching;
#ifdef CONFIG_BT_TRANSMIT
	struct bt_device_info_t bt_earphone_info[MAX_SUPPORT_CNT];
#endif
	lv_timer_t *timer;
} lcmusic_view_data_t;

static uint8_t p_music_player = 0;/*0---lcmusic ,1 ---btmusic player*/

lcmusic_view_data_t *p_lview_data;
extern void lcmusic_start_player(void);
extern void lcmusic_stop(void);
extern void lcmusic_play_or_pause(void);

extern void lcmusic_play_next(void);
extern void lcmusic_play_prev(void);
extern void lcmusic_set_play_mode(void);
extern void lcmusic_get_song_name(char **song_name, uint16_t *strlen);
extern void lcmusic_vol_adjust(bool is_add);
extern void lcmusic_vol_sync(int music_vol);
extern void lcmusic_get_song_bp(uint32_t *total_time, int *cur_time);
extern bool lcmusic_is_playing(void);
extern uint8_t lcmusic_get_play_mode(void);
extern void lcmusic_seek(int seek_time, int cur_time, bool relative_seek);

#ifdef CONFIG_BT_TRANSMIT
static void _display_bt_earphone_list(view_data_t *view_data);
static void _bt_earphone_list_info_update(struct bt_device_info_t *bt_info, uint8_t count);
#endif

#if DISPLAY_MORE_VIEW
static void _display_more_view(view_data_t *view_data);
#endif

static void _display_vol_view(view_data_t * view_data);

void lcmusic_display_song_info(char *song_info, bool direct_update);

#ifdef CONFIG_BT_PLAYER
static void _sync_btmusic_play_info(const char *song_name, const char *album, uint32_t total_time);
static void _sync_btmusic_play_pos(uint32_t cur_time);
static void _sync_btmusic_volume(int volume);

static const btmusic_callback_t btmusic_callback = {
	.set_play_info = _sync_btmusic_play_info,
	.set_play_pos = _sync_btmusic_play_pos,
	.set_volume = _sync_btmusic_volume,
};
#endif

static OS_MUTEX_DEFINE(music_view_mutex);

void lcmusic_restore_music_player_mode(launcher_app_t *app)
{
	if (app)
		app->cur_player = p_music_player;
}

static void _delete_obj_array(lv_obj_t **pobj, uint32_t num)
{
	for (int i = 0; i < num; i++) {
		if (pobj[i]) {
			lv_obj_del(pobj[i]);
			pobj[i] = NULL;
		}
	}
}
static void _reset_label_style_array(lv_style_t *sty, uint32_t num)
{
	for (int i = 0; i < num; i++) {
		lv_style_reset(&sty[i]);
	}
}

static void btn_state_toggle(lv_obj_t * btn)
{
	if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
		lv_obj_clear_state(btn, LV_STATE_CHECKED);
	} else {
		lv_obj_add_state(btn, LV_STATE_CHECKED);
	}
}

static void _exit_more_view(void)
{
	if (p_lview_data->bt_state) {
		lv_obj_del(p_lview_data->bt_state);
		p_lview_data->bt_state = NULL;
	}
	_delete_obj_array(p_lview_data->more_btn, NUM_MORE_BTNS);
	if (p_lview_data->more_bmp) {
		lv_obj_del(p_lview_data->more_bmp);
		p_lview_data->more_bmp = NULL;
	}

	lvgl_res_unload_pictures(p_lview_data->img_dsc_more_btn, NUM_MORE_BTNS);
	lvgl_res_unload_pictures(p_lview_data->img_dsc_more_btn_sel, NUM_MORE_BTNS);
	lvgl_res_unload_pictures(p_lview_data->img_dsc_bt_state_bmp, NUM_BT_STATE_IMGS);
	ui_manager_gesture_set_dir(GESTURE_ALL_BITFIELD);
}

static void _exit_vol_view(void)
{
	/*delete vol bar*/
	if (p_lview_data->vol_slider) {
		lv_obj_del(p_lview_data->vol_slider);
		p_lview_data->vol_slider = NULL;
	}
	lv_style_reset(&p_lview_data->style_slider_bg);
	lv_style_reset(&p_lview_data->style_slider_indic);
	lv_style_reset(&p_lview_data->style_slider_knob);

	/*delete vol btn*/
	_delete_obj_array(p_lview_data->vol_btn, NUM_VOL_BTNS);

	/*delete vol bg*/
	if (p_lview_data->vol_bmp) {
		lv_obj_del(p_lview_data->vol_bmp);
		p_lview_data->vol_bmp = NULL;
	}

	lvgl_res_unload_pictures(p_lview_data->img_dsc_vol_btn, NUM_VOL_BTNS);

	ui_manager_gesture_wait_release();
	ui_manager_gesture_set_dir(GESTURE_ALL_BITFIELD);
}

#ifdef CONFIG_BT_TRANSMIT
static void _exit_bt_earphone_list_view(void)
{
	_delete_obj_array(p_lview_data->bt_earphone_name, NUM_BT_EARPHONE_LIST_TXTS);
	_reset_label_style_array(p_lview_data->style_txt_bt_earphone_list, NUM_BT_EARPHONE_LIST_TXTS);
	_delete_obj_array(p_lview_data->bt_earphone_update_btn, NUM_BT_EARPHONE_LIST_BTNS);
	if (p_lview_data->bt_earphone_list_bg_img) {
		lv_obj_del(p_lview_data->bt_earphone_list_bg_img);
		p_lview_data->bt_earphone_list_bg_img = NULL;
	}
	lvgl_res_unload_pictures(p_lview_data->img_dsc_bt_earphone_update_btn, NUM_BT_EARPHONE_LIST_BTNS);
	lvgl_res_unload_strings(p_lview_data->res_txt_bt_earphone_list, NUM_BT_EARPHONE_LIST_TXTS);

	ui_manager_gesture_wait_release();
	ui_manager_gesture_set_dir(GESTURE_ALL_BITFIELD);

	os_mutex_lock(&music_view_mutex, OS_FOREVER);
	p_lview_data->update_bt_earphone_list = 0;
	os_mutex_unlock(&music_view_mutex);
}
#endif

static void _bmp_event_handler(lv_event_t * e)
{
	lv_event_code_t event = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_current_target(e);
#ifdef CONFIG_BT_TRANSMIT
	view_data_t *view_data = lv_event_get_user_data(e);
#endif

	if (event == LV_EVENT_GESTURE) {
		uint8_t gesture = lv_indev_get_gesture_dir(lv_event_get_param(e));
		if (p_lview_data && p_lview_data->vol_bmp == obj && (gesture & LV_DIR_HOR)) {
			_exit_vol_view();
			lv_indev_wait_release(lv_indev_get_act());
		#ifdef CONFIG_BT_TRANSMIT
		} else if (p_lview_data && p_lview_data->bt_earphone_list_bg_img == obj) {
			_exit_bt_earphone_list_view();
			lv_indev_wait_release(lv_indev_get_act());
			_display_more_view(view_data);
		#endif
		}
	} else if (event == LV_EVENT_VALUE_CHANGED) {
		SYS_LOG_INF("%p Toggled\n", obj);
	}

}

static void _slider_event_handle(lv_event_t * e)
{
	uint8_t player = launcher_get_cur_player();
	int vol;

	if (!p_lview_data || !p_lview_data->vol_slider)
		return;

	vol = lv_slider_get_value(p_lview_data->vol_slider);

	if (player == LCMUSIC_PLAYER) {
		lcmusic_vol_sync(vol);
	} else if (player == BTMUSIC_PLAYER) {
		btmusic_vol_sync(vol);
	}

	p_lview_data->volume_need_sync = 0;
	SYS_LOG_DBG("slider volume %d\n", vol);
}

#ifdef CONFIG_LCMUSIC_APP
static void _music_play_view(uint8_t cur_player, uint8_t prev_player)
{
	uint8_t btn_state;

	btn_state_toggle(p_lview_data->btn[BTN_BMP_MUSIC_SOURCE]);
	lv_obj_invalidate(p_lview_data->btn[BTN_BMP_MUSIC_SOURCE]);

	lv_label_set_text(p_lview_data->lbl[TXT_SONG_NAME], "");
	lv_label_set_text(p_lview_data->lbl[TXT_ALBUM], "");

	if (cur_player == BTMUSIC_PLAYER) {
		lv_obj_add_flag(p_lview_data->btn[BTN_SEQ_PLAY_M], LV_OBJ_FLAG_HIDDEN);
		lcmusic_stop();
	#ifdef CONFIG_BT_PLAYER
		btmusic_check_remote_state();
	#endif
	} else {
		lv_obj_clear_flag(p_lview_data->btn[BTN_SEQ_PLAY_M], LV_OBJ_FLAG_HIDDEN);
	#ifdef CONFIG_BT_PLAYER
		btmusic_stop(true);
	#endif
		lcmusic_display_song_info(NULL, true);
	}

	btn_state = lv_obj_get_state(p_lview_data->btn[BTN_PLAY]);
	if (btn_state == LV_STATE_PRESSED || btn_state == LV_STATE_CHECKED) {
		btn_state_toggle(p_lview_data->btn[BTN_PLAY]);
		lv_obj_invalidate(p_lview_data->btn[BTN_PLAY]);
	}
}
#endif

#ifdef CONFIG_BT_TRANSMIT
static void _bt_earphone_event_handler(lv_event_t * e)
{
	lv_event_code_t event = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (!p_lview_data)
		return;

	if (event == LV_EVENT_SHORT_CLICKED) {
		lv_obj_set_user_data(obj, (void *)1);
	} else if (event == LV_EVENT_CLICKED) {
		bool clear_pairing = true;

		if (lv_obj_get_user_data(obj) != NULL) { /* long click */
			lv_obj_set_user_data(obj, NULL);
			clear_pairing = false;
		}

		lv_obj_t *name_objs[] = {
			p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1],
			p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2],
		};

		os_mutex_lock(&music_view_mutex, OS_FOREVER);

		for (int i = 1; i >= 0; i--) {
			if (obj != name_objs[i] || !p_lview_data->bt_earphone_info[i].name)
				continue;

			if (p_lview_data->bt_earphone_info[i].connected) {
				SYS_LOG_INF("disconnect %s", p_lview_data->bt_earphone_info[i].name);
				bt_manager_disconnect_device(BTSRV_DISCONNECT_TRS_MODE);		/* Or disconnect device by address */
				//bt_manager_br_disconnect(&p_lview_data->bt_earphone_info[i].addr);
			} else if (clear_pairing == false) {
				SYS_LOG_INF("start connect %s", p_lview_data->bt_earphone_info[i].name);
				bt_manager_trs_start_connect(p_lview_data->bt_earphone_info[i].addr.val);
			}

			if (clear_pairing) {
				SYS_LOG_INF("clear pairing %s", p_lview_data->bt_earphone_info[i].name);
				bt_manager_clear_trs_dev_info(&p_lview_data->bt_earphone_info[i].addr);
			}
		}

		os_mutex_unlock(&music_view_mutex);
	}
}
#endif /* CONFIG_BT_TRANSMIT */

static void _btn_event_handler(lv_event_t * e)
{
	launcher_app_t *app = launcher_app_get();
	lv_obj_t *obj = lv_event_get_target(e);
	uint8_t btn_state = 0;
	view_data_t *view_data = lv_event_get_user_data(e);

	if (!app || !p_lview_data)
		return;

	SYS_LOG_INF("%p Clicked\n", obj);

#ifdef CONFIG_LCMUSIC_APP
	if (p_lview_data->btn[BTN_PLAY] == obj) {
		btn_state = lv_obj_get_state(p_lview_data->btn[BTN_PLAY]);
		p_lview_data->filter_update_pstate = 600 / MUSIC_INFO_UPDATE_PERIOD;
		if (btn_state == LV_STATE_PRESSED || ((btn_state & (LV_STATE_CHECKED | LV_STATE_PRESSED)) == LV_STATE_CHECKED)) {
			if (app->cur_player == LCMUSIC_PLAYER) {
				lcmusic_start_player();
			} else if (app->cur_player == BTMUSIC_PLAYER) {
				btmusic_start();
			}
		} else {
			if (app->cur_player == LCMUSIC_PLAYER) {
				lcmusic_stop();
			} else if (app->cur_player == BTMUSIC_PLAYER) {
				btmusic_stop(false);
			}
		}

		/* use the feedback play state by phone */
		if (app->cur_player == BTMUSIC_PLAYER) {
			btn_state_toggle(p_lview_data->btn[BTN_PLAY]);
			lv_obj_invalidate(p_lview_data->btn[BTN_PLAY]);
		}
	} else if (p_lview_data->btn[BTN_PRE] == obj) {
		if (app->cur_player == LCMUSIC_PLAYER) {
			if(!p_lview_data->is_switching)
			{
				lcmusic_play_prev();
				p_lview_data->is_switching = true;
			}
		} else if (app->cur_player == BTMUSIC_PLAYER) {
			btmusic_play_prev();
		}

		btn_state = lv_obj_get_state(p_lview_data->btn[BTN_PLAY]);
		if ((btn_state & (LV_STATE_PRESSED | LV_STATE_CHECKED)) == 0 || btn_state == (LV_STATE_PRESSED | LV_STATE_CHECKED)) {
			btn_state_toggle(p_lview_data->btn[BTN_PLAY]);
			lv_obj_invalidate(p_lview_data->btn[BTN_PLAY]);
		}
	} else if (p_lview_data->btn[BTN_NEXT] == obj) {
		if (app->cur_player == LCMUSIC_PLAYER) {
			if(!p_lview_data->is_switching)
			{
				lcmusic_play_next();
				p_lview_data->is_switching = true;
			}		
		} else if (app->cur_player == BTMUSIC_PLAYER) {
			btmusic_play_next();
		}

		btn_state = lv_obj_get_state(p_lview_data->btn[BTN_PLAY]);
		if ((btn_state & (LV_STATE_PRESSED | LV_STATE_CHECKED)) == 0 || btn_state == (LV_STATE_PRESSED | LV_STATE_CHECKED)) {
			btn_state_toggle(p_lview_data->btn[BTN_PLAY]);
			lv_obj_invalidate(p_lview_data->btn[BTN_PLAY]);
		}
	} else if (p_lview_data->btn[BTN_SEQ_PLAY_M] == obj) {
		if (app->cur_player == LCMUSIC_PLAYER)
			lcmusic_set_play_mode();
	} else if (p_lview_data->btn[BTN_VOL] == obj) {
		_display_vol_view(view_data);
	} else if (p_lview_data->vol_btn[BTN_VOLUP] == obj) {
		/* adjust vol bar and mechine vol */
		if (app->cur_player == LCMUSIC_PLAYER) {
			int maxvol = lv_slider_get_max_value(p_lview_data->vol_slider);
			lcmusic_vol_sync(maxvol);
		} else if (app->cur_player == BTMUSIC_PLAYER) {
		#ifdef CONFIG_BT_PLAYER
			int maxvol = lv_slider_get_max_value(p_lview_data->vol_slider);
			btmusic_vol_sync(maxvol);
		#endif
		}
		//lv_slider_set_value(p_lview_data->vol_slider, lv_slider_get_value(p_lview_data->vol_slider) + 1, LV_ANIM_OFF);
		lv_slider_set_value(p_lview_data->vol_slider, 15, LV_ANIM_OFF);
	} else if (p_lview_data->vol_btn[BTN_VOLDOWN] == obj) {
		/* adjust vol bar and mechine vol */
		if (app->cur_player == LCMUSIC_PLAYER) {
			int minvol = lv_slider_get_min_value(p_lview_data->vol_slider);
			lcmusic_vol_sync(minvol);
		} else if (app->cur_player == BTMUSIC_PLAYER) {
		#ifdef CONFIG_BT_PLAYER
			int minvol = lv_slider_get_min_value(p_lview_data->vol_slider);
			btmusic_vol_sync(minvol);
		#endif
		}
		//lv_slider_set_value(p_lview_data->vol_slider, lv_slider_get_value(p_lview_data->vol_slider) - 1, LV_ANIM_OFF);
		lv_slider_set_value(p_lview_data->vol_slider, 0, LV_ANIM_OFF);
#if DISPLAY_MORE_VIEW
	} else if (p_lview_data->btn[BTN_MORE] == obj) {
		/* display more view */
		_display_more_view(view_data);
#endif
	} else if (p_lview_data->more_btn[BTN_WATCH] == obj
#if DEF_UI_WIDTH >= 454
		|| p_lview_data->more_btn[BTN_WATCH_TXT] == obj
		|| p_lview_data->more_btn[BTN_WATCH_CH] == obj
#endif
		) {
		/* delect more view */
		_exit_more_view();
		if (app->cur_player == BTMUSIC_PLAYER) {
			SYS_LOG_INF("switch to lcmusic player\n");
			app->cur_player = LCMUSIC_PLAYER;
			p_music_player = LCMUSIC_PLAYER;
		#ifdef CONFIG_LCMUSIC_APP
			_music_play_view(app->cur_player, BTMUSIC_PLAYER);
		#endif
		}
	} else if (p_lview_data->more_btn[BTN_PHONE] == obj
#if DEF_UI_WIDTH >= 454
		|| p_lview_data->more_btn[BTN_PHONE_TXT] == obj
		|| p_lview_data->more_btn[BTN_PHONE_CH] == obj
#endif
		) {
		/* delect more view */
		_exit_more_view();
		if (app->cur_player == LCMUSIC_PLAYER) {
			SYS_LOG_INF("switch to btmusic player\n");
			app->cur_player = BTMUSIC_PLAYER;
			p_music_player = BTMUSIC_PLAYER;
		#ifdef CONFIG_LCMUSIC_APP
			_music_play_view(app->cur_player, LCMUSIC_PLAYER);
		#endif
		}
	#ifdef CONFIG_BT_TRANSMIT
	} else if (!p_music_player &&
		(p_lview_data->more_btn[BTN_BT_EARPHONE] == obj
#if DEF_UI_WIDTH >= 454
		|| p_lview_data->more_btn[BTN_BT_EARPHONE_TXT] == obj
		|| p_lview_data->more_btn[BTN_BT_EARPHONE_CH] == obj
#endif
		)) {
		/* delect more view */
		_exit_more_view();
		bt_transmit_inquiry_start_notify(_bt_earphone_list_info_update);
		_display_bt_earphone_list(view_data);
	} else if (p_lview_data->bt_earphone_update_btn[BTN_BT_EARPHONE_UPDATE] == obj) {
		bt_transmit_inquiry_start_notify(_bt_earphone_list_info_update);
	#endif
	}
#endif
}

static void _cvt_txt_array(lv_point_t *pt, lv_style_t *sty, lv_font_t *font, lvgl_res_string_t* txt, uint8_t align, uint32_t num)
{
	int i;

	for (i = 0; i < num; i++) {
		pt[i].x = txt[i].x;
		pt[i].y = txt[i].y;

		lv_style_init(&sty[i]);
		lv_style_set_text_font(&sty[i], font);
		lv_style_set_text_color(&sty[i], txt[i].color);
		lv_style_set_text_align(&sty[i], align);
	}
}

static void _create_img_array(lv_obj_t *par, lv_obj_t **pobj, lv_point_t *pt,
										lv_img_dsc_t *img, uint32_t num)
{
	int i;

	for (i = 0; i < num; i++) {
		pobj[i] = lv_img_create(par);
		lv_img_set_src(pobj[i], &img[i]);
		lv_obj_set_pos(pobj[i], pt[i].x, pt[i].y);
	}
}

static void _create_btn_array(lv_obj_t *par, lv_obj_t **pobj, lv_point_t *pt,
										lv_img_dsc_t *def, lv_img_dsc_t *sel, uint32_t num, void* data)
{
	int i;

	for (i = 0; i < num; i++) {
		pobj[i] = lv_imgbtn_create(par);
		lv_obj_set_pos(pobj[i], pt[i].x, pt[i].y);
		lv_obj_set_size(pobj[i], def[i].header.w, def[i].header.h);
		lv_obj_set_user_data(pobj[i], (void*)i);
		lv_obj_add_event_cb(pobj[i], _btn_event_handler, LV_EVENT_CLICKED, data);

		lv_imgbtn_set_src(pobj[i], LV_IMGBTN_STATE_RELEASED, NULL, &def[i], NULL);
		lv_imgbtn_set_src(pobj[i], LV_IMGBTN_STATE_PRESSED, NULL, &sel[i], NULL);
		lv_imgbtn_set_src(pobj[i], LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &sel[i], NULL);
		lv_imgbtn_set_src(pobj[i], LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &def[i], NULL);
	}
}

#if DISPLAY_MORE_VIEW
static void _create_bt_connect_state_img(lv_obj_t *scr)
{
	if (p_lview_data->bt_state) {
		lv_obj_del(p_lview_data->bt_state);
		p_lview_data->bt_state = NULL;
	}

	p_lview_data->bt_state = lv_img_create(scr);
#ifdef CONFIG_BT_TRANSMIT
	if (bt_manager_trs_get_connected_dev_num()) {
		lv_img_set_src(p_lview_data->bt_state, &p_lview_data->img_dsc_bt_state_bmp[BMP_BT_CONNECTED]);
		lv_obj_set_pos(p_lview_data->bt_state, p_lview_data->pt_bt_state_bmp[BMP_BT_CONNECTED].x, p_lview_data->pt_bt_state_bmp[BMP_BT_CONNECTED].y);
	}
	else
#endif
	{
		lv_img_set_src(p_lview_data->bt_state, &p_lview_data->img_dsc_bt_state_bmp[BMP_BT_DISCONNECT]);
		lv_obj_set_pos(p_lview_data->bt_state, p_lview_data->pt_bt_state_bmp[BMP_BT_DISCONNECT].x, p_lview_data->pt_bt_state_bmp[BMP_BT_DISCONNECT].y);
	}
}
#endif

static void _create_label_array(lv_obj_t *par, lv_obj_t **pobj, lv_point_t *pt,
								lv_style_t *sty, lvgl_res_string_t *res_txt, uint32_t num)
{
	int i;

	for (i = 0; i < num; i++) {
		pobj[i] = lv_label_create(par);
		lv_label_set_text(pobj[i], "");
		lv_label_set_long_mode(pobj[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
		lv_obj_set_style_anim_speed(pobj[i], 20, LV_PART_MAIN);

		lv_obj_add_style(pobj[i], &sty[i], LV_PART_MAIN);
		lv_obj_set_pos(pobj[i], pt[i].x, pt[i].y);
		//lv_obj_set_size(pobj[i], res_txt[i].width, res_txt[i].height);
		lv_obj_set_width(pobj[i], res_txt[i].width);

		SYS_LOG_INF("x:%d y=%d width =%d height=%d\n",
			pt[i].x, pt[i].y, res_txt[i].width, res_txt[i].height);
	}
}

static void _create_vol_bar(lv_obj_t *scr)
{
	int16_t music_vol = 18;

	lv_style_init(&p_lview_data->style_slider_bg);
	lv_style_set_bg_color(&p_lview_data->style_slider_bg, lv_color_make(0xC0, 0xC0, 0xC0));
	lv_style_set_bg_opa(&p_lview_data->style_slider_bg, LV_OPA_COVER);
	lv_style_set_radius(&p_lview_data->style_slider_bg, LV_RADIUS_CIRCLE);

	lv_style_init(&p_lview_data->style_slider_indic);
	lv_style_set_bg_color(&p_lview_data->style_slider_indic, lv_color_make(0, 0, 255));
	lv_style_set_bg_opa(&p_lview_data->style_slider_indic, LV_OPA_COVER);

	lv_style_init(&p_lview_data->style_slider_knob);
	lv_style_set_bg_opa(&p_lview_data->style_slider_knob, LV_OPA_COVER);
	lv_style_set_bg_color(&p_lview_data->style_slider_knob, lv_color_make(0, 0, 255));
	lv_style_set_radius(&p_lview_data->style_slider_knob, LV_RADIUS_CIRCLE);
	lv_style_set_pad_left(&p_lview_data->style_slider_knob, 6);
	lv_style_set_pad_right(&p_lview_data->style_slider_knob, 6);
	lv_style_set_pad_top(&p_lview_data->style_slider_knob, 6);
	lv_style_set_pad_bottom(&p_lview_data->style_slider_knob, 6);

	p_lview_data->vol_slider = lv_slider_create(scr);
	lv_obj_set_size(p_lview_data->vol_slider, 6, 227);
	lv_obj_set_ext_click_area(p_lview_data->vol_slider, LV_DPX(32));
	lv_obj_add_style(p_lview_data->vol_slider, &p_lview_data->style_slider_bg, LV_PART_MAIN);
	lv_obj_add_style(p_lview_data->vol_slider, &p_lview_data->style_slider_indic, LV_PART_INDICATOR);
	lv_obj_add_style(p_lview_data->vol_slider, &p_lview_data->style_slider_knob, LV_PART_KNOB);


	lv_obj_center(p_lview_data->vol_slider);
	lv_obj_add_event_cb(p_lview_data->vol_slider, _slider_event_handle, LV_EVENT_VALUE_CHANGED, NULL);

#ifdef CONFIG_AUDIO
	if (launcher_get_cur_player() == BTMUSIC_PLAYER) {
		music_vol = system_volume_get(AUDIO_STREAM_MUSIC);
	} else {
		music_vol = system_volume_get(AUDIO_STREAM_LOCAL_MUSIC);
	}

	lv_slider_set_range(p_lview_data->vol_slider, 0, audio_policy_get_volume_level());
#else
	lv_slider_set_range(p_lview_data->vol_slider, 0, 15);
#endif

	lv_slider_set_value(p_lview_data->vol_slider, music_vol, LV_ANIM_OFF);
	//lv_obj_set_adv_hittest(p_lview_data->vol_slider, true);
}

static void get_center(lv_obj_t * obj, lv_point_t * center, lv_coord_t * arc_r)
{
    lv_coord_t left_bg = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    lv_coord_t right_bg = lv_obj_get_style_pad_right(obj, LV_PART_MAIN);
    lv_coord_t top_bg = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t bottom_bg = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);

    lv_coord_t r = (LV_MIN(lv_obj_get_width(obj) - left_bg - right_bg,
                           lv_obj_get_height(obj) - top_bg - bottom_bg)) / 2;

    *arc_r = r;
    center->x = obj->coords.x1 + r + left_bg;
    center->y = obj->coords.y1 + r + top_bg;

    lv_coord_t indic_width = lv_obj_get_style_arc_width(obj, LV_PART_INDICATOR);
    r -= indic_width;
}

static void _progress_bar_event_cb(lv_event_t *e)
{
	lv_event_code_t event = lv_event_get_code(e);
//	lv_obj_t *obj = lv_event_get_current_target(e);
	uint32_t total_time = 0;
	int cur_time = 0;
	int seek_time = 0;
    lv_obj_t * obj = lv_event_get_target(e);
    lv_arc_t * arc = (lv_arc_t *)lv_event_get_target(e);

	if(event == LV_EVENT_SHORT_CLICKED)
	{
		if(p_music_player == 1)
		{
			//dont seek when in bt music
			return;
		}
	
		//get progress value
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;

        //Handle only pointers here
        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        if(indev_type != LV_INDEV_TYPE_POINTER) return;

        lv_point_t p;
        lv_indev_get_point(indev, &p);
		SYS_LOG_INF("click %d %d\n", p.x, p.y);

        lv_point_t center;
        lv_coord_t r;
        get_center(obj, &center, &r);

        p.x -=  center.x;
        p.y -=  center.y;

        /*Calculate the angle of the pressed point*/
        int16_t angle;
        int16_t bg_end = arc->bg_angle_end;
        if(arc->bg_angle_end < arc->bg_angle_start) {
            bg_end = arc->bg_angle_end + 360;
        }

        angle = lv_atan2(p.y, p.x);
        angle -= arc->rotation;
        angle -= arc->bg_angle_start;  /*Make the angle relative to the start angle*/

        if(angle < 0) angle += 360;

		SYS_LOG_INF("angle %d\n", angle);

#ifdef CONFIG_LCMUSIC_APP
		lcmusic_get_song_bp(&total_time, &cur_time);

		seek_time = (angle*total_time)/360;
        lcmusic_seek(seek_time, cur_time, true);
        SYS_LOG_INF("total time %d, seek time %d, cur time %d", total_time, seek_time - cur_time, cur_time);
#endif
	}
}


static void _create_music_play_progress_bar(lv_obj_t *scr)
{
	/* background style */
	lv_style_init(&p_lview_data->style_arc_bg);
	lv_style_set_arc_color(&p_lview_data->style_arc_bg, lv_color_make(50, 50, 50));
	lv_style_set_arc_width(&p_lview_data->style_arc_bg, 4);
	lv_style_set_arc_rounded(&p_lview_data->style_arc_bg, true);

	/* line style_red */
	lv_style_init(&p_lview_data->style_arc_red);
	lv_style_set_arc_color(&p_lview_data->style_arc_red, lv_color_make(255, 0, 0));
	lv_style_set_arc_width(&p_lview_data->style_arc_red, 6);
	lv_style_set_arc_rounded(&p_lview_data->style_arc_red, true);

	/* draw background */
	p_lview_data->progress_arc= progress_arc_create(scr);
	lv_obj_set_size(p_lview_data->progress_arc, 200, 200);
	lv_obj_add_style(p_lview_data->progress_arc, &p_lview_data->style_arc_bg, LV_PART_MAIN);
	lv_obj_add_style(p_lview_data->progress_arc, &p_lview_data->style_arc_red, LV_PART_INDICATOR);
//	lv_obj_clear_flag(p_lview_data->progress_arc, LV_OBJ_FLAG_CLICKABLE);
	lv_arc_set_bg_angles(p_lview_data->progress_arc, 0, 360);
	lv_arc_set_rotation(p_lview_data->progress_arc,270);
	lv_arc_set_angles(p_lview_data->progress_arc, 0, 0);
	lv_obj_center(p_lview_data->progress_arc);
	lv_obj_set_ext_click_area(p_lview_data->progress_arc, 15);
	lv_obj_add_event_cb(p_lview_data->progress_arc, _progress_bar_event_cb, LV_EVENT_ALL, NULL);
}

static void _display_vol_view(view_data_t *view_data)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);

	
	/* load resource */
	lvgl_res_load_pictures_from_scene(&p_lview_data->res_scene, vol_btn_def_ids, p_lview_data->img_dsc_vol_btn, p_lview_data->pt_vol_btn, NUM_VOL_BTNS);

	p_lview_data->vol_bmp = lv_img_create(scr);
	lv_obj_set_pos(p_lview_data->vol_bmp, 0, 0);
	lv_obj_set_size(p_lview_data->vol_bmp, DEF_UI_WIDTH, DEF_UI_HEIGHT);

	lv_obj_set_style_bg_color(p_lview_data->vol_bmp, lv_color_make(0x3b, 0x3b, 0x3b), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(p_lview_data->vol_bmp, LV_OPA_COVER, LV_PART_MAIN);

	lv_obj_add_event_cb(p_lview_data->vol_bmp, _bmp_event_handler, LV_EVENT_ALL, view_data);
	/*set current background clickable to cut out click event send to front bg*/
	lv_obj_add_flag(p_lview_data->vol_bmp, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_clear_flag(p_lview_data->vol_bmp, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

	/* create button */
	_create_btn_array(p_lview_data->vol_bmp, p_lview_data->vol_btn, p_lview_data->pt_vol_btn, p_lview_data->img_dsc_vol_btn, p_lview_data->img_dsc_vol_btn, NUM_VOL_BTNS, view_data);

	/* create bar to display volume bar */
	_create_vol_bar(p_lview_data->vol_bmp);

	/* hidden left and right view to receive callback function when BMP_VOL_BG was draw */
	ui_manager_gesture_set_dir(0);
}

#ifdef CONFIG_BT_TRANSMIT
static void _display_bt_earphone_list(view_data_t *view_data)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);
	/* load resource */
	lvgl_res_load_pictures_from_scene(&p_lview_data->res_scene, bt_earphone_update_btn_ids, p_lview_data->img_dsc_bt_earphone_update_btn, p_lview_data->pt_bt_earphone_update_btn, NUM_BT_EARPHONE_LIST_BTNS);
	lvgl_res_load_strings_from_scene(&p_lview_data->res_scene, bt_earphone_list_txt_ids, p_lview_data->res_txt_bt_earphone_list, NUM_BT_EARPHONE_LIST_TXTS);
	/* convert resource */
	_cvt_txt_array(p_lview_data->pt_bt_earphone_list_txt, p_lview_data->style_txt_bt_earphone_list, &p_lview_data->font, p_lview_data->res_txt_bt_earphone_list, LV_TEXT_ALIGN_CENTER, NUM_BT_EARPHONE_LIST_TXTS);

	/* create bg image */
	p_lview_data->bt_earphone_list_bg_img = lv_img_create(scr);
	lv_obj_set_pos(p_lview_data->bt_earphone_list_bg_img, 0, 0);
	lv_obj_set_size(p_lview_data->bt_earphone_list_bg_img, DEF_UI_WIDTH, DEF_UI_HEIGHT);

	lv_obj_set_style_bg_color(p_lview_data->bt_earphone_list_bg_img, lv_color_make(0x3b, 0x3b, 0x3b), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(p_lview_data->bt_earphone_list_bg_img, LV_OPA_COVER, LV_PART_MAIN);

	lv_obj_add_event_cb(p_lview_data->bt_earphone_list_bg_img, _bmp_event_handler, LV_EVENT_ALL, view_data);
	/*set current background clickable to cut out click event send to front bg*/
	lv_obj_add_flag(p_lview_data->bt_earphone_list_bg_img, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_clear_flag(p_lview_data->bt_earphone_list_bg_img, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

	/* create button */
	_create_btn_array(p_lview_data->bt_earphone_list_bg_img, p_lview_data->bt_earphone_update_btn, p_lview_data->pt_bt_earphone_update_btn, p_lview_data->img_dsc_bt_earphone_update_btn, p_lview_data->img_dsc_bt_earphone_update_btn, NUM_BT_EARPHONE_LIST_BTNS, view_data);
	/*create text*/
	_create_label_array(p_lview_data->bt_earphone_list_bg_img, p_lview_data->bt_earphone_name, p_lview_data->pt_bt_earphone_list_txt, p_lview_data->style_txt_bt_earphone_list, p_lview_data->res_txt_bt_earphone_list, NUM_BT_EARPHONE_LIST_TXTS);

	lv_obj_add_event_cb(p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1], _bt_earphone_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2], _bt_earphone_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1], LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2], LV_OBJ_FLAG_CLICKABLE);
	lv_label_set_text(p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_LIST_NAME], "蓝牙耳机");

	p_lview_data->update_bt_earphone_list = 1;

	/* hidden left and right view to receive callback function when bt earphone list was draw */
	ui_manager_gesture_set_dir(0);
}
#endif /* CONFIG_BT_TRANSMIT */

#if DISPLAY_MORE_VIEW
static void _display_more_view(view_data_t *view_data)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);
	/* load resource */
	lvgl_res_load_pictures_from_scene(&p_lview_data->res_scene, more_btn_def_ids, p_lview_data->img_dsc_more_btn, p_lview_data->pt_more_btn, NUM_MORE_BTNS);
	lvgl_res_load_pictures_from_scene(&p_lview_data->res_scene, more_btn_sel_ids, p_lview_data->img_dsc_more_btn_sel, p_lview_data->pt_more_btn, NUM_MORE_BTNS);
	lvgl_res_load_pictures_from_scene(&p_lview_data->res_scene, bt_state_bmp_ids, p_lview_data->img_dsc_bt_state_bmp, p_lview_data->pt_bt_state_bmp, NUM_BT_STATE_IMGS);

	/* create image */
	p_lview_data->more_bmp = lv_img_create(scr);
	lv_obj_set_pos(p_lview_data->more_bmp, 0, 0);
	lv_obj_set_size(p_lview_data->more_bmp, DEF_UI_WIDTH, DEF_UI_HEIGHT);

	lv_obj_set_style_bg_color(p_lview_data->more_bmp, lv_color_make(0x3b, 0x3b, 0x3b), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(p_lview_data->more_bmp, LV_OPA_COVER, LV_PART_MAIN);

	/*set current background clickable to cut out click event send to front bg*/
	lv_obj_add_flag(p_lview_data->more_bmp, LV_OBJ_FLAG_CLICKABLE);

	/* create button */
	_create_btn_array(p_lview_data->more_bmp, p_lview_data->more_btn, p_lview_data->pt_more_btn, p_lview_data->img_dsc_more_btn, p_lview_data->img_dsc_more_btn_sel, NUM_MORE_BTNS, view_data);
#if DEF_UI_WIDTH >= 454
	lv_obj_set_ext_click_area(p_lview_data->more_btn[BTN_WATCH_CH], lv_obj_get_style_width(p_lview_data->more_btn[BTN_WATCH_CH], LV_PART_MAIN));
	lv_obj_set_ext_click_area(p_lview_data->more_btn[BTN_PHONE_CH], lv_obj_get_style_width(p_lview_data->more_btn[BTN_PHONE_CH], LV_PART_MAIN));
	lv_obj_set_ext_click_area(p_lview_data->more_btn[BTN_BT_EARPHONE_CH], lv_obj_get_style_width(p_lview_data->more_btn[BTN_BT_EARPHONE_CH], LV_PART_MAIN));

	lv_obj_set_ext_click_area(p_lview_data->more_btn[BTN_WATCH_TXT], lv_obj_get_style_height(p_lview_data->more_btn[BTN_WATCH_TXT], LV_PART_MAIN));
	lv_obj_set_ext_click_area(p_lview_data->more_btn[BTN_PHONE_TXT], lv_obj_get_style_height(p_lview_data->more_btn[BTN_PHONE_TXT], LV_PART_MAIN));
	lv_obj_set_ext_click_area(p_lview_data->more_btn[BTN_BT_EARPHONE_TXT], lv_obj_get_style_height(p_lview_data->more_btn[BTN_BT_EARPHONE_TXT], LV_PART_MAIN));

	lv_obj_add_flag(p_lview_data->more_btn[BTN_WATCH_CH], LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(p_lview_data->more_btn[BTN_PHONE_CH], LV_OBJ_FLAG_CHECKABLE);
	if (p_music_player) {
		btn_state_toggle(p_lview_data->more_btn[BTN_PHONE_CH]);
		lv_obj_invalidate(p_lview_data->more_btn[BTN_PHONE_CH]);
	} else {
		btn_state_toggle(p_lview_data->more_btn[BTN_WATCH_CH]);
		lv_obj_invalidate(p_lview_data->more_btn[BTN_WATCH_CH]);
	}
#endif
	/* create bt connect state image */
	_create_bt_connect_state_img(p_lview_data->more_bmp);

	/* hidden left and right view to receive callback function when more view was draw */
	ui_manager_gesture_set_dir(0);
}
#endif

static int _load_resource(lcmusic_view_data_t *data, bool first_layout)
{
	int32_t ret;

	if(first_layout)
	{
		/* load scene */
		ret = lvgl_res_load_scene(SCENE_LCMUSIC_VIEW, &data->res_scene, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
		if (ret < 0) {
			SYS_LOG_ERR("SCENE_LCMUSIC_VIEW not found");
			return -ENOENT;
		}

		/* open font */
#if DEF_UI_WIDTH < 454
		if (lvgl_bitmap_font_open(&data->font, DEF_FONT22_FILE) < 0) {
			SYS_LOG_ERR("font not found");
			return -ENOENT;
		}
#else
		if (lvgl_bitmap_font_open(&data->font, DEF_FONT32_FILE) < 0) {
			SYS_LOG_ERR("font not found");
			return -ENOENT;
		}
#endif
	}

	/* load resource */
	lvgl_res_load_pictures_from_scene(&data->res_scene, _bmp_ids, data->img_dsc_bmp, data->pt_bmp, NUM_IMGS);
	lvgl_res_load_pictures_from_scene(&data->res_scene, _btn_def_ids, data->img_dsc_def, data->pt_def, NUM_BTNS);
	lvgl_res_load_pictures_from_scene(&data->res_scene, _btn_sel_ids, data->img_dsc_sel, data->pt_sel, NUM_BTNS);

	if(first_layout)
	{
		lvgl_res_load_strings_from_scene(&data->res_scene, _txt_ids, data->res_txt, NUM_TXTS);

		/* convert resource */
		_cvt_txt_array(data->pt_txt, data->style_txt, &data->font, data->res_txt, LV_TEXT_ALIGN_CENTER, NUM_TXTS);
	}


	SYS_LOG_INF("load resource succeed");

	return 0;
}

static void _unload_pic_resource(lcmusic_view_data_t *data)
{
	lvgl_res_unload_pictures(data->img_dsc_def, NUM_BTNS);
	lvgl_res_unload_pictures(data->img_dsc_sel, NUM_BTNS);
	lvgl_res_unload_pictures(data->img_dsc_bmp, NUM_IMGS);
}

static void _unload_resource(lcmusic_view_data_t *data)
{
	lvgl_res_unload_pictures(data->img_dsc_def, NUM_BTNS);
	lvgl_res_unload_pictures(data->img_dsc_sel, NUM_BTNS);
	lvgl_res_unload_pictures(data->img_dsc_bmp, NUM_IMGS);
	lvgl_res_unload_strings(data->res_txt, NUM_TXTS);

	lvgl_bitmap_font_close(&data->font);

	lvgl_res_unload_scene(&data->res_scene);

}

static void _set_view_play_state(bool playing)
{
	uint8_t btn_state = lv_obj_get_state(p_lview_data->btn[BTN_PLAY]);

	if (!(btn_state & LV_STATE_PRESSED) &&
		((playing && ((btn_state & LV_STATE_CHECKED) == 0))
		|| (!playing && ((btn_state & LV_STATE_CHECKED) == LV_STATE_CHECKED)))) {
		btn_state_toggle(p_lview_data->btn[BTN_PLAY]);
		lv_obj_invalidate(p_lview_data->btn[BTN_PLAY]);
	}
}

static void _display_song_name_album(void)
{
	if (!p_lview_data || !p_lview_data->song_name || !p_lview_data->album)
		return;

	lv_label_set_text(p_lview_data->lbl[TXT_SONG_NAME], p_lview_data->song_name);
	lv_label_set_text(p_lview_data->lbl[TXT_ALBUM], p_lview_data->album);

	SYS_LOG_INF("song_name:%s album:%s\n", p_lview_data->song_name, p_lview_data->album);

	app_mem_free(p_lview_data->song_name);
	app_mem_free(p_lview_data->album);
	p_lview_data->song_name = NULL;
	p_lview_data->album = NULL;
}

#ifdef CONFIG_BT_PLAYER
static void _sync_btmusic_play_pos(uint32_t cur_time)
{
	if (!p_lview_data)
		return;

	SYS_LOG_INF("song_pos:%u\n", cur_time);
	p_lview_data->song_cur_time = cur_time;
}

static void _sync_btmusic_volume(int volume)
{
	if (!p_lview_data)
		return;

	p_lview_data->volume_need_sync = 1;
}

static void _sync_btmusic_play_info(const char *song_name, const char *album, uint32_t total_time)
{
	os_mutex_lock(&music_view_mutex, OS_FOREVER);

	if (!p_lview_data)
	{
		os_mutex_unlock(&music_view_mutex);	
		return;
	}

	if (p_lview_data->song_name) {
		app_mem_free(p_lview_data->song_name);
		p_lview_data->song_name = NULL;
	}
	if (p_lview_data->album) {
		app_mem_free(p_lview_data->album);
		p_lview_data->album = NULL;
	}

	p_lview_data->song_name = app_mem_malloc(strlen(song_name) + 1);
	if (!p_lview_data->song_name) {
		SYS_LOG_ERR("malloc fail\n");
		os_mutex_unlock(&music_view_mutex);
		return;
	}
	memset(p_lview_data->song_name, 0, strlen(song_name) + 1);
	memcpy(p_lview_data->song_name, song_name, strlen(song_name));

	p_lview_data->album = app_mem_malloc(strlen(album) + 1);
	if (!p_lview_data->album) {
		SYS_LOG_ERR("malloc fail\n");
		if (p_lview_data->song_name) {
			app_mem_free(p_lview_data->song_name);
			p_lview_data->song_name = NULL;
		}
		os_mutex_unlock(&music_view_mutex);
		return;
	}
	memset(p_lview_data->album, 0, strlen(album) + 1);
	memcpy(p_lview_data->album, album, strlen(album));

	p_lview_data->song_time = total_time;

	p_lview_data->update_song_info = 1;

	os_mutex_unlock(&music_view_mutex);
	SYS_LOG_INF("song_time:%d\n", p_lview_data->song_time);
}
#endif /* CONFIG_BT_PLAYER */

#ifdef CONFIG_BT_TRANSMIT
static void _bt_earphone_list_info_update(struct bt_device_info_t *bt_info, uint8_t count)
{
	struct bt_device_info_t *info = p_lview_data->bt_earphone_info;
	int i = 0, j = 0;
	int connected_idx = -1;

	if (!p_lview_data)
		return;

	os_mutex_lock(&music_view_mutex, OS_FOREVER);

	p_lview_data->update_bt_earphone_list = 1;

	/* clear disconnected device info */
	for (i = 0; i < MAX_SUPPORT_CNT; i++) {
		if (info[i].name == NULL)
			continue;

		if (!info[i].connected) {
			app_mem_free(info[i].name);
			memset(&info[i], 0, sizeof(info[i]));
		} else if (connected_idx < 0) {
			connected_idx = i;
		}
	}

	/* move connected device to the first line*/
	if (connected_idx >= 0 && connected_idx < MAX_SUPPORT_CNT) {
		memcpy(&info[0], &info[connected_idx], sizeof(info[0]));
		memset(&info[connected_idx], 0, sizeof(info[0]));
	}

	if (count == 0) {
		os_mutex_unlock(&music_view_mutex);
		return;
	}

	for (i = 0, j = 0; i < MAX_SUPPORT_CNT; i++) {
		if (info[i].name)
			continue;

		/* avoid redisplay connected device*/
		if (info[0].connected) {
			if (info[0].addr.val[0] == bt_info[j].addr.val[0] &&
				info[0].addr.val[1] == bt_info[j].addr.val[1] &&
				info[0].addr.val[2] == bt_info[j].addr.val[2] &&
				info[0].addr.val[3] == bt_info[j].addr.val[3] &&
				info[0].addr.val[4] == bt_info[j].addr.val[4] &&
				info[0].addr.val[5] == bt_info[j].addr.val[5]) {

				if (++j >= count) break;
				else continue;
			}
		}

		info[i].name = app_mem_malloc(strlen(bt_info[j].name) + 1);
		if (!info[i].name) {
			SYS_LOG_ERR("malloc failed\n");
			os_mutex_unlock(&music_view_mutex);
			return;
		}

		strcpy(info[i].name, bt_info[j].name);
		memcpy(&info[i].addr, &bt_info[j].addr, sizeof(bd_address_t));
		info[i].connected = bt_info[j].connected;
		info[i].paired = bt_info[j].paired;

		if (++j >= count) break;
	}

	os_mutex_unlock(&music_view_mutex);
}

static void _display_bt_earphone_list_info(void)
{
	if (!p_lview_data || !p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1] ||
		!p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2] ||
		!p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1_CNT_STATE] ||
		!p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2_CNT_STATE])
		return;

	struct bt_device_info_t *info = p_lview_data->bt_earphone_info;

	lv_obj_t *name_objs[] = {
		p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1],
		p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2],
	};

	lv_obj_t *state_objs[] = {
		p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME1_CNT_STATE],
		p_lview_data->bt_earphone_name[TXT_BT_EARPHONE_NAME2_CNT_STATE],
	};

	for (int i = 0; i < 2; i++) {
		if (info[i].name == NULL) {
			lv_label_set_text(name_objs[i], "");
			lv_label_set_text(state_objs[i], "");
			continue;
		}

		lv_label_set_text(name_objs[i], info[i].name);

		if (info[i].connected) {
			lv_label_set_text(state_objs[i], "已连接");
		} else if (info[i].paired) {
			lv_label_set_text(state_objs[i], "已配对");
		} else {
			lv_label_set_text(state_objs[i], "");
		}

		SYS_LOG_INF("[%d] name %s, connected %d, paired %d\n",
			i, info[i].name, info[i].connected, info[i].paired);
	}
}

static void _bt_earphone_info_update_check(void)
{
	struct bt_device_info_t *info = p_lview_data->bt_earphone_info;
	bool need_update = false;

	if (!p_lview_data)
		return;

	for (int i = 0; i < MAX_SUPPORT_CNT; i++) {
		bool connected, paired;

		if (info[i].name == NULL)
			continue;

		connected = bt_transmit_check_dev_connected(info[i].addr.val) ? 1 : 0;
		paired = bt_manager_get_addr_linkkey(&info[i].addr, NULL) ? 0 : 1;

		if (connected != info[i].connected || paired != info[i].paired) {
			SYS_LOG_INF("name %s, connected %d, paired %d\n", info[i].name,
				info[i].connected, info[i].paired);
			info[i].connected = connected;
			info[i].paired = paired;
			need_update = true;
		}
	}

	if (need_update)
		_display_bt_earphone_list_info();
}
#endif /* CONFIG_BT_TRANSMIT */

#ifdef CONFIG_LCMUSIC_APP
void lcmusic_display_song_info(char *song_info, bool direct_update)
{
	char *song_name = NULL;
	uint16_t strlen = 0;

	lcmusic_get_song_name(&song_name, &strlen);
	if (strlen == 0)
	{
		os_mutex_lock(&music_view_mutex, OS_FOREVER);
		if(p_lview_data)
		{
			p_lview_data->late_update_song_info = 1;
		}
		os_mutex_unlock(&music_view_mutex);
		return;
	}

	os_mutex_lock(&music_view_mutex, OS_FOREVER);
	if (!p_lview_data)
	{
		os_mutex_unlock(&music_view_mutex);
		return;	
	}

	if (p_lview_data->song_name) {
		app_mem_free(p_lview_data->song_name);
		p_lview_data->song_name = NULL;
	}
	if (p_lview_data->album) {
		app_mem_free(p_lview_data->album);
		p_lview_data->album = NULL;
	}

	p_lview_data->song_name = app_mem_malloc(strlen + 1);
	if (!p_lview_data->song_name) {
		LOG_ERR("malloc fail\n");
		os_mutex_unlock(&music_view_mutex);
		return;
	}
	memset(p_lview_data->song_name, 0, strlen + 1);
	memcpy(p_lview_data->song_name, song_name, strlen);

	p_lview_data->album = app_mem_malloc(strlen + 1);
	if (!p_lview_data->album) {
		LOG_ERR("malloc fail\n");
		if (p_lview_data->song_name) {
			app_mem_free(p_lview_data->song_name);
			p_lview_data->song_name = NULL;
		}
		os_mutex_unlock(&music_view_mutex);
		return;
	}
	memset(p_lview_data->album, 0, strlen + 1);
	memcpy(p_lview_data->album, song_name, strlen);

	/*direct update song info must in ui_service thread*/
	if (direct_update) {
		_display_song_name_album();
	} else {
		p_lview_data->update_song_info = 1;
	}
	p_lview_data->is_switching = false;
	os_mutex_unlock(&music_view_mutex);
}
#endif

static void _display_play_progress()
{
	uint16_t end_angle = 0;
	uint16_t old_angle = 0;
	uint32_t total_time = 0;
	int cur_time = 0;
	uint8_t player = launcher_get_cur_player();

	if (!p_lview_data || !p_lview_data->progress_arc)
		return;

	if (player == BTMUSIC_PLAYER) {
		total_time = p_lview_data->song_time;
		cur_time = p_lview_data->song_cur_time;
	} else if (player == LCMUSIC_PLAYER) {
	#ifdef CONFIG_LCMUSIC_APP
		lcmusic_get_song_bp(&total_time, &cur_time);
	#endif
	}

	if (total_time) {
		end_angle = 360 * cur_time / total_time;
		old_angle = lv_arc_get_angle_end(p_lview_data->progress_arc);
		if(old_angle != end_angle)
		{
			lv_obj_invalidate(p_lview_data->progress_arc);
			lv_arc_set_end_angle(p_lview_data->progress_arc, end_angle);
		}
	} else {
		lv_obj_invalidate(p_lview_data->progress_arc);
		lv_arc_set_end_angle(p_lview_data->progress_arc, 0);
	}

	SYS_LOG_DBG("total_time=%d cur_time=%d end_angle=%d\n", total_time, cur_time, end_angle);
}

static int _music_view_preload(view_data_t *view_data, bool update)
{
	if (music_preload_inited == 0) {
		lvgl_res_preload_scene_compact_default_init(SCENE_LCMUSIC_VIEW, music_resource_id, ARRAY_SIZE(music_resource_id),
			NULL, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
		music_preload_inited = 1;
	}

	return lvgl_res_preload_scene_compact_default(SCENE_LCMUSIC_VIEW, MUSIC_VIEW, update, 0);
}

static void _music_view_update_process_task_cb(lv_timer_t *timer);

static int _music_view_layout_update(view_data_t *view_data, bool first_layout)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);
	lcmusic_view_data_t *data = view_data->user_data;

	if(first_layout)
	{
		SYS_LOG_INF("_lcmusic_view_layout first\n");
		data = app_mem_malloc(sizeof(*data));
		if (!data) {
			return -ENOMEM;
		}

		view_data->user_data = data;
		memset(data, 0, sizeof(*data));
	}

	if (_load_resource(data, first_layout)) {
		app_mem_free(data);
		view_data->user_data = NULL;
		return -ENOENT;
	}

	if(first_layout)
	{
		p_lview_data = data;
		/* create image */
		_create_img_array(scr, data->bmp, data->pt_bmp, data->img_dsc_bmp, NUM_IMGS);

		/* create play progress */
		_create_music_play_progress_bar(scr);
		/* create button */
		_create_btn_array(scr, data->btn, data->pt_def, data->img_dsc_def, data->img_dsc_sel, NUM_BTNS, view_data);

		/* FIXME: donot update the play/pause icon when pressed */
		lv_imgbtn_set_src(data->btn[BTN_PLAY], LV_IMGBTN_STATE_RELEASED, NULL, &data->img_dsc_def[BTN_PLAY], NULL);
		lv_imgbtn_set_src(data->btn[BTN_PLAY], LV_IMGBTN_STATE_PRESSED, NULL, &data->img_dsc_def[BTN_PLAY], NULL);
		lv_imgbtn_set_src(data->btn[BTN_PLAY], LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &data->img_dsc_sel[BTN_PLAY], NULL);
		lv_imgbtn_set_src(data->btn[BTN_PLAY], LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &data->img_dsc_sel[BTN_PLAY], NULL);

		lv_obj_add_flag(data->btn[BTN_PLAY], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_add_flag(data->btn[BTN_SEQ_PLAY_M], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_add_flag(data->btn[BTN_BMP_MUSIC_SOURCE], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_clear_flag(data->btn[BTN_BMP_MUSIC_SOURCE], LV_OBJ_FLAG_CLICKABLE);

		/*set button ext click area to improve the recognition rate*/
		lv_obj_set_ext_click_area(data->btn[BTN_PRE], lv_obj_get_style_width(data->btn[BTN_PRE], LV_PART_MAIN));
		lv_obj_set_ext_click_area(data->btn[BTN_NEXT], lv_obj_get_style_width(data->btn[BTN_NEXT], LV_PART_MAIN));
		lv_obj_set_ext_click_area(data->btn[BTN_SEQ_PLAY_M], lv_obj_get_style_width(data->btn[BTN_SEQ_PLAY_M], LV_PART_MAIN));
	#if DISPLAY_MORE_VIEW
		lv_obj_set_ext_click_area(data->btn[BTN_MORE], lv_obj_get_style_width(data->btn[BTN_MORE], LV_PART_MAIN));
	#endif
		lv_obj_set_ext_click_area(data->btn[BTN_VOL], lv_obj_get_style_width(data->btn[BTN_VOL], LV_PART_MAIN) / 2);

		/* create label */
		_create_label_array(scr, data->lbl, data->pt_txt, data->style_txt, data->res_txt, NUM_TXTS);

		/* init text */
#ifdef CONFIG_LCMUSIC_APP
		if (!p_music_player)
		{
			lcmusic_display_song_info(NULL, true);
		}

		if (lcmusic_get_play_mode()) {
			btn_state_toggle(data->btn[BTN_SEQ_PLAY_M]);
			lv_obj_invalidate(data->btn[BTN_SEQ_PLAY_M]);
		}
#endif

#ifdef CONFIG_BT_PLAYER
		btmusic_set_play_callback(&btmusic_callback);
#endif

		if (p_music_player) {
		#ifdef CONFIG_LCMUSIC_APP
			_music_play_view(p_music_player, NUM_PLAYERS);
		#endif
		}
	}

	return 0;
}

static int _music_view_layout(view_data_t *view_data)
{
	int ret;
	lcmusic_view_data_t *data;

	ret = _music_view_layout_update(view_data, true);
	if(ret < 0)
	{
		return ret;
	}

	data = view_data->user_data;
	data->timer = lv_timer_create(_music_view_update_process_task_cb, MUSIC_INFO_UPDATE_PERIOD, data);
	if (data->timer)
		lv_timer_ready(data->timer);

	lv_refr_now(view_data->display);
	SYS_LOG_INF("_lcmusic_view_layout\n");

	return 0;
}

static void _music_view_update_process_task_cb(lv_timer_t *timer)
{
	uint8_t player = launcher_get_cur_player();

	if (!p_lview_data)
		return;

	_display_play_progress();

	if (p_lview_data->filter_update_pstate == 0) {
		if (player == LCMUSIC_PLAYER) {
		#ifdef CONFIG_BT_PLAYER
			_set_view_play_state(lcmusic_is_playing());
		#endif
		} else if (player == BTMUSIC_PLAYER) {
		#ifdef CONFIG_BT_PLAYER
			_set_view_play_state(btmusic_get_play_state());
			p_lview_data->get_playback_pos++;
			if (btmusic_get_play_state() && (p_lview_data->get_playback_pos * MUSIC_INFO_UPDATE_PERIOD >= 1000)) {
		#ifdef CONFIG_BT_AVRCP
				bt_manager_avrcp_get_playback_pos();
		#endif
				p_lview_data->get_playback_pos = 0;
			}
		#endif
		}
	} else {
		p_lview_data->filter_update_pstate--;
	}

#ifdef CONFIG_BT_PLAYER
	if (p_lview_data->volume_need_sync) {
		p_lview_data->volume_need_sync = 0;

		if (player == BTMUSIC_PLAYER && p_lview_data->vol_slider) {
			lv_slider_set_value(p_lview_data->vol_slider,
					system_volume_get(AUDIO_STREAM_MUSIC), LV_ANIM_OFF);
		}
	}
#endif

	os_mutex_lock(&music_view_mutex, OS_FOREVER);
	if (p_lview_data->update_song_info) {
		p_lview_data->update_song_info = 0;
		SYS_LOG_INF("display song name\n");
		_display_song_name_album();
	}

	if(p_lview_data->late_update_song_info)
	{
		SYS_LOG_INF("p_lview_data->late_update_song_info\n");
		p_lview_data->late_update_song_info = 0; //will be set to 1 again, if lcmusic still not inited
#ifdef CONFIG_LCMUSIC_APP
		lcmusic_display_song_info(NULL, false);
#endif
	}

#ifdef CONFIG_BT_TRANSMIT
	if (p_lview_data->update_bt_earphone_list) {
		p_lview_data->update_bt_earphone_list = 0;
		_display_bt_earphone_list_info();
	}else if (p_lview_data->bt_earphone_list_bg_img && p_lview_data->bt_earphone_info[0].name) {
		_bt_earphone_info_update_check();
	}
#endif

	os_mutex_unlock(&music_view_mutex);
}

static int _music_view_delete(view_data_t *view_data)
{
	lcmusic_view_data_t *data = view_data->user_data;

	if (data) {
		if (data->vol_slider || data->vol_bmp)
			_exit_vol_view();
		if (data->more_bmp || data->bt_state)
			_exit_more_view();
	#ifdef CONFIG_BT_TRANSMIT
		if (data->bt_earphone_list_bg_img)
			_exit_bt_earphone_list_view();

		/* free bt transmit device info */
		os_mutex_lock(&music_view_mutex, OS_FOREVER);
		for (int i = 0; i < MAX_SUPPORT_CNT; i++) {
			if (p_lview_data->bt_earphone_info[i].name)
				app_mem_free(p_lview_data->bt_earphone_info[i].name);
		}
		os_mutex_unlock(&music_view_mutex);
	#endif
	#ifdef CONFIG_BT_PLAYER
		btmusic_set_play_callback(NULL);
	#endif

		_unload_resource(data);
		_delete_obj_array(data->bmp, NUM_IMGS);
		_delete_obj_array(data->btn, NUM_BTNS);
		_delete_obj_array(data->lbl, NUM_TXTS);
		_reset_label_style_array(data->style_txt, NUM_TXTS);

		lv_obj_del(data->progress_arc);

		lv_style_reset(&data->style_arc_bg);
		lv_style_reset(&data->style_arc_red);

		os_mutex_lock(&music_view_mutex, OS_FOREVER);
		if (data->song_name) {
			app_mem_free(data->song_name);
			data->song_name = NULL;
		}
		if (data->album) {
			app_mem_free(data->album);
			data->album = NULL;
		}
	
		if (data->timer)
			lv_timer_del(data->timer);

		app_mem_free(data);
		view_data->user_data = NULL;
		
		p_lview_data = NULL;
		os_mutex_unlock(&music_view_mutex);
		
	} else {
		lvgl_res_preload_cancel_scene(SCENE_LCMUSIC_VIEW);
	}

	lvgl_res_unload_scene_compact(SCENE_LCMUSIC_VIEW);

	return 0;
}

static int _music_view_updated(view_data_t* view_data)
{
	return _music_view_layout_update(view_data, false);
}

static int _music_view_focus_changed(view_data_t *view_data, bool focused)
{
	lcmusic_view_data_t *data = view_data->user_data;
	launcher_app_t *app = launcher_app_get();

	if (focused)
	{
		if(!data)
		{
			return 0;
		}

		if(p_music_player)
		{
			app->cur_player = BTMUSIC_PLAYER;
		}

		if(data->timer)
		{
			lv_timer_resume(data->timer);
		}
		if(!lvgl_res_scene_is_loaded(SCENE_LCMUSIC_VIEW))
		{
			_music_view_preload(view_data, true);
		}
	}
	else
	{
		if(data)
		{
			_unload_pic_resource(data);
			if(data->timer)
			{
				lv_timer_pause(data->timer);
			}
		}
		lvgl_res_preload_cancel_scene(SCENE_LCMUSIC_VIEW);
		lvgl_res_unload_scene_compact(SCENE_LCMUSIC_VIEW);
	}

	return 0;
}

int _music_view_handler(uint16_t view_id, uint8_t msg_id, void * msg_data)
{
	view_data_t *view_data = msg_data;

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _music_view_preload(view_data, false);
	case MSG_VIEW_LAYOUT:
		return _music_view_layout(view_data);
	case MSG_VIEW_DELETE:
		return _music_view_delete(view_data);
	case MSG_VIEW_FOCUS:
		return _music_view_focus_changed(view_data, true);
	case MSG_VIEW_DEFOCUS:
		return _music_view_focus_changed(view_data, false);
	case MSG_VIEW_UPDATE:
		return _music_view_updated(view_data);
	case MSG_VIEW_PAINT:
	default:
		return 0;
	}
	return 0;
}

VIEW_DEFINE(music_view, _music_view_handler, NULL, \
		NULL, MUSIC_VIEW, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
