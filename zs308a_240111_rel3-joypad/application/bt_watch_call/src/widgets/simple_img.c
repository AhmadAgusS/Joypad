/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file simple_img.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <sys/byteorder.h>
#include <src/misc/lv_style.h>
#include "simple_img.h"
#include "draw_util.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &simple_img_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void simple_img_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void simple_img_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void draw_img(lv_event_t * e);

static void invalidate_image_area(lv_obj_t * img);
static void refresh_image_area(lv_obj_t * img);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t simple_img_class = {
	.constructor_cb = simple_img_constructor,
	.event_cb = simple_img_event,
	.width_def = LV_SIZE_CONTENT,
	.height_def = LV_SIZE_CONTENT,
	.instance_size = sizeof(simple_img_t),
	.base_class = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * simple_img_create(lv_obj_t * parent)
{
	LV_LOG_INFO("begin");
	lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
	lv_obj_class_init_obj(obj);
	return obj;
}

/*=====================
 * Setter functions
 *====================*/

void simple_img_set_src(lv_obj_t *obj, const lv_img_dsc_t *src)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	lv_obj_invalidate(obj);

	img->src = src;
	if (img->src) {
		img->pivot.x = img->src->header.w / 2;
		img->pivot.y = img->src->header.h / 2;

		refresh_image_area(obj);
		lv_obj_refresh_self_size(obj);
	}
}

void simple_img_set_pivot(lv_obj_t * obj, lv_coord_t pivot_x, lv_coord_t pivot_y)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	if (img->pivot.x != pivot_x || img->pivot.y != pivot_y) {
		img->pivot.x = pivot_x;
		img->pivot.y = pivot_y;
		if (img->src && (img->angle > 0 || img->zoom != LV_IMG_ZOOM_NONE)) {
			refresh_image_area(obj);
		}
	}
}

void simple_img_set_angle(lv_obj_t *obj, uint16_t angle)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	while (angle >= 3600) angle -= 3600;
	while (angle < 0) angle += 3600;

	if (angle != img->angle) {
		img->angle = angle;
		if (img->src) {
			refresh_image_area(obj);
		}
	}
}

void simple_img_set_zoom(lv_obj_t * obj, uint16_t zoom)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	if (zoom == 0) zoom = 1;

	if (zoom != img->zoom) {
		img->zoom = zoom;
		if (img->src) {
			refresh_image_area(obj);
		}
	}
}

const lv_img_dsc_t * simple_img_get_src(lv_obj_t * obj)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	return img->src;
}

void simple_img_get_pivot(lv_obj_t * obj, lv_point_t * pivot)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	*pivot = img->pivot;
}

uint16_t simple_img_get_angle(lv_obj_t * obj)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	simple_img_t * img = (simple_img_t *)obj;

	return img->angle;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void invalidate_image_area(lv_obj_t * obj)
{
	simple_img_t * img = (simple_img_t *)obj;
	lv_coord_t ext_inv = (img->zoom != LV_IMG_ZOOM_NONE) ? 1 : 0;
	lv_area_t abs_area;

	abs_area.x1 = img->area.x1 + obj->coords.x1 - ext_inv;
	abs_area.y1 = img->area.y1 + obj->coords.y1 - ext_inv;
	abs_area.x2 = img->area.x2 + obj->coords.x1 + ext_inv;
	abs_area.y2 = img->area.y2 + obj->coords.y1 + ext_inv;

	lv_obj_invalidate_area(obj, &abs_area);
}

