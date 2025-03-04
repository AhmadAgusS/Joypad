/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file discrete_pointer.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <assert.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <display/sw_draw.h>
#include "discrete_pointer.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &discrete_pointer_class

/**********************
 *      TYPEDEFS
 **********************/
typedef enum quadrant {
	QUADRANT_0 = 0, /* first quadrant top half area */
	QUADRANT_1,     /* first quadrant bottom half area */

	QUADRANT_2, /* fourth quadrant top half area */
	QUADRANT_3, /* fourth quadrant bottom half area */

	QUADRANT_4, /* third quadrant bottom half area */
	QUADRANT_5, /* third quadrant top half area */

	QUADRANT_6, /* second quadrant bottom half area */
	QUADRANT_7, /* second quadrant top half area */
} quadrant_t;

typedef void (*get_quadrant_new_point_t)(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

typedef void (*get_quadrant_old_point_delta_t)(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void discrete_pointer_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void discrete_pointer_event(const lv_obj_class_t * class_p, lv_event_t * e);

static quadrant_t _pointer_get_quadrant(uint16_t angle, uint8_t num_images, uint8_t *image_idx);
static bool _pointer_get_new_image_size(quadrant_t type,
		int16_t old_w, int16_t old_h, int16_t *new_w, int16_t *new_h);

static void _pointer_get_new_point_quadrant_0(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_0(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_1(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_1(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_2(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_2(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_3(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_3(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_7(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_7(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_6(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_6(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_5(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_5(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

static void _pointer_get_new_point_quadrant_4(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt);

static void _pointer_get_old_point_quadrant_4(
		int16_t old_w, int16_t old_h,
		lv_point_t *new_pt, lv_point_t *old_pt,
		lv_point_t *old_dx, lv_point_t *old_dy);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t discrete_pointer_class = {
	.constructor_cb = discrete_pointer_constructor,
	.event_cb = discrete_pointer_event,
	.width_def = LV_SIZE_CONTENT,
	.height_def = LV_SIZE_CONTENT,
	.instance_size = sizeof(discrete_pointer_t),
	.base_class = &lv_obj_class
};

static const get_quadrant_new_point_t _pointer_get_new_point_fn_tbl[] = {
	_pointer_get_new_point_quadrant_0,
	_pointer_get_new_point_quadrant_1,
	_pointer_get_new_point_quadrant_2,
	_pointer_get_new_point_quadrant_3,
	_pointer_get_new_point_quadrant_4,
	_pointer_get_new_point_quadrant_5,
	_pointer_get_new_point_quadrant_6,
	_pointer_get_new_point_quadrant_7,
};

static const get_quadrant_old_point_delta_t _pointer_get_old_point_fn_tbl[] = {
	_pointer_get_old_point_quadrant_0,
	_pointer_get_old_point_quadrant_1,
	_pointer_get_old_point_quadrant_2,
	_pointer_get_old_point_quadrant_3,
	_pointer_get_old_point_quadrant_4,
	_pointer_get_old_point_quadrant_5,
	_pointer_get_old_point_quadrant_6,
	_pointer_get_old_point_quadrant_7,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * discrete_pointer_create(lv_obj_t * parent)
{
	LV_LOG_INFO("begin");
	lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
	lv_obj_class_init_obj(obj);
	return obj;
}

/*=====================
 * Setter functions
 *====================*/

void discrete_pointer_set_src(lv_obj_t *obj, const lv_img_dsc_t *srcs, uint8_t cnt)
{
	discrete_pointer_t * pointer = (discrete_pointer_t *)obj;

	pointer->num_srcs = cnt;
	pointer->srcs = srcs;
}

void discrete_pointer_set_pivot(lv_obj_t *obj, lv_coord_t pivot_x,
		lv_coord_t pivot_y, lv_coord_t pos_x, lv_coord_t pos_y)
{
	discrete_pointer_t * pointer = (discrete_pointer_t *)obj;

	pointer->pivot.x = pivot_x;
	pointer->pivot.y = pivot_y;
	pointer->pos.x = pos_x;
	pointer->pos.y = pos_y;
}

void discrete_pointer_set_angle(lv_obj_t *obj, uint16_t angle)
{
	discrete_pointer_t * pointer = (discrete_pointer_t *)obj;
	uint8_t src_idx = 0;

	if (angle >= 360 || angle == pointer->angle)
		return;

	pointer->angle = angle;
	pointer->quadrant = _pointer_get_quadrant(angle, pointer->num_srcs, &src_idx);
	pointer->cur_src = &pointer->srcs[src_idx];

	lv_point_t old_pt, new_pt;
	int16_t new_w, new_h;

	_pointer_get_new_image_size(pointer->quadrant, pointer->cur_src->header.w,
			pointer->cur_src->header.h, &new_w, &new_h);

	old_pt.x = pointer->pivot.x;
	old_pt.y = pointer->cur_src->header.h - 1 - pointer->pivot.y;

	_pointer_get_new_point_fn_tbl[pointer->quadrant](
			pointer->cur_src->header.w, pointer->cur_src->header.h, &new_pt, &old_pt);

	LV_LOG_INFO("angle %d, quadrant %d, src_idx %d\n", angle, pointer->quadrant, src_idx);

	switch (pointer->quadrant) {
	case QUADRANT_1:
		new_pt.x += 1;
		new_pt.y += 1;
		break;
	case QUADRANT_2:
		new_pt.x += 1;
		new_pt.y += 0;
		break;
	case QUADRANT_3:
		new_pt.y += 1;
		break;
	case QUADRANT_4:
		new_pt.x += 1;
		new_pt.y += 1;
		break;
	case QUADRANT_5:
		new_pt.x += 0;
		new_pt.y += 0;
		break;
	case QUADRANT_6:
		new_pt.x += 0;
		new_pt.y += 1;
		break;
	case QUADRANT_7:
		new_pt.x += 1;
		break;
	case QUADRANT_0:
	default:
		break;
	}

	lv_obj_set_pos(obj, pointer->pos.x - new_pt.x, pointer->pos.y - new_pt.y);
	lv_obj_set_size(obj, new_w, new_h);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void discrete_pointer_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
	LV_UNUSED(class_p);
	LV_TRACE_OBJ_CREATE("begin");

	lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);

	LV_TRACE_OBJ_CREATE("finished");
}

static void discrete_pointer_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
	LV_UNUSED(class_p);

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	discrete_pointer_t * pointer = (discrete_pointer_t *)obj;

	/*Ancestor events will be called during drawing*/
	if (code != LV_EVENT_DRAW_MAIN && code != LV_EVENT_DRAW_POST) {
		/*Call the ancestor's event handler*/
		lv_res_t res = lv_obj_event_base(MY_CLASS, e);
		if(res != LV_RES_OK) return;
	}

	if (code == LV_EVENT_GET_SELF_SIZE) {
		lv_point_t * p = lv_event_get_param(e);
		if (pointer->cur_src) {
			p->x = pointer->cur_src->header.w;
			p->y = pointer->cur_src->header.h;
		}
	} else if (code == LV_EVENT_COVER_CHECK) {
		lv_cover_check_info_t * info = lv_event_get_param(e);
		info->res = LV_COVER_RES_NOT_COVER;
	} else if (code == LV_EVENT_DRAW_MAIN) {
		if (pointer->cur_src == NULL) {
			return;
		}

		const lv_area_t * clip_area = lv_event_get_param(e);
		lv_area_t draw_area;

		if (_lv_area_intersect(&draw_area, &obj->coords, clip_area) == false) {
			return;
		}

		lv_disp_t * disp = _lv_refr_get_disp_refreshing();
		lv_disp_draw_buf_t * draw_buf = lv_disp_get_draw_buf(disp);
		const lv_area_t * disp_area = &draw_buf->area;
		lv_point_t old_pt, new_pt, pt_dx, pt_dy;

		if (disp->driver->gpu_wait_cb) {
			disp->driver->gpu_wait_cb(disp->driver);
		}

		new_pt.x = draw_area.x1 - obj->coords.x1;
		new_pt.y = draw_area.y1 - obj->coords.y1;
		_pointer_get_old_point_fn_tbl[pointer->quadrant](
				pointer->cur_src->header.w, pointer->cur_src->header.h,
				&new_pt, &old_pt, &pt_dx, &pt_dy);

		lv_color_t * dst = (lv_color_t *)draw_buf->buf_act +
				(draw_area.y1 - disp_area->y1) * lv_area_get_width(disp_area) +
				(draw_area.x1 - disp_area->x1);
		lv_coord_t dst_w = lv_area_get_width(disp_area);
		lv_coord_t draw_w = lv_area_get_width(disp_area);

#if LV_COLOR_DEPTH == 16
		if (pointer->cur_src->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
			/* dst rgb-565, src argb-8565 */
			uint8_t *src8 = (uint8_t *)pointer->cur_src->data + old_pt.y * pointer->cur_src->header.w * 3 + old_pt.x * 3;
			lv_coord_t src_x_step = pointer->cur_src->header.w * 3 * pt_dx.y + 3 * pt_dx.x;
			lv_coord_t src_y_step = pointer->cur_src->header.w * 3 * pt_dy.y + 3 * pt_dy.x;

			for (int j = lv_area_get_height(&draw_area); j > 0; j--) {
				lv_color_t *tmp_dst = dst;
				uint8_t *tmp_src8 = src8;

				for (int i = draw_w; i > 0; i--) {
					tmp_dst->full = blend_rgb565_over_rgb565(
							tmp_dst->full, sys_get_le16(tmp_src8), tmp_src8[2]);
					tmp_dst++;
					tmp_src8 += src_x_step;
				}

				dst += dst_w;
				src8 += src_y_step;
			}
		} else
#endif
		{
			/* src argb-8888 */
			uint8_t *src8 = (uint8_t *)pointer->cur_src->data + old_pt.y * pointer->cur_src->header.w * 4 + old_pt.x * 4;
			lv_coord_t src_x_step = pointer->cur_src->header.w * 4 * pt_dx.y + 4 * pt_dx.x;
			lv_coord_t src_y_step = pointer->cur_src->header.w * 4 * pt_dy.y + 4 * pt_dy.x;

			for (int j = lv_area_get_height(&draw_area); j > 0; j--) {
				lv_color_t *tmp_dst = dst;
				uint8_t *tmp_src8 = src8;

				for (int i = draw_w; i > 0; i--) {
#if LV_COLOR_DEPTH == 16
					tmp_dst->full = blend_argb8888_over_rgb565(
							tmp_dst->full, *(uint32_t *)tmp_src8);
#else
					tmp_dst->full = blend_argb8888_over_argb8888(
							tmp_dst->full, *(uint32_t *)tmp_src8);
#endif
					tmp_dst++;
					tmp_src8 += src_x_step;
				}

				dst += dst_w;
				src8 += src_y_step;
			}
		}
	}
}

static quadrant_t _pointer_get_quadrant(uint16_t angle, uint8_t num_images, uint8_t *image_idx)
{
	const uint16_t degree_per_image = 45 / (num_images - 1);
	quadrant_t quadrant;

	if (angle < 45) {
		quadrant = QUADRANT_0;
	} else if (angle < 90) {
		angle = 90 - angle;
		quadrant = QUADRANT_1;
	} else if (angle < 135) {
		angle = angle - 90;
		quadrant = QUADRANT_2;
	} else if (angle < 180) {
		angle = 180 - angle;
		quadrant = QUADRANT_3;
	} else if (angle < 225) {
		angle = angle - 180;
		quadrant = QUADRANT_4;
	} else if (angle < 270) {
		angle = 270 - angle;
		quadrant = QUADRANT_5;
	} else if (angle < 315) {
		angle = angle - 270;
		quadrant = QUADRANT_6;
	} else {
		angle = 360 - angle;
		quadrant = QUADRANT_7;
	}

	*image_idx = angle / degree_per_image;
	return quadrant;
}

static bool _pointer_get_new_image_size(quadrant_t type,
		int16_t old_w, int16_t old_h, int16_t *new_w, int16_t *new_h)
{
	switch (type) {
	case QUADRANT_0:
	case QUADRANT_7:
	case QUADRANT_4:
	case QUADRANT_3:
		*new_w = old_w;
		*new_h = old_h;
	break;
	case QUADRANT_1:
	case QUADRANT_6:
	case QUADRANT_5:
	case QUADRANT_2:
		*new_w = old_h;
		*new_h = old_w;
		break;
	default:
		return false;
	}

	return true;
}

static void _pointer_get_new_point_quadrant_0(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_pt->x;
	new_pt->y = old_pt->y;
}

static void _pointer_get_old_point_quadrant_0(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = new_pt->x;
	old_pt->y = new_pt->y;

	old_dx->x = 1;
	old_dx->y = 0;
	old_dy->x = 0;
	old_dy->y= 1;
}

static void _pointer_get_new_point_quadrant_1(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_h - 1 - old_pt->y;
#if 0
	new_pt->y = old_pt->x;
	new_pt->y = gui_abs16(old_w - 1 - new_pt->y);
#else
	new_pt->y = old_w - 1 - old_pt->x;
#endif
}

static void _pointer_get_old_point_quadrant_1(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = old_w - 1 - new_pt->y;
	old_pt->y = old_h - 1 - new_pt->x;

	old_dx->x = 0;
	old_dx->y = -1;
	old_dy->x = -1;
	old_dy->y= 0;
}

static void _pointer_get_new_point_quadrant_2(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_h - 1 - old_pt->y;
#if 0
	new_pt->y = old_pt->x;
	new_pt->y = gui_abs16(old_w - 1 - new_pt->y);
	new_pt->y = old_w - new_pt->y - 1;
#else
	new_pt->y = old_pt->x;
#endif
}

static void _pointer_get_old_point_quadrant_2(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = new_pt->y;
	old_pt->y = old_h - 1 - new_pt->x;

	old_dx->x = 0;
	old_dx->y = -1;
	old_dy->x = 1;
	old_dy->y= 0;
}

static void _pointer_get_new_point_quadrant_3(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_pt->x;
	new_pt->y = old_h - 1 - old_pt->y;
}

static void _pointer_get_old_point_quadrant_3(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = new_pt->x;
	old_pt->y = old_h - 1 - new_pt->y;

	old_dx->x = 1;
	old_dx->y = 0;
	old_dy->x = 0;
	old_dy->y= -1;
}

static void _pointer_get_new_point_quadrant_7(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_w - 1 - old_pt->x;
	new_pt->y = old_pt->y;
}

static void _pointer_get_old_point_quadrant_7(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = old_w - 1 - new_pt->x;
	old_pt->y = new_pt->y;

	old_dx->x = -1;
	old_dx->y = 0;
	old_dy->x = 0;
	old_dy->y= 1;
}

static void _pointer_get_new_point_quadrant_6(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
#if 0
	new_pt->x = old_h - 1 - old_pt->y;
	new_pt->y = old_pt->x;
	new_pt->x = gui_abs16(old_h - 1 - new_pt->x);
	new_pt->y = gui_abs16(old_w - 1 - new_pt->y);
#else
	new_pt->x = old_pt->y;
	new_pt->y = old_w - 1 - old_pt->x;
#endif
}

static void _pointer_get_old_point_quadrant_6(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = old_w - 1 - new_pt->y;
	old_pt->y = old_h - old_h + new_pt->x;

	old_dx->x = 0;
	old_dx->y = 1;
	old_dy->x = -1;
	old_dy->y= 0;
}

static void _pointer_get_new_point_quadrant_5(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_pt->y;
	new_pt->y = old_pt->x;
}

static void _pointer_get_old_point_quadrant_5(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = new_pt->y;
	old_pt->y = new_pt->x;

	old_dx->x = 0;
	old_dx->y = 1;
	old_dy->x = 1;
	old_dy->y= 0;
}

static void _pointer_get_new_point_quadrant_4(
      int16_t old_w, int16_t old_h,
      lv_point_t *new_pt, lv_point_t *old_pt)
{
	new_pt->x = old_w - 1 - old_pt->x;
	new_pt->y = old_h - 1 - old_pt->y;
}

static void _pointer_get_old_point_quadrant_4(
      int16_t old_w, int16_t old_h,
	  lv_point_t *new_pt, lv_point_t *old_pt,
	  lv_point_t *old_dx, lv_point_t *old_dy)
{
	old_pt->x = old_w - 1 - new_pt->x;
	old_pt->y = old_h - 1 - new_pt->y;

	old_dx->x = -1;
	old_dx->y = 0;
	old_dy->x = 0;
	old_dy->y= -1;
}
