/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file heart view
 */
#include <assert.h>
#include <lvgl.h>
#include <lvgl/lvgl_res_loader.h>
#include <lvgl/lvgl_bitmap_font.h>
#include <view_manager.h>
#include <res_manager_api.h>
#include "app_ui.h"
#include "app_defines.h"
#include "system_app.h"
#include "heart_view.h"
#ifdef CONFIG_SENSOR_MANAGER
#include <sensor_manager.h>
#endif

enum {
	BMP_BG = 0,
	NUM_BMPS,
};

enum {
	TXT_HRATE = 0,
	NUM_TXTS,
};

const static uint32_t _bmp_ids[] = {
	PIC_BG,
};

const static uint32_t _txt_ids[] = {
	STR_HRATE,
};

static int32_t heart_preload_inited = 0;

typedef struct heart_view_data {
	/* lvgl object */
	lv_obj_t *img;
	lv_obj_t *lbl;

	/* ui-editor resource */
	lvgl_res_scene_t res_scene;
	lv_img_dsc_t img_dsc_bmp[NUM_BMPS];
	lvgl_res_string_t res_txt[NUM_TXTS];
	
	/* lvgl resource */
	lv_point_t pt_bmp[NUM_BMPS];
	lv_point_t pt_txt[NUM_TXTS];
	lv_style_t style_txt[NUM_TXTS];
	lv_font_t font;
	
	/* user data */
	uint32_t hrate_val;
	char hrate_buf[4];
} heart_view_data_t;

static int _heart_view_paint(view_data_t *view_data);

static void _cvt_txt_array(lv_point_t *pt, lv_style_t *sty, lv_font_t *font, lvgl_res_string_t *txt, uint32_t num)
{
	int i;
	
	for (i = 0; i < num; i++) {
		pt[i].x = txt[i].x;
		pt[i].y = txt[i].y;

		lv_style_init(&sty[i]);
		lv_style_set_text_font(&sty[i], font);
		lv_style_set_text_color(&sty[i], txt[i].color);
	}
}

