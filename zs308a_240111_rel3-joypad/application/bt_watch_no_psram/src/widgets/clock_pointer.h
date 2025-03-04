/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file clock_pointer.h
 *
 */

#ifndef BT_WATCH_SRC_WIDGETS_CLOCK_POINTER_H_
#define BT_WATCH_SRC_WIDGETS_CLOCK_POINTER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include "simple_img.h"
#include "discrete_pointer.h"

/*********************
 *      DEFINES
 *********************/
#define USE_CONTINUOUS_POINTER  1

/**********************
 *      TYPEDEFS
 **********************/

/** Data of clock pointer */
typedef struct {
#if USE_CONTINUOUS_POINTER
	simple_img_t img;
#else
	discrete_pointer_t img;
#endif
} clock_pointer_t;

extern const lv_obj_class_t clock_pointer_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an clock pointer object
 * @param par pointer to an object, it will be the parent of the new object
 * @return pointer to the created clock pointer object
 */
lv_obj_t * clock_pointer_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the src images and their (same) pivot of the pointer
 * @param obj pointer to an clock pointer object
 * @param srcs Array of image sources
 * @param cnt Number of image sources in src array, must be 8 at persent
 * @param pivot_x: pivot x of the image sources
 * @param pivot_y: pivot y of the image sources
 */
void clock_pointer_set_images(lv_obj_t * obj, const lv_img_dsc_t * srcs,
		uint8_t cnt, lv_coord_t pivot_x, lv_coord_t pivot_y);

/**
 * Set the pointer angle.
 * @param obj pointer to an clock pointer object
 * @param angle angle in degree with 0.1 degree resolution, range [0, 3600)
 */
void clock_pointer_set_angle(lv_obj_t * obj, uint16_t angle);

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

#endif /* BT_WATCH_SRC_WIDGETS_CLOCK_POINTER_H_ */
