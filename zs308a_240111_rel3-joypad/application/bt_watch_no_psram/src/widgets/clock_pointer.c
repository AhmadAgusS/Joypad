/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file clock_pointer.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "clock_pointer.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &clock_pointer_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t clock_pointer_class = {
	.width_def = LV_SIZE_CONTENT,
	.height_def = LV_SIZE_CONTENT,
	.instance_size = sizeof(clock_pointer_t),
#if USE_CONTINUOUS_POINTER
	.base_class = &simple_img_class,
#else
	.base_class = &dicrete_pointer_class,
#endif
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * clock_pointer_create(lv_obj_t * parent)
{
	LV_LOG_INFO("begin");
	lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
	lv_obj_class_init_obj(obj);
	return obj;
}

/*=====================
 * Setter functions
 *====================*/

void clock_pointer_set_images(lv_obj_t *pointer, const lv_img_dsc_t *srcs,
		uint8_t cnt, lv_coord_t pivot_x, lv_coord_t pivot_y)
{
	lv_coord_t par_w = lv_obj_get_style_width(pointer->parent, LV_PART_MAIN);
	lv_coord_t par_h = lv_obj_get_style_height(pointer->parent, LV_PART_MAIN);

#if USE_CONTINUOUS_POINTER
	pivot_y = srcs[0].header.h - 1 - pivot_y;
	lv_obj_set_pos(pointer, par_w / 2 - pivot_x, par_h / 2 - pivot_y);
	simple_img_set_src(pointer, srcs);
	simple_img_set_pivot(pointer, pivot_x, pivot_y);
#else
	discrete_pointer_set_src(pointer, srcs, cnt);
	discrete_pointer_set_pivot(pointer, pivot_x, pivot_y,  par_w / 2, par_h / 2);
#endif
}

void clock_pointer_set_angle(lv_obj_t *pointer, uint16_t angle)
{
#if USE_CONTINUOUS_POINTER
	simple_img_set_angle(pointer, angle);
#else
	discrete_pointer_set_angle(pointer, angle / 10);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