static void refresh_image_area(lv_obj_t * obj)
{
	simple_img_t * img = (simple_img_t *)obj;

	invalidate_image_area(obj);

	img->area.x1 = 0;
	img->area.y1 = 0;
	img->area.x2 = img->src->header.w - 1;
	img->area.y2 = img->src->header.h - 1;

	switch (img->src->header.cf) {
	case LV_IMG_CF_ARGB_6666:
	case LV_IMG_CF_ARGB_8888:
	case LV_IMG_CF_TRUE_COLOR:
	case LV_IMG_CF_TRUE_COLOR_ALPHA:
		break;
	default:
		if (img->angle > 0) {
			LV_LOG_ERROR("simple_img cannot rotate cf %d\n", img->src->header.cf);
			img->angle = 0;
		}
		break;
	}

	if (img->angle > 0 || img->zoom != LV_IMG_ZOOM_NONE) {
		int16_t x1, y1, x2, y2;
		uint16_t zoom_recp = LV_IMG_ZOOM_NONE * LV_IMG_ZOOM_NONE / img->zoom;

		/* zoom is to .8 fixedpoint */
		sw_transform_area16(&x1, &y1, &x2, &y2,
				img->area.x1, img->area.y1, img->area.x2, img->area.y2,
				img->pivot.x, img->pivot.y, img->angle, img->zoom, img->zoom, 8);

		/* in case that LV_USE_LARGE_COORD set */
		img->area.x1 = x1;
		img->area.y1 = y1;
		img->area.x2 = x2;
		img->area.y2 = y2;

		sw_transform_config(0, 0, img->pivot.x, img->pivot.y,
				img->angle, zoom_recp, zoom_recp, 8, &img->matrix);

		lv_obj_refresh_ext_draw_size(obj);
	}

	invalidate_image_area(obj);
}

static void simple_img_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
	simple_img_t * img = (simple_img_t *)obj;

	LV_UNUSED(class_p);
	LV_TRACE_OBJ_CREATE("begin");

	lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(obj, LV_OBJ_FLAG_ADV_HITTEST);

	img->zoom = LV_IMG_ZOOM_NONE;

	LV_TRACE_OBJ_CREATE("finished");
}

static void simple_img_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
	LV_UNUSED(class_p);

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	simple_img_t * img = (simple_img_t *)obj;

	/*Ancestor events will be called during drawing*/
	if (code != LV_EVENT_COVER_CHECK && code != LV_EVENT_DRAW_MAIN && code != LV_EVENT_DRAW_POST) {
		/*Call the ancestor's event handler*/
		lv_res_t res = lv_obj_event_base(MY_CLASS, e);
		if(res != LV_RES_OK) return;
	}

	if (code == LV_EVENT_STYLE_CHANGED) {
		if (img->angle > 0 || img->zoom != LV_IMG_ZOOM_NONE) {
			/*With transformation it might change*/
			lv_obj_refresh_ext_draw_size(obj);
		}
	} else if (code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
		if (img->angle > 0 || img->zoom != LV_IMG_ZOOM_NONE) {
			lv_coord_t * s = lv_event_get_param(e);
			lv_coord_t pad_ori = *s;
			lv_coord_t w = lv_obj_get_width(obj);
			lv_coord_t h = lv_obj_get_height(obj);

			*s = LV_MAX(*s, pad_ori - img->area.x1);
			*s = LV_MAX(*s, pad_ori - img->area.y1);
			*s = LV_MAX(*s, pad_ori + img->area.x2 - w + 1);
			*s = LV_MAX(*s, pad_ori + img->area.y2 - h + 1);
		}
	} else if (code == LV_EVENT_HIT_TEST) {
		lv_hit_test_info_t * info = lv_event_get_param(e);
		lv_area_t coords;
		coords.x1 = img->area.x1 + obj->coords.x1;
		coords.y1 = img->area.y1 + obj->coords.y1;
		coords.x2 = img->area.x2 + obj->coords.x1;
		coords.y2 = img->area.y2 + obj->coords.y1;

		info->res = _lv_area_is_point_on(&coords, info->point, 0);
	} else if (code == LV_EVENT_GET_SELF_SIZE) {
		lv_point_t * p = lv_event_get_param(e);
		if (img->src) {
			p->x = img->src->header.w;
			p->y = img->src->header.h;
		}
	} else if (code == LV_EVENT_DRAW_MAIN || code == LV_EVENT_DRAW_POST || code == LV_EVENT_COVER_CHECK) {
		draw_img(e);
	}
}

