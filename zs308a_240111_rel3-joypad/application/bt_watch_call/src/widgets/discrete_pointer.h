/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file discrete_pointer.h
 *
 */

#ifndef BT_WATCH_SRC_WIDGETS_DISCRETE_POINTER_H_
#define BT_WATCH_SRC_WIDGETS_DISCRETE_POINTER_H_

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

/** Data of discrete clock pointer */
typedef struct {
	lv_obj_t obj;

	const lv_img_dsc_t *cur_src;
	const lv_img_dsc_t *srcs; /* Image source array */
	uint8_t num_srcs;
	uint8_t quadrant;
	uint16_t angle;   /* Pointer angle range [0, 360) */
	lv_point_t pivot; /* Anchor of the images (measured from left-bottom corner) */
	lv_point_t pos;   /* Position of the pivot from the left-top of the parent */
} discrete_pointer_t;

extern const lv_obj_class_t discrete_pointer_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an discrete clock pointer object
 * @param parent pointer to an object, it will be the parent of the new object
 * @return pointer to the created clock pointer object
 */
lv_obj_t * discrete_pointer_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the pixel map array of the pointer
 * @param obj pointer to an clock pointer object
 * @param srcs Array of image sources corresponding to the top half of the first quarant.
 * @param cnt Number of image sources in src array, can be 8/16, etc.
 */
void discrete_pointer_set_src(lv_obj_t *obj, const lv_img_dsc_t * srcs, uint8_t cnt);

/**
 * Set the position of the pivot of the image sources
 * 
 * The pivot is offset of the image rotation center from the left-bootom corner,
 * and the image must be vertical and pointing to the top.
 * 
 * @param obj pointer to an clock pointer object
 * @param pivot_x: pivot x of the image sources
 * @param pivot_y: pivot y of the image sources
 * @param pos_x: position x of the image pivot from the left of the parent
 * @param pos_y: position y of the image pivot from the top of the parent
 */
void discrete_pointer_set_pivot(lv_obj_t *obj,
		lv_coord_t pivot_x, lv_coord_t pivot_y, lv_coord_t pos_x, lv_coord_t pos_y);

/**
 * Set the pointer angle.
 * @param obj pointer to an clock pointer object
 * @param angle angle in degree with 1 degree resolution, range [0, 360)
 */
void discrete_pointer_set_angle(lv_obj_t * obj, uint16_t angle);

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

#endif /* BT_WATCH_SRC_WIDGETS_DISCRETE_POINTER_H_ */
