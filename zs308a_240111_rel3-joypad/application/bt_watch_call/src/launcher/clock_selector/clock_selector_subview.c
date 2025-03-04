/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lvgl.h>
#include <lvgl/lvgl_bitmap_font.h>
#include <lvgl/lvgl_res_loader.h>
#include <lvgl/lvgl_img_loader.h>
#include <ui_manager.h>
#include <view_manager.h>
#include "widgets/simple_img.h"
#include "clock_selector_subview.h"

#define SCENE_CLOCK_SELECTOR_ID SCENE_CLOCK_SEL_VIEW

#define PRELOAD_5_PICS

typedef struct clocksel_slide_data {
	lv_font_t font;
	lv_style_t sty_txt;
	lv_style_t sty_bg;

	lvgl_res_picregion_t picreg;
	lvgl_res_string_t strtxt[1];
	lvgl_res_scene_t scene;

	uint8_t ref_count;
} clocksel_share_data_t;

typedef struct clocksel_subview_data {
	lv_obj_t *img;
	lv_obj_t *txt;

	lv_img_dsc_t src;
	int8_t clk_idx;
} clocksel_subview_data_t;

static void _clocksel_unload_shared_resource(void);
static int _clocksel_subview_paint(uint16_t view_id, view_data_t *view_data);

static clocksel_share_data_t *clocksel_dptr;

