/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file simple_img.h
 *
 */

#ifndef BT_WATCH_SRC_WIDGETS_SIMPLE_IMG_H_
#define BT_WATCH_SRC_WIDGETS_SIMPLE_IMG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include <display/sw_rotate.h>


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of rotate image */
typedef struct {
	lv_obj_t obj;

	const lv_img_dsc_t *src; /* image source */
	lv_area_t  area;   /* image draw area (relative to the top-left of the object) */
	lv_point_t pivot;  /* Rotation center of the image, relative to the top-left corner of the image */
	uint16_t angle;    /* Rotation angle in 0.1 degree [0, 3600) of the image */
	uint16_t zoom;

	/* immtermediate variables to accelerate rotation computing */
	sw_matrix_t matrix;
} simple_img_t;

extern const lv_obj_class_t simple_img_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an simple image objects
 * @param parent pointer to an object, it will be the parent of the new image object
 * @return pointer to the created image object
 */
lv_obj_t * simple_img_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the pixel map to display by the image
 * @param obj pointer to an image object
 * @param src pointer to an image source (a C array or path to a file)
 */
void simple_img_set_src(lv_obj_t *obj, const lv_img_dsc_t * src);

/**
 * Set the rotation center of the image.
 * The image will be rotated around this point
 * @param obj pointer to an image object
 * @param pivot_x rotation/zoom center x of the image
 * @param pivot_y rotation/zoom center y of the image
 */
void simple_img_set_pivot(lv_obj_t * obj, lv_coord_t pivot_x, lv_coord_t pivot_y);

/**
 * Set the rotation angle of the image.
 * The image will be rotated around the set pivot set by `simple_img_set_pivot()`
 * @param obj pointer to an image object
 * @param angle rotation angle in degree with 0.1 degree resolution ([0, 3600): clock wise)
 */
void simple_img_set_angle(lv_obj_t * obj, uint16_t angle);

/**
 * Set the zoom factor of the image.
 * @param obj       pointer to an image object
 * @param zoom      the zoom factor.
 * @example 256 or LV_ZOOM_IMG_NONE for no zoom
 * @example <256: scale down
 * @example >256 scale up
 * @example 128 half size
 * @example 512 double size
 */
void simple_img_set_zoom(lv_obj_t * obj, uint16_t zoom);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the source of the image
 * @param obj pointer to an image object
 * @return the image source (symbol, file name or C array)
 */
const lv_img_dsc_t * simple_img_get_src(lv_obj_t * obj);

/**
 * Get the rotation center of the image.
 * @param obj pointer to an image object
 * @param pivot rotation center of the image
 */
void simple_img_get_pivot(lv_obj_t * obj, lv_point_t * pivot);

/**
 * Get the rotation angle of the image.
 * @param obj pointer to an image object
 * @return rotation angle in 0.1 degree [0, 3600)
 */
uint16_t simple_img_get_angle(lv_obj_t * obj);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

/*Use this macro to declare an image in a c file*/
#ifndef LV_IMG_DECLARE
#define LV_IMG_DECLARE(var_name) extern const lv_img_dsc_t var_name;
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*BT_WATCH_SRC_WIDGETS_SIMPLE_IMG_H_*/