static void draw_img(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	simple_img_t * img = (simple_img_t *)obj;
	lv_area_t abs_area;

	lv_area_set(&abs_area,
		img->area.x1 + obj->coords.x1, img->area.y1 + obj->coords.y1,
		img->area.x2 + obj->coords.x1, img->area.y2 + obj->coords.y1);

	if (code == LV_EVENT_COVER_CHECK) {
		lv_cover_check_info_t * info = lv_event_get_param(e);
		if (info->res == LV_COVER_RES_MASKED) return;

		if (img->angle != 0 || img->src == NULL) {
			info->res = LV_COVER_RES_NOT_COVER;
			return;
		}

		if (img->src->header.cf != LV_IMG_CF_TRUE_COLOR && img->src->header.cf != LV_IMG_CF_RAW) {
			info->res = LV_COVER_RES_NOT_COVER;
			return;
		}

		/*With not LV_OPA_COVER images can't cover an area */
		if (lv_obj_get_style_img_opa(obj, LV_PART_MAIN) != LV_OPA_COVER) {
			info->res = LV_COVER_RES_NOT_COVER;
			return;
		}

		if (_lv_area_is_in(info->area, &abs_area, 0) == false) {
			info->res = LV_COVER_RES_NOT_COVER;
			return;
		}

		info->res = LV_COVER_RES_COVER;
	} else if (code == LV_EVENT_DRAW_MAIN) {
		const lv_area_t * clip_area = lv_event_get_param(e);
		lv_area_t draw_area;

		if (img->src == NULL || _lv_area_intersect(&draw_area, &abs_area, clip_area) == false)
			return;

		if (img->angle == 0 && img->zoom == LV_IMG_ZOOM_NONE) {
			lv_draw_img_dsc_t img_dsc;
			lv_draw_img_dsc_init(&img_dsc);
			//lv_obj_init_draw_img_dsc(obj, LV_IMG_PART_MAIN, &img_dsc);
			img_dsc.recolor = lv_obj_get_style_img_recolor(obj, LV_PART_MAIN);
			img_dsc.recolor_opa = lv_obj_get_style_img_recolor_opa(obj, LV_PART_MAIN);
			img_dsc.opa = lv_obj_get_style_img_opa(obj, LV_PART_MAIN);

			lvgl_draw_img_dsc(&abs_area, &draw_area, img->src, &img_dsc);
			return;
		}

		lv_disp_t * disp = _lv_refr_get_disp_refreshing();
		lv_disp_draw_buf_t * draw_buf = lv_disp_get_draw_buf(disp);
		const lv_area_t * disp_area = &draw_buf->area;

		lv_color_t * disp_buf = (lv_color_t *)draw_buf->buf_act +
				(draw_area.y1 - disp_area->y1) * lv_area_get_width(disp_area) +
				(draw_area.x1 - disp_area->x1);

		void (*sw_transform_fn)(void *, const void *, uint16_t, uint16_t, uint16_t,
			int16_t, int16_t, uint16_t, uint16_t, const sw_matrix_t *);

	#if LV_COLOR_DEPTH == 16
		if (img->src->header.cf == LV_IMG_CF_TRUE_COLOR) {
			sw_transform_fn = sw_transform_rgb565_over_rgb565;
		} else if (img->src->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
			sw_transform_fn = sw_transform_argb8565_over_rgb565;
		} else if (img->src->header.cf == LV_IMG_CF_ARGB_6666) {
			sw_transform_fn = sw_transform_argb6666_over_rgb565;
		} else {
			sw_transform_fn = sw_transform_argb8888_over_rgb565;
		}
	#else
		if (img->src->header.cf == LV_IMG_CF_ARGB_6666) {
			sw_transform_fn = sw_transform_argb6666_over_argb8888;
		} else {
			sw_transform_fn = sw_transform_argb8888_over_argb8888;
		}
	#endif

		if (disp->driver->gpu_wait_cb) {
			disp->driver->gpu_wait_cb(disp->driver);
		}

		sw_transform_fn(disp_buf, img->src->data, lv_area_get_width(disp_area),
				img->src->header.w, img->src->header.h,
				draw_area.x1 - obj->coords.x1, draw_area.y1 - obj->coords.y1,
				lv_area_get_width(&draw_area), lv_area_get_height(&draw_area),
				&img->matrix);
	}
}
