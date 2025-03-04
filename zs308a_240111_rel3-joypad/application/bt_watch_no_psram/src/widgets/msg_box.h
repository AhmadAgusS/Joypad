/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file msg_box.h
 *
 */

#ifndef BT_WATCH_SRC_WIDGETS_MSG_BOX_H_
#define BT_WATCH_SRC_WIDGETS_MSG_BOX_H_

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
	lv_coord_t h_max; /*maximum height of text area*/

	lv_style_t *sty_app_icon;
	lv_style_t *sty_app_name;
	lv_style_t *sty_name;
	lv_style_t *sty_text;
} msg_box_t;

extern const lv_obj_class_t msg_box_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an message box object
 * @param parent pointer to an object, it will be the parent of the new object
 * @return pointer to the created message box object
 */
lv_obj_t * msg_box_create(lv_obj_t * parent);

/**
 * Clean the text content
 *
 * @param obj pointer to box object
 */
void msg_box_clean(lv_obj_t *obj);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set box title and text style
 *
 * The style should contain the size and the text prop.
 *
 * @param obj pointer to box object
 * @param sty_app_icon style applied to the application icon
 * @param sty_app_name style applied to the application name
 * @param sty_name style applied to the name
 * @param sty_text style applied to the title
 * @param auto_en enable auto-layout for the sub-widgets or not. If enabled,
 *               some props, like position, in style will be ignored.
 */
void msg_box_set_layout(lv_obj_t * obj, lv_style_t *sty_app_icon,
		lv_style_t *sty_app_name, lv_style_t *sty_name, lv_style_t *sty_text,
		bool auto_en);

/**
 * Add box elements to the box
 *
 * Convert the text to A4 image, then draw the image.
 *
 * @param obj pointer to box object
 * @param app_icon application icon
 * @param app_name application name
 * @param name who send the message
 * @param text message content
 */
void msg_box_set_text(lv_obj_t * obj, const lv_img_dsc_t * app_icon,
		const char * app_name, const char * name, const char * text);

/**
 * Set the maximum height of the text area (including the scroll part).
 *
 * @param obj pointer to a text page object
 * @param h the maximum height of the text area, it take effect on next set_text()
 */
void msg_box_set_max_height(lv_obj_t * obj, lv_coord_t h);

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BT_WATCH_SRC_WIDGETS_MSG_BOX_H_ */
