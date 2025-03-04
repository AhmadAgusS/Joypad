/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <assert.h>
#include <lvgl.h>
#include <lvgl/lvgl_res_loader.h>
#include <lvgl/lvgl_bitmap_font.h>
#include <ui_manager.h>
#include <res_manager_api.h>
#include "widgets/text_canvas.h"
#include "widgets/watch_list.h"
#include "applist_view.h"

#define LIST_SCENE_ID SCENE_APPLIST565_VIEW

#if DEF_UI_WIDTH >= 454
#  define APPLIST_FONT DEF_FONT32_FILE
#else
#  define APPLIST_FONT DEF_FONT24_FILE
#endif /* DEF_UI_WIDTH == 454 */

enum applist_idx {
	STOPWATCH_IDX = 0,
	ALARMCLOCK_IDX,
	TIMER_IDX,
	PHONE_IDX,
	VIBRATOR_IDX,
	AOD_IDX,
	RECOVERY_IDX,
	POWEROFF_IDX,
	ALIPAY_IDX,
	LONGVIEW_IDX,
	COMPASS_IDX,

	NUM_ITEMS,

	PHONE_OFF_IDX = NUM_ITEMS,
	VIBRATOR_OFF_IDX,
	AOD_OFF_IDX,

	NUM_ICONS,

	PHONE_ON_IDX = PHONE_IDX,
	VIBRATOR_ON_IDX = VIBRATOR_IDX,
	AOD_ON_IDX = AOD_IDX,
};

typedef struct list_view_data {
	lv_obj_t *list;

	lv_font_t font;
	lv_style_t sty_txt;
	lv_style_t sty_btn;

	/* lvgl resource */
	lv_img_dsc_t img[NUM_ICONS];
	lvgl_res_string_t txt[NUM_ICONS];

	lv_coord_t item_height;
	lv_coord_t item_space;
	lv_coord_t pad_top;
	lv_coord_t pad_bottom;
	lv_coord_t track_radius;
	lv_point_t track_center;
} list_view_data_t;

static void _list_view_unload_resource(list_view_data_t *data);

static const uint32_t pic_grp_ids[] = {
	RES_STOPWATCH, RES_ALARMCLOCK, RES_TIMER, RES_PHONE_ON,
	RES_VIBRATION_ON, RES_AOD_ON, RES_RECOVERY, RES_POWEROFF,
	RES_ALIPAY, RES_TEST_LONGVIEW, RES_COMPASS, RES_PHONE_OFF,
	RES_VIBRATION_OFF, RES_AOD_OFF,
};

static lv_coord_t g_list_scrl_y = INT16_MAX;

