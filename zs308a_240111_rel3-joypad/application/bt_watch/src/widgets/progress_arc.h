/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file progress_arc.h
 *
 */

#ifndef BT_WATCH_SRC_WIDGETS_PROGRESS_ARC_H_
#define BT_WATCH_SRC_WIDGETS_PROGRESS_ARC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include <lvgl/lvgl_img_loader.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of anim image */
typedef struct {
	lv_arc_t arc;
} progress_arc_t;

extern const lv_obj_class_t progress_arc_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an animated image object
 * @param parent pointer to an object, it will be the parent of the new object
 * @return pointer to the created animated image object
 */
lv_obj_t * progress_arc_create(lv_obj_t * parent);

/**
 * Clean the image frame resource
 *
 * @param obj pointer to an image animation object
 */
void progress_arc_clean(lv_obj_t * obj);

/*=====================
 * Setter functions
 *====================*/


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
 
#endif /* BT_WATCH_SRC_WIDGETS_IMG_ANIM_H_ */