static int _clocksel_load_shared_resource(void)
{
	uint32_t res_id = STR_NAME;
	int ret;

	if (clocksel_dptr) {
		clocksel_dptr->ref_count++;
		return 0;
	}

	clocksel_dptr = app_mem_malloc(sizeof(*clocksel_dptr));
	if (clocksel_dptr == NULL)
		return -ENOMEM;

	clocksel_dptr->ref_count = 1;

	ret = lvgl_res_load_scene(SCENE_CLOCK_SELECTOR_ID, &clocksel_dptr->scene,
			DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
	if (ret) {
		SYS_LOG_ERR("scene selector view not found");
		app_mem_free(clocksel_dptr);
		clocksel_dptr = NULL;
		return -ENOENT;
	}

	ret = lvgl_res_load_picregion_from_scene(&clocksel_dptr->scene, RES_THUMBNAIL, &clocksel_dptr->picreg);
	if (ret || clocksel_dptr->picreg.frames < 1) {
		SYS_LOG_ERR("cannot find picreg 0x%x\n", RES_THUMBNAIL);
		goto fail_exit;
	}

	ret = lvgl_res_load_strings_from_scene(&clocksel_dptr->scene, &res_id,
			clocksel_dptr->strtxt, ARRAY_SIZE(clocksel_dptr->strtxt));
	if (ret == 0) {
#if DEF_UI_WIDTH < 454
		ret = lvgl_bitmap_font_open(&clocksel_dptr->font, DEF_FONT24_FILE);
#else
		ret = lvgl_bitmap_font_open(&clocksel_dptr->font, DEF_FONT32_FILE);
#endif
		if (ret) {
			goto fail_exit;
		}
	}

	lv_style_init(&clocksel_dptr->sty_bg);
	lv_style_set_bg_color(&clocksel_dptr->sty_bg, clocksel_dptr->scene.background);
	lv_style_set_bg_opa(&clocksel_dptr->sty_bg, LV_OPA_COVER);

	if (clocksel_dptr->strtxt[0].txt) {
		lv_style_init(&clocksel_dptr->sty_txt);
		lv_style_set_text_font(&clocksel_dptr->sty_txt, &clocksel_dptr->font);
		lv_style_set_text_color(&clocksel_dptr->sty_txt, clocksel_dptr->strtxt[0].color);
		lv_style_set_text_align(&clocksel_dptr->sty_txt, LV_TEXT_ALIGN_CENTER);
		lv_style_set_bg_opa(&clocksel_dptr->sty_txt, LV_OPA_TRANSP);
		lv_style_set_align(&clocksel_dptr->sty_txt, LV_ALIGN_TOP_MID);
	}

	return 0;
fail_exit:
	_clocksel_unload_shared_resource();
	return -ENOENT;
}

static void _clocksel_unload_shared_resource(void)
{
	if (!clocksel_dptr || --clocksel_dptr->ref_count > 0) {
		return;
	}

	lvgl_res_preload_cancel();

	lv_style_reset(&clocksel_dptr->sty_bg);
	if (clocksel_dptr->strtxt[0].txt) {
		lv_style_reset(&clocksel_dptr->sty_txt);
		lvgl_bitmap_font_close(&clocksel_dptr->font);
		lvgl_res_unload_strings(clocksel_dptr->strtxt, ARRAY_SIZE(clocksel_dptr->strtxt));
	}

	lvgl_res_unload_picregion(&clocksel_dptr->picreg);
	lvgl_res_unload_scene(&clocksel_dptr->scene);
	lvgl_res_unload_scene_compact(SCENE_CLOCK_SELECTOR_ID);

	app_mem_free(clocksel_dptr);
	clocksel_dptr = NULL;

	SYS_LOG_INF("shared resource free");
}

static int _clocksel_subview_preload(uint16_t view_id, view_data_t *view_data)
{
	clocksel_subview_data_t *data;

	data = app_mem_malloc(sizeof(*data));
	if (data == NULL) {
		return -ENOMEM;
	}

	memset(data, 0, sizeof(*data));
	data->clk_idx = -1;

	if (_clocksel_load_shared_resource()) {
		app_mem_free(data);
		return -ENOENT;
	}

	view_data->user_data = data;
	ui_view_layout(view_id);
	return 0;
}

static void _clocksel_slide_click_event_handler(lv_event_t * e)
{
	view_data_t *view_data = lv_event_get_user_data(e);
	const clocksel_subview_presenter_t *presenter = view_get_presenter(view_data);
	clocksel_subview_data_t *data = view_data->user_data;

	SYS_LOG_INF("select clock %d", data->clk_idx);
	presenter->set_clock_id(data->clk_idx);
}

static int _clocksel_subview_layout(uint16_t view_id, view_data_t *view_data)
{
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);
	clocksel_subview_data_t *data = view_data->user_data;

	data->img = simple_img_create(scr);
	if (data->img == NULL) {
		return -ENOMEM;
	}

	if (clocksel_dptr->strtxt[0].txt) {
		data->txt = lv_label_create(scr);
		if (data->txt == NULL) {
			return -ENOMEM;
		}

		lv_obj_add_style(data->txt, &clocksel_dptr->sty_txt, LV_PART_MAIN);
	}

	lv_obj_add_style(scr, &clocksel_dptr->sty_bg, LV_PART_MAIN);
	lv_obj_add_style(data->img, &clocksel_dptr->sty_bg, LV_PART_MAIN);
	lv_obj_add_flag(data->img, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(data->img, _clocksel_slide_click_event_handler, LV_EVENT_SHORT_CLICKED, view_data);
	lv_obj_set_pos(data->img, clocksel_dptr->picreg.x, clocksel_dptr->picreg.y);
	lv_obj_set_size(data->img, clocksel_dptr->picreg.width, clocksel_dptr->picreg.height);

	_clocksel_subview_paint(view_id, view_data);
	SYS_LOG_INF("clock selector view inflated");
	return 0;
}

static int _clocksel_subview_load(uint16_t view_id, view_data_t *view_data)
{
	clocksel_subview_data_t *data = view_data->user_data;
	const clocksel_subview_presenter_t *presenter = view_get_presenter(view_data);
	int8_t clk_idx = presenter->get_clock_id(view_id);
	int ret;

	if (clk_idx < 0 || clk_idx >= clocksel_dptr->picreg.frames)
		return -EINVAL;

	ret = lvgl_res_load_pictures_from_picregion(
				&clocksel_dptr->picreg, clk_idx, clk_idx, &data->src);
	if (ret == 0) {
		lv_img_cache_invalidate_src(&data->src);
	}

	return ret;
}

static int _clocksel_subview_paint(uint16_t view_id, view_data_t *view_data)
{
	clocksel_subview_data_t *data = view_data->user_data;
	const clocksel_subview_presenter_t *presenter = view_get_presenter(view_data);
	int8_t clk_idx = presenter->get_clock_id(view_id);
	int ret;

	SYS_LOG_INF("view %d, clock %d->%d", view_id, data->clk_idx, clk_idx);

	ret = _clocksel_subview_load(view_id, view_data);
	if (ret == 0) {
		simple_img_set_src(data->img, &data->src);
		if (data->txt) {
			const char *name = presenter->get_clock_name(clk_idx);
			lv_label_set_text_static(data->txt, name ? name : "unknown");
		}

		/* Only refresh once */
		bool focused = view_is_focused(view_id);
		if (!focused) {
			view_set_refresh_en(view_id, true);
		}

		lv_refr_now(view_data->display);

		if (!focused) {
			view_set_refresh_en(view_id, false);
			lvgl_res_unload_pictures(&data->src, 1);
		}
	}

	data->clk_idx = clk_idx;
	return 0;
}

static int _clocksel_subview_focus(uint16_t view_id, view_data_t *view_data, bool focused)
{
	clocksel_subview_data_t *data = view_data->user_data;

	if (focused) {
		if (data->src.data == NULL)
			_clocksel_subview_load(view_id, view_data);
	} else {
		lvgl_res_unload_pictures(&data->src, 1);
	}

	return 0;
}

static int _clocksel_subview_delete(view_data_t *view_data)
{
	clocksel_subview_data_t *data = view_data->user_data;
	lv_obj_t *scr = lv_disp_get_scr_act(view_data->display);

	if (data->img) {
		lv_obj_remove_style(scr, &clocksel_dptr->sty_bg, LV_PART_MAIN);
		lv_obj_remove_style(data->img, &clocksel_dptr->sty_bg, LV_PART_MAIN);
		lvgl_res_unload_pictures(&data->src, 1);
	}

	if (data->txt) {
		lv_obj_remove_style(data->txt, &clocksel_dptr->sty_txt, LV_PART_MAIN);
	}

	_clocksel_unload_shared_resource();
	app_mem_free(data);
	return 0;
}

int _clocksel_subview_handler(uint16_t view_id, uint8_t msg_id, void *msg_data)
{
	view_data_t *view_data = msg_data;

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _clocksel_subview_preload(view_id, view_data);
	case MSG_VIEW_LAYOUT:
		return _clocksel_subview_layout(view_id, view_data);
	case MSG_VIEW_PAINT:
		return _clocksel_subview_paint(view_id, view_data);
	case MSG_VIEW_FOCUS:
		return _clocksel_subview_focus(view_id, view_data, true); 
	case MSG_VIEW_DEFOCUS:
		return _clocksel_subview_focus(view_id, view_data, false);
	case MSG_VIEW_DELETE:
		return _clocksel_subview_delete(view_data);
	default:
		return 0;
	}
}

VIEW_DEFINE(clocksel_subview_0, _clocksel_subview_handler, NULL,
		NULL, CLOCK_SELECTOR_SUBVIEW_0, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
VIEW_DEFINE(clocksel_subview_1, _clocksel_subview_handler, NULL,
		NULL, CLOCK_SELECTOR_SUBVIEW_1, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
VIEW_DEFINE(clocksel_subview_2, _clocksel_subview_handler, NULL,
		NULL, CLOCK_SELECTOR_SUBVIEW_2, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
VIEW_DEFINE(clocksel_subview_3, _clocksel_subview_handler, NULL,
		NULL, CLOCK_SELECTOR_SUBVIEW_3, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
VIEW_DEFINE(clocksel_subview_4, _clocksel_subview_handler, NULL,
		NULL, CLOCK_SELECTOR_SUBVIEW_4, NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
