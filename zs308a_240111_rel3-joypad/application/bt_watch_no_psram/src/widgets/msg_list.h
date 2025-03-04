/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file msg_list.h
 *
 */

#ifndef BT_WATCH_SRC_WIDGETS_MSG_LIST_H_
#define BT_WATCH_SRC_WIDGETS_MSG_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of text page */
typedef struct {
	lv_obj_t obj; /*Ext. of ancestor*/
	uint8_t layout_en : 1;

	lv_style_t *sty_app_icon;
	lv_style_t *sty_app_name;
	lv_style_t *sty_name;
	lv_style_t *sty_text;
	lv_style_t *sty_time;
} msg_list_t;

extern const lv_obj_class_t msg_list_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an message list object
 * @param parent pointer to an object, it will be the parent of the new object
 * @return pointer to the created message list object
 */
lv_obj_t * msg_list_create(lv_obj_t * parent);

/**
 * Delete all messages
 * @param obj pointer to an object
 */
void msg_list_clean(lv_obj_t * obj);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set list title and text style
 *
 * The style should contain the size and the text prop.
 *
 * @param obj pointer to list object
 * @param sty_app_icon style applied to the application icon
 * @param sty_app_name style applied to the application name
 * @param sty_name style applied to the name
 * @param sty_text style applied to the title
 * @param sty_time style applied to the time
 * @param auto_en enable auto-layout for the sub-widgets or not. If enabled,
 *               some props, like position, in style will be ignored.
 */
void msg_list_set_layout(lv_obj_t * obj, lv_style_t *sty_app_icon,
		lv_style_t *sty_app_name, lv_style_t *sty_name, lv_style_t *sty_text,
		lv_style_t *sty_time, bool auto_en);

/**
 * Add list elements to the list
 * 
 * Convert the text to A4 image, then draw the image.
 *
 * @param obj pointer to list object
 * @param app_icon application icon
 * @param app_name application name
 * @param name who send the message
 * @param text message content
 * @param time message time
 * @return pointer to the new list element which can be customized (a button)
 */
lv_obj_t * msg_list_add(lv_obj_t * obj, const lv_img_dsc_t * app_icon,
		const char * app_name, const char * name, const char * text,
		const char * time);

/**
 * Remove the index of the element in the list
 * @param obj pointer to a list object
 * @param index the element index in the list
 */
void msg_list_remove(lv_obj_t * obj, uint16_t index);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the index of the button in the list
 * @param obj pointer to a list object. If NULL, assumes btn is part of a list.
 * @param btn pointer to a list element (button)
 * @return the index of the button in the list, or -1 of the button not in this list
 */
int16_t msg_list_get_btn_index(const lv_obj_t * obj, const lv_obj_t * btn);

/**
 * Get the number of buttons in the list
 * @param list pointer to a list object
 * @return the number of buttons in the list
 */
uint16_t msg_list_get_size(const lv_obj_t * obj);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BT_WATCH_SRC_WIDGETS_MSG_LIST_H_ */
