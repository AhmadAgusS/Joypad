/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file msg_box.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <sys/util.h>

#include "msg_box.h"
#include "simple_img.h"
#include "text_canvas.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &msg_box_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t msg_box_class = {
	.width_def = LV_PCT(100),
	.height_def = LV_PCT(100),
	.instance_size = sizeof(msg_box_t),
	.base_class = &lv_obj_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * msg_box_create(lv_obj_t * parent)
{
	LV_LOG_INFO("begin");
	lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
	lv_obj_class_init_obj(obj);

	lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE);
	return obj;
}

void msg_box_clean(lv_obj_t * obj)
{
	lv_obj_clean(obj);
}

/*=====================
 * Setter functions
 *====================*/

void msg_box_set_layout(lv_obj_t * obj, lv_style_t *sty_app_icon,
		lv_style_t *sty_app_name, lv_style_t *sty_name, lv_style_t *sty_text,
		bool auto_en)
{
	msg_box_t * list = (msg_box_t *)obj;

	if (auto_en) {
		lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_set_style_pad_row(obj, 10, LV_PART_MAIN);
		lv_obj_set_style_pad_column(obj, 10, LV_PART_MAIN);
	} else {
		lv_obj_set_style_layout(obj, 0, LV_PART_MAIN);
	}

	list->sty_app_icon = sty_app_icon;
	list->sty_app_name = sty_app_name;
	list->sty_name = sty_name;
	list->sty_text = sty_text;
}

void msg_box_set_text(lv_obj_t * obj, const lv_img_dsc_t * app_icon,
		const char * app_name, const char * name, const char * text)
{
	msg_box_t * list = (msg_box_t *)obj;

	lv_obj_clean(obj);

	if (app_icon) {
		lv_obj_t *obj_appicon = simple_img_create(obj);
		simple_img_set_src(obj_appicon, app_icon);
		if (list->sty_app_icon)
			lv_obj_add_style(obj_appicon, list->sty_app_icon, LV_PART_MAIN);
	}

	if (app_name) {
		lv_obj_t *obj_appname = lv_label_create(obj);
		//lv_obj_set_flex_grow(obj_appname, 1);
		lv_obj_add_style(obj_appname, list->sty_app_name, LV_PART_MAIN);
		lv_label_set_long_mode(obj_appname, LV_LABEL_LONG_CLIP /* LV_LABEL_LONG_SCROLL_CIRCULAR */);
		lv_label_set_text(obj_appname, app_name);
	}

	if (name) {
		lv_obj_t *obj_name = lv_label_create(obj);
		//lv_obj_set_flex_grow(obj_name, 1);
		lv_obj_add_style(obj_name, list->sty_name, LV_PART_MAIN);
		lv_label_set_long_mode(obj_name, LV_LABEL_LONG_CLIP /* LV_LABEL_LONG_SCROLL_CIRCULAR */);
		lv_label_set_text(obj_name, name);
	}

	lv_obj_t *obj_text = text_canvas_create(obj);
#ifdef CONFIG_BITMAP_FONT_SUPPORT_EMOJI
	text_canvas_set_emoji_enable(obj_text, true);
#endif
	lv_obj_add_flag(obj_text, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
	lv_obj_add_style(obj_text, list->sty_text, LV_PART_MAIN);
	lv_obj_set_scroll_dir(obj_text, LV_DIR_VER);
	text_canvas_set_max_height(obj_text, list->h_max);
	text_canvas_set_text_static(obj_text, text);
}

void msg_box_set_max_height(lv_obj_t * obj, lv_coord_t h)
{
	msg_box_t * list = (msg_box_t *)obj;

	list->h_max = h;
}

/*=====================
 * Getter functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/