static int _list_view_load_resource(list_view_data_t *data, lvgl_res_scene_t *res_scene)
{
	lvgl_res_group_t res_grp;
	int ret;

	/* scene */
	ret = lvgl_res_load_scene(LIST_SCENE_ID, res_scene, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
	if (ret < 0) {
		SYS_LOG_ERR("scene not found");
		return -ENOENT;
	}

	ret = lvgl_res_load_group_from_scene(res_scene, RES_DRAG_TRACK, &res_grp);
	if (ret < 0) {
		data->track_radius = 0;
	} else {
		data->track_center.x = res_grp.x;
		data->track_center.y = res_grp.y;
		data->track_radius = res_grp.width;
		lvgl_res_unload_group(&res_grp);
		SYS_LOG_INF("drag circle center (%d, %d), radius %d", data->track_center.x,
				data->track_center.y, data->track_radius);
	}

	ret = lvgl_res_load_group_from_scene(res_scene, RES_PADTOP, &res_grp);
	if (ret < 0) {
		data->pad_top = 0;
	} else {
		data->pad_top = res_grp.height;
		lvgl_res_unload_group(&res_grp);
	}

	ret = lvgl_res_load_group_from_scene(res_scene, RES_PADBOTTOM, &res_grp);
	if (ret < 0) {
		data->pad_bottom = 0;
	} else {
		data->pad_bottom = res_grp.height;
		lvgl_res_unload_group(&res_grp);
		SYS_LOG_INF("drag pad top %d, bottom %d", data->pad_top, data->pad_bottom);
	}

	for (int i = 0; i < ARRAY_SIZE(pic_grp_ids); i++) {
		uint32_t res_id;
		lv_point_t pic_pos;

		ret = lvgl_res_load_group_from_scene(res_scene, pic_grp_ids[i], &res_grp);
		if (ret < 0) {
			goto fail_exit;
		}

		res_id = PIC_ICON;
		ret = lvgl_res_load_pictures_from_group(&res_grp, &res_id, &data->img[i], &pic_pos, 1);
		if (ret < 0) {
			lvgl_res_unload_group(&res_grp);
			goto fail_exit;
		}

		res_id = STR_TEXT;
		ret = lvgl_res_load_strings_from_group(&res_grp, &res_id, &data->txt[i], 1);
		if (ret < 0) {
			lvgl_res_unload_group(&res_grp);
			goto fail_exit;
		}

		data->item_height = res_grp.height;
		data->item_space = data->txt[i].x - (pic_pos.x + data->img[i].header.w);
		lvgl_res_unload_group(&res_grp);
	}

#if defined(CONFIG_LV_USE_GPU) && defined(CONFIG_LV_COLOR_DEPTH_16)
	data->item_space &= ~0x1;
#endif

	/* open font */
	if (lvgl_bitmap_font_open(&data->font, APPLIST_FONT) < 0) {
		goto fail_exit;
	}

	lvgl_res_unload_scene(res_scene);
	return 0;
fail_exit:
	_list_view_unload_resource(data);
	lvgl_res_unload_scene(res_scene);
	return -ENOENT;
}

static void _list_view_unload_resource(list_view_data_t *data)
{
	lvgl_bitmap_font_close(&data->font);
	lvgl_res_unload_pictures(data->img, ARRAY_SIZE(data->img));
	lvgl_res_unload_strings(data->txt, ARRAY_SIZE(data->txt));
}

static lv_img_dsc_t * _get_image(view_data_t *view_data, uint8_t idx)
{
	const applist_view_presenter_t *presenter = view_get_presenter(view_data);
	list_view_data_t *data = view_data->user_data;

	switch (idx) {
	case PHONE_IDX:
		return presenter->phone_is_on() ?
				&data->img[PHONE_ON_IDX] : &data->img[PHONE_OFF_IDX];
	case VIBRATOR_IDX:
		return presenter->vibrator_is_on() ?
				&data->img[VIBRATOR_ON_IDX]: &data->img[VIBRATOR_OFF_IDX];
	case AOD_IDX:
		return presenter->aod_mode_is_on() ?
				&data->img[AOD_ON_IDX]: &data->img[AOD_OFF_IDX];
	default:
		return &data->img[idx];
	}
}

static const char * _get_text(view_data_t *view_data, uint8_t idx)
{
	const applist_view_presenter_t *presenter = view_get_presenter(view_data);
	list_view_data_t *data = view_data->user_data;

	switch (idx) {
	case PHONE_IDX:
		return presenter->phone_is_on() ?
				data->txt[PHONE_ON_IDX].txt : data->txt[PHONE_OFF_IDX].txt;
	case VIBRATOR_IDX:
		return presenter->vibrator_is_on() ?
				data->txt[VIBRATOR_ON_IDX].txt: data->txt[VIBRATOR_OFF_IDX].txt;
	case AOD_IDX:
		return presenter->aod_mode_is_on() ?
				data->txt[AOD_ON_IDX].txt: data->txt[AOD_OFF_IDX].txt;
	default:
		return data->txt[idx].txt;
	}
}

static void _list_btn_event_handler(lv_event_t * e)
{
	lv_obj_t *btn = lv_event_get_current_target(e);
	view_data_t *view_data = lv_event_get_user_data(e);
	const applist_view_presenter_t *presenter = view_get_presenter(view_data);
	list_view_data_t *data = view_data->user_data;
	bool update_img = false;

	int idx = (int)lv_obj_get_user_data(btn);

	SYS_LOG_INF("click btn %d", idx);

	switch (idx) {
	case PHONE_IDX:
		presenter->toggle_phone();
		update_img = true;
		break;
	case VIBRATOR_IDX:
		presenter->toggle_vibrator();
		update_img = true;
		break;
	case AOD_IDX:
		presenter->toggle_aod_mode();
		update_img = true;
		break;
	case STOPWATCH_IDX:
		presenter->open_stopwatch();
		break;
	case ALARMCLOCK_IDX:
		presenter->open_alarmclock();
		break;
	case COMPASS_IDX:
		presenter->open_compass();
		break;
	case LONGVIEW_IDX:
		/* TEST: open test long view */
		presenter->open_longview();
		break;
	case TIMER_IDX:
		break;
	case ALIPAY_IDX:
		presenter->open_alipay();
		break;
	default:
		break;
	}

	if (update_img) {
		lv_obj_t * img = watch_list_get_icon(data->list, idx);
		if (img)
			simple_img_set_src(img, _get_image(view_data, idx));

		lv_obj_t * txt = watch_list_get_text(data->list, idx);
		if (txt)
			text_canvas_set_text_static(txt, _get_text(view_data, idx));
	}
}

static void _list_scroll_handler(lv_obj_t *list, lv_point_t *icon_center)
{
	list_view_data_t *data = lv_obj_get_user_data(list);
	lv_coord_t cy = icon_center->y;

	cy -= data->track_center.y;
	if (cy < -data->track_radius || cy > data->track_radius) {
		icon_center->x = data->track_center.x;
		return;
	}

	lv_sqrt_res_t res;
	lv_sqrt(data->track_radius * data->track_radius - cy * cy, &res, 0x800);
	icon_center->x = data->track_center.x - res.i;
}

static int _list_view_preload(view_data_t *view_data)
{
	list_view_data_t *data = app_mem_malloc(sizeof(*data));
	if (data == NULL) {
		return -ENOMEM;
	}

	int res = lvgl_res_preload_scene_compact(LIST_SCENE_ID, NULL, 0,
			lvgl_res_scene_preload_default_cb_for_view, (void *)APPLIST_VIEW,
			DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
	if (res) {
		app_mem_free(data);
		return res;
	}

	memset(data, 0, sizeof(*data));
	view_data->user_data = data;
	return 0;
}

static int _list_view_layout(view_data_t *view_data)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);
	list_view_data_t *data = view_data->user_data;
	lvgl_res_scene_t res_scene;
	int i;

	if (_list_view_load_resource(data, &res_scene)) {
		return -ENOENT;
	}

	if (res_scene.width == lv_obj_get_width(scr) && res_scene.height == lv_obj_get_height(scr)) {
		data->list = watch_list_create(NULL);
		if (data->list == NULL) {
			_list_view_unload_resource(data);
			return -ENOMEM;
		}

		lv_disp_load_scr(data->list);
		lv_obj_del(scr);
		scr = data->list;
	} else {
		data->list = watch_list_create(scr);
		if (data->list == NULL) {
			_list_view_unload_resource(data);
			return -ENOMEM;
		}

		lv_obj_set_pos(data->list, res_scene.x, res_scene.y);
		lv_obj_set_size(data->list, res_scene.width, res_scene.height);
	}

	lv_obj_set_style_bg_color(scr, res_scene.background, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

	lv_obj_set_style_pad_top(data->list, data->pad_top, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(data->list, data->pad_bottom, LV_PART_MAIN);
	lv_obj_set_user_data(data->list, data);

	lv_style_init(&data->sty_txt);
	lv_style_set_text_font(&data->sty_txt, &data->font);
	lv_style_set_text_color(&data->sty_txt, data->txt[0].color);

	lv_style_init(&data->sty_btn);
	lv_style_set_height(&data->sty_btn, data->item_height);
	lv_style_set_pad_column(&data->sty_btn, data->item_space);

	/* Add buttons to the list */
	for (i = 0; i < NUM_ITEMS; i++) {
		lv_obj_t *btn = watch_list_add_btn3(data->list, _get_image(view_data, i), _get_text(view_data, i), &data->sty_txt);

		lv_obj_add_style(btn, &data->sty_btn, LV_PART_MAIN);
		lv_obj_add_event_cb(btn, _list_btn_event_handler, LV_EVENT_SHORT_CLICKED, view_data);
		lv_obj_set_user_data(btn, (void *)i);
	}

	if (data->track_radius > 0) {
		watch_list_set_scroll_cb(data->list, _list_scroll_handler);
	}

	if (g_list_scrl_y == INT16_MAX) {
		g_list_scrl_y = data->item_height;
	}

	/* must update layout before calling lv_obj_scroll_to_y() */
	lv_obj_update_layout(data->list);
	lv_obj_scroll_to_y(data->list, g_list_scrl_y, LV_ANIM_OFF);

	/* set system gesture */
	ui_manager_gesture_set_dir(GESTURE_HOR_BITFIELD);

	SYS_LOG_INF("applist view created");
	return 0;
}

static int _list_view_delete(view_data_t *view_data)
{
	list_view_data_t *data = view_data->user_data;

	/* restore system gesture */
	ui_manager_gesture_set_dir(GESTURE_ALL_BITFIELD);

	if (data->list) {
		g_list_scrl_y = lv_obj_get_scroll_y(data->list);

		lv_obj_del(data->list);
		lv_style_reset(&data->sty_txt);
		lv_style_reset(&data->sty_btn);
		_list_view_unload_resource(data);
	} else {
		lvgl_res_preload_cancel_scene(LIST_SCENE_ID);
	}

	lvgl_res_unload_scene_compact(LIST_SCENE_ID);

	app_mem_free(data);
	return 0;
}

int _list_view_handler(uint16_t view_id, uint8_t msg_id, void * msg_data)
{
	view_data_t *view_data = msg_data;

	assert(view_id == APPLIST_VIEW);

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _list_view_preload(view_data);
	case MSG_VIEW_LAYOUT:
		return _list_view_layout(view_data);
	case MSG_VIEW_DELETE:
		return _list_view_delete(view_data);
	default:
		return 0;
	}
}

VIEW_DEFINE(watch_applist, _list_view_handler, NULL, NULL, APPLIST_VIEW,
		NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
