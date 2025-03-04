/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file msg_list.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <sys/util.h>

#include "msg_list.h"
#include "text_canvas.h"
#include "simple_img.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &msg_list_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void msg_list_event(const lv_obj_class_t * class_p, lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t msg_list_class = {
	.event_cb = msg_list_event,
	.width_def = LV_PCT(100),
	.height_def = LV_PCT(100),
	.instance_size = sizeof(msg_list_t),
	.base_class = &lv_obj_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * msg_list_create(lv_obj_t * parent)
{
	LV_LOG_INFO("begin");
	lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
	lv_obj_class_init_obj(obj);

	lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_scroll_dir(obj, LV_DIR_VER);
	return obj;
}

void msg_list_clean(lv_obj_t * obj)
{
	lv_obj_clean(obj);
}

/*=====================
 * Setter functions
 *====================*/

void msg_list_set_layout(lv_obj_t * obj, lv_style_t *sty_app_icon,
		lv_style_t *sty_app_name, lv_style_t *sty_name, lv_style_t *sty_text,
		lv_style_t *sty_time, bool auto_en)
{
	msg_list_t * list = (msg_list_t *)obj;

	list->layout_en = auto_en;
	list->sty_app_icon = sty_app_icon;
	list->sty_app_name = sty_app_name;
	list->sty_name = sty_name;
	list->sty_text = sty_text;
	list->sty_time = sty_time;
}

lv_obj_t * msg_list_add(lv_obj_t * obj, const lv_img_dsc_t * app_icon,
		const char * app_name, const char * name, const char * text,
		const char * time)
{
	msg_list_t * list = (msg_list_t *)obj;

	lv_obj_t *item = lv_obj_create(obj);
	lv_obj_set_size(item, LV_PCT(100), LV_SIZE_CONTENT);

	if (list->layout_en) {
		lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_flex_cross_place(item, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_set_style_pad_column(item, 10, LV_PART_MAIN);
	}

	if (app_icon) {
		lv_obj_t *obj_appicon = simple_img_create(item);
		simple_img_set_src(obj_appicon, app_icon);
		if (list->sty_app_icon)
			lv_obj_add_style(obj_appicon, list->sty_app_icon, LV_PART_MAIN);
	}

	if (app_name) {
		lv_obj_t *obj_appname = text_canvas_create(item);
		//lv_obj_set_flex_grow(obj_appname, 1);
		lv_obj_add_style(obj_appname, list->sty_app_name, LV_PART_MAIN);
		text_canvas_set_max_height(obj_appname, lv_obj_get_style_height(obj_appname, LV_PART_MAIN));
		text_canvas_set_text_static(obj_appname, app_name);
	}

	lv_obj_t *btn = lv_btn_create(item);
	lv_obj_add_flag(btn, LV_OBJ_FLAG_SCROLL_CHAIN);
	lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	//lv_obj_set_size(btn, LV_PCT(100), LV_SIZE_CONTENT);
	//lv_obj_set_flex_grow(btn, 1);
	if (list->layout_en) {
		lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
	}

	if (name) {
		lv_obj_t *obj_name = text_canvas_create(btn);
		lv_obj_add_flag(obj_name, LV_OBJ_FLAG_EVENT_BUBBLE);
		lv_obj_clear_flag(obj_name, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_style(obj_name, list->sty_name, LV_PART_MAIN);
		text_canvas_set_max_height(obj_name, lv_obj_get_style_height(obj_name, LV_PART_MAIN));
		text_canvas_set_text_static(obj_name, name);
	}

	if (text) {
		lv_obj_t *obj_text = text_canvas_create(btn);
#ifdef CONFIG_BITMAP_FONT_SUPPORT_EMOJI
		text_canvas_set_emoji_enable(obj_text, true);
#endif
		lv_obj_add_flag(obj_text, LV_OBJ_FLAG_EVENT_BUBBLE);
		lv_obj_clear_flag(obj_text, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_style(obj_text, list->sty_text, LV_PART_MAIN);
		text_canvas_set_max_height(obj_text, lv_obj_get_style_height(obj_text, LV_PART_MAIN));
		/* use default TEXT_CANVAS_LONG_WRAP for performance */
		//text_canvas_set_long_mode(obj_text, TEXT_CANVAS_LONG_DOT);
		text_canvas_set_text_static(obj_text, text);
	}

	if (time) {
		lv_obj_t *obj_time = text_canvas_create(btn);
		lv_obj_add_flag(obj_time, LV_OBJ_FLAG_EVENT_BUBBLE);
		lv_obj_clear_flag(obj_time, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_style(obj_time, list->sty_time, LV_PART_MAIN);
		text_canvas_set_max_height(obj_time, lv_obj_get_style_height(obj_time, LV_PART_MAIN));
		text_canvas_set_text_static(obj_time, time);
	}

	return btn;
}

void msg_list_remove(lv_obj_t * obj, uint16_t index)
{
	lv_obj_t * child = lv_obj_get_child(obj, index);

	if (child) {
		lv_obj_del(child);
	}
}

/*=====================
 * Getter functions
 *====================*/

int16_t msg_list_get_btn_index(const lv_obj_t * obj, const lv_obj_t * btn)
{
	int16_t index = 0;

	for (index = lv_obj_get_child_cnt(obj) - 1; index >= 0; index--) {
		if (btn == lv_obj_get_child(obj, index)) {
			break;
		}
	}

	return index;
}

uint16_t msg_list_get_size(const lv_obj_t * obj)
{
	return lv_obj_get_child_cnt(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void msg_list_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
	LV_UNUSED(class_p);

	/*Call the ancestor's event handler*/
	lv_res_t res = lv_obj_event_base(MY_CLASS, e);
	if(res != LV_RES_OK) return;
}
