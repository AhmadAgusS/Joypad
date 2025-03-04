/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file progress_arc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <assert.h>
#include <sys/util.h>
#include "progress_arc.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &progress_arc_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void progress_arc_event(const lv_obj_class_t * class_p, lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t progress_arc_class = {
	.event_cb = progress_arc_event,
	.base_class = &lv_arc_class,
};

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * progress_arc_create(lv_obj_t * parent)
{
	LV_LOG_INFO("begin");
	lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
	lv_obj_class_init_obj(obj);
	return obj;
}

void progress_arc_clean(lv_obj_t * obj)
{
	return;
}

/*=====================
 * Setter functions
 *====================*/

 /**********************
  *   STATIC FUNCTIONS
  **********************/

 static void progress_arc_event(const lv_obj_class_t * class_p, lv_event_t * e)
 {
	 LV_UNUSED(class_p);
 
	 /*Call the ancestor's event handler*/
	 lv_event_code_t code = lv_event_get_code(e);
	 if(code == LV_EVENT_PRESSING || code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
	 {
	 	return;
	 }
	 
	 lv_res_t res = lv_obj_event_base(MY_CLASS, e);
	 if(res != LV_RES_OK) return;
 }