static int _load_resource(heart_view_data_t *data, bool first_layout)
{
	int32_t ret;

	/* load scene */
	if(first_layout)
	{
		ret = lvgl_res_load_scene(SCENE_HEART_VIEW, &data->res_scene, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
		if (ret < 0) {
			SYS_LOG_ERR("SCENE_HEART_VIEW not found");
			return -ENOENT;
		}
	}

	/* load resource */
	lvgl_res_load_pictures_from_scene(&data->res_scene, _bmp_ids, data->img_dsc_bmp, data->pt_bmp, NUM_BMPS);

	if(first_layout)
	{
		/* open font */
#if DEF_UI_WIDTH < 454
		if (lvgl_bitmap_font_open(&data->font, DEF_FONT24_FILE) < 0) {
			SYS_LOG_ERR("font not found");
			return -ENOENT; 
		}
#else
		if (lvgl_bitmap_font_open(&data->font, DEF_FONT32_FILE) < 0) {
			SYS_LOG_ERR("font not found");
			return -ENOENT; 
		}
#endif
	
		lvgl_res_load_strings_from_scene(&data->res_scene, _txt_ids, data->res_txt, NUM_TXTS);
		/* convert resource */
		_cvt_txt_array(data->pt_txt, data->style_txt, &data->font, data->res_txt, NUM_TXTS);
	}
	SYS_LOG_INF("load resource succeed");

	return 0;
}

static void _unload_pic_resource(heart_view_data_t *data)
{
    lvgl_res_unload_pictures(data->img_dsc_bmp, NUM_BMPS);
}

static void _unload_resource(heart_view_data_t *data)
{
	lvgl_res_unload_pictures(data->img_dsc_bmp, NUM_BMPS);
	lvgl_res_unload_strings(data->res_txt, NUM_TXTS);

	lvgl_bitmap_font_close(&data->font);

	lvgl_res_unload_scene(&data->res_scene);
}

static int _heart_view_preload(view_data_t *view_data, bool update)
{
	if (heart_preload_inited == 0) {
		lvgl_res_preload_scene_compact_default_init(SCENE_HEART_VIEW, NULL, 0,
			NULL, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
		heart_preload_inited = 1;
	}
	
	return lvgl_res_preload_scene_compact_default(SCENE_HEART_VIEW, HEART_VIEW, update, 0);
}

static int _heart_view_layout_update(view_data_t *view_data, bool first_layout)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);
	heart_view_data_t *data = view_data->user_data;

	if(first_layout)
	{
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

	// create image
	if(first_layout)
	{
		data->img = lv_img_create(scr);
		lv_obj_set_pos(data->img, data->pt_bmp[BMP_BG].x, data->pt_bmp[BMP_BG].y);
	}
	lv_img_set_src(data->img, &data->img_dsc_bmp[BMP_BG]);
	
	// create label
	if(first_layout)
	{
		data->lbl = lv_label_create(scr);
		lv_obj_set_pos(data->lbl, data->pt_txt[TXT_HRATE].x, data->pt_txt[TXT_HRATE].y);
		lv_obj_add_style(data->lbl, &data->style_txt[TXT_HRATE], LV_PART_MAIN);
	}

	return 0;
}

static int _heart_view_layout(view_data_t *view_data)
{
	int ret;
	
	ret = _heart_view_layout_update(view_data, true);
	if(ret < 0)
	{
		return -1;
	}
	
	// paint view
	_heart_view_paint(view_data);
	lv_refr_now(view_data->display);
	SYS_LOG_INF("_heart_view_layout");
	
	return 0;
}

static int _heart_view_paint(view_data_t *view_data)
{
	const heart_view_presenter_t *presenter = view_get_presenter(view_data);
	heart_view_data_t *data = view_data->user_data;

	if (data) {
		SYS_LOG_INF("_heart_view_paint");

		data->hrate_val = presenter->get_heart_rate();
		snprintf(data->hrate_buf, sizeof(data->hrate_buf), "%02d", data->hrate_val);
		lv_label_set_text(data->lbl, data->hrate_buf);
	}

	return 0;
}

static int _heart_view_delete(view_data_t *view_data)
{
	heart_view_data_t *data = view_data->user_data;
	int i;

	if (data) {
		_unload_resource(data);
		lv_obj_del(data->img);
		lv_obj_del(data->lbl);
		for (i = 0; i < ARRAY_SIZE(data->style_txt); i++) {
			lv_style_reset(&data->style_txt[i]);
		}
		app_mem_free(data);
		view_data->user_data = NULL;
	} else {
		lvgl_res_preload_cancel_scene(SCENE_HEART_VIEW);
	}

	lvgl_res_unload_scene_compact(SCENE_HEART_VIEW);
	return 0;
}


static int _heart_view_updated(view_data_t* view_data)
{
	int ret;

	ret = _heart_view_layout_update(view_data, false);
	
	return ret;
	
}

static int _heart_view_focus_changed(view_data_t *view_data, bool focused)
{
	heart_view_data_t *data = view_data->user_data;

	if (focused) 
	{
#ifdef CONFIG_SENSOR_MANAGER	
		sensor_manager_enable(IN_HEARTRATE, 0);
#endif

		if(!lvgl_res_scene_is_loaded(SCENE_HEART_VIEW))
		{
			_heart_view_preload(view_data, true);
		}
	}
	else
	{
#ifdef CONFIG_SENSOR_MANAGER
		sensor_manager_disable(IN_HEARTRATE, 0);
#endif
        if(data)
        {
            _unload_pic_resource(data);        
        }
        
		lvgl_res_preload_cancel_scene(SCENE_HEART_VIEW);
		lvgl_res_unload_scene_compact(SCENE_HEART_VIEW);
	}

	return 0;
}

int _heart_view_handler(uint16_t view_id, uint8_t msg_id, void * msg_data)
{
	view_data_t *view_data = msg_data;

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _heart_view_preload(view_data, false);
	case MSG_VIEW_LAYOUT:
		return _heart_view_layout(view_data);
	case MSG_VIEW_DELETE:
#ifdef CONFIG_SENSOR_MANAGER
		sensor_manager_disable(IN_HEARTRATE, 0);
#endif
		return _heart_view_delete(view_data);
	case MSG_VIEW_PAINT:
		return _heart_view_paint(view_data);
	case MSG_VIEW_FOCUS:
		return _heart_view_focus_changed(view_data, true);
	case MSG_VIEW_DEFOCUS:
		return _heart_view_focus_changed(view_data, false);
	case MSG_VIEW_UPDATE:
		return _heart_view_updated(view_data);
	default:
		return 0;
	}
	return 0;
}

VIEW_DEFINE(heart_view, _heart_view_handler, NULL, \
		NULL, HEART_VIEW, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
