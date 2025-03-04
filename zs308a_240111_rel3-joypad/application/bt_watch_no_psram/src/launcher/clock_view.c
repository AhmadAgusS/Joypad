/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <assert.h>
#include <lvgl.h>
#include <lvgl/lvgl_bitmap_font.h>
#include <lvgl/lvgl_res_loader.h>
#include <ui_manager.h>
#include <view_manager.h>
#include "widgets/watch_clock.h"
#include "widgets/simple_img.h"
#include "widgets/anim_img.h"
#include "widgets/img_number.h"
#include "clock_view.h"
#include "sdfs.h"
//LOG_MODULE_DECLARE(clock);

#define CLOCK_SWITCH_PRESSING_TIME 1000 /* ms */
extern const clock_view_presenter_t clock_view_presenter;
enum {
	INFO_STEPCOUNT = 0,
	INFO_HEARTRATE,
	INFO_CALORIES,
	INFO_DISTANCE,
	INFO_SLEEPTIME,

	INFO_BATTERY,

	INFO_TEMPERATURE,
	INFO_WEATHER,

	INFO_ANIM,
	INFO_DATE,
	INFO_LUNAR,

	NUM_INFO,
};

typedef struct clock_view_tmp_res {
	lvgl_res_group_t grp_hour;
	lvgl_res_group_t grp_minute;
	lvgl_res_group_t grp_second;
	lvgl_res_group_t grp_stepcount;
	lvgl_res_group_t grp_heartrate;
	lvgl_res_group_t grp_calories;
	lvgl_res_group_t grp_distance;
	lvgl_res_group_t grp_sleeptime;
	lvgl_res_group_t grp_battery;
	lvgl_res_group_t grp_temperature;

	lv_point_t pt_colon;
} clock_view_tmp_res_t;

typedef struct clock_view_data {
	bool is_digital;

	/* previous value */
	uint8_t weather;
	uint8_t tm_mday;
	uint8_t tm_mon;
	uint16_t tm_year;
	uint32_t distance;
	uint32_t sleep_time;

	lv_obj_t *obj_clock;
	lv_obj_t *obj_info[NUM_INFO];

	lv_font_t font;
	lv_style_t sty_text[2];

	/* lvgl resource */
	lv_img_dsc_t img_bg;
	lv_img_dsc_t img_tm_colon; /* : between hour and minute */
	lv_img_dsc_t *img_tm_hour; /* 0~9 */
	lv_img_dsc_t *img_tm_min;  /* 0~9 */
	lv_img_dsc_t *img_tm_sec;  /* 0~9 */

	lvgl_res_string_t res_str[2];
	lvgl_res_scene_t scene;

	/* click event */
	uint32_t pressing_start;
	bool preview_invoked;
} clock_view_data_t;

view_data_t globle_view_data;

static int _clock_view_paint(view_data_t *view_data, bool forced);

static const uint32_t preload_ids[] = {
	PIC_BACKGROUND, PIC_TM_COLON, RES_TM_HOUR, RES_TM_MINUTE, RES_TM_SECOND,
	RES_STEP_COUNT, RES_HEART_RATE, RES_CALORIES, RES_BATTERY, RES_TEMPERATURE,
	RES_DISTANCE, RES_SLEEP_TIME,
};

static const uint32_t pic_common_ids[] = {
	PIC_0, PIC_1, PIC_2, PIC_3, PIC_4, PIC_5, PIC_6, PIC_7, PIC_8, PIC_9,
};

static uint8_t clock_preload_inited = 0;


static int _load_pictures(lvgl_res_scene_t *scene, uint32_t group_id,
		lvgl_res_group_t *group, const uint32_t *pic_ids,
		int num_ids, lv_img_dsc_t **images, bool first_layout)
{
	if (first_layout) {
		*images = app_mem_malloc(num_ids * sizeof(lv_img_dsc_t));
		if (*images == NULL) {
			return -ENOMEM;
		}
	}

	if (lvgl_res_load_group_from_scene(scene, group_id, group)) {
		goto fail_free_image;
	}

	if (lvgl_res_load_pictures_from_group(group, pic_ids, *images, NULL, num_ids)) {
		lvgl_res_unload_group(group);
		goto fail_free_image;
	}

	lvgl_res_unload_group(group);
	return 0;
fail_free_image:
	if (first_layout) {
		app_mem_free(*images);
		*images = NULL;
	}

	return -ENOENT;
}

static void _unload_pictures(lv_img_dsc_t **images, int num_images, bool deleting)
{
	if (*images) {
		lvgl_res_unload_pictures(*images, num_images);
		if (deleting) {
			app_mem_free(*images);
			*images = NULL;
		}
	}
}

static int _clock_view_load_resource(clock_view_data_t *data, clock_view_tmp_res_t *tmp_res, bool first_layout)
{
	uint32_t pic_id;
	int ret;

	/* scene */
	ret = lvgl_res_load_scene(SCENE_CLOCK_VIEW, &data->scene,
			DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
	if (ret < 0) {
		SYS_LOG_ERR("scene 0x%x not found", SCENE_CLOCK_VIEW);
		return -ENOENT;
	}

	if (first_layout) {
		//test scene is digital or not
		ret = lvgl_res_load_group_from_scene(&data->scene, RES_DIGITAL_ID, &tmp_res->grp_hour);
		if (ret == 0) {
			data->is_digital = true;
			lvgl_res_unload_group(&tmp_res->grp_hour);
		}
	}

	/* hour */
	ret = _load_pictures(&data->scene, RES_TM_HOUR, &tmp_res->grp_hour,
			pic_common_ids, data->is_digital ? 10 : 1, &data->img_tm_hour, first_layout);
	if (ret < 0) {
		return -ENOENT;
	}

	/* minute */
	ret = _load_pictures(&data->scene, RES_TM_MINUTE, &tmp_res->grp_minute,
			pic_common_ids, data->is_digital ? 10 : 1, &data->img_tm_min, first_layout);
	if (ret < 0) {
		return -ENOENT;
	}

	/* second */
	ret = _load_pictures(&data->scene, RES_TM_SECOND, &tmp_res->grp_second,
			pic_common_ids, data->is_digital ? 10 : 1, &data->img_tm_sec, first_layout);

	/* time colon */
	pic_id = PIC_TM_COLON;
	lvgl_res_load_pictures_from_scene(&data->scene, &pic_id,
			&data->img_tm_colon, &tmp_res->pt_colon, 1);

	if (first_layout) {
		if (data->res_str[0].width > 0) {
			ret = lvgl_bitmap_font_open(&data->font, DEF_FONT22_FILE);
			if (ret < 0) {
				return -ENOENT;
			}
		}
	}

	return 0;
}

static void _clock_view_unload_resource(clock_view_data_t *data, bool deleting)
{
	lvgl_res_preload_cancel_scene(SCENE_CLOCK_VIEW);
	lvgl_res_unload_scene_compact(SCENE_CLOCK_VIEW);

	if (data) {
		if (deleting) {
			if (data->res_str[0].width > 0) {
				lvgl_bitmap_font_close(&data->font);
			}
		}

		_unload_pictures(&data->img_tm_hour, data->is_digital ? 10 : 1, deleting);
		_unload_pictures(&data->img_tm_min, data->is_digital ? 10 : 1, deleting);
		_unload_pictures(&data->img_tm_sec, data->is_digital ? 10 : 1, deleting);

		//lvgl_res_unload_pictures(&data->img_bg, 1);
		lvgl_res_unload_strings(data->res_str, ARRAY_SIZE(data->res_str));
		lvgl_res_unload_scene(&data->scene);
	}
}

static int _clock_view_preload(view_data_t *view_data, bool update)
{
	//uint32_t regular_id[2] = {PIC_BACKGROUND, PIC_TM_COLON};

	if (clock_preload_inited == 0) {
		lvgl_res_preload_scene_compact_default_init(SCENE_CLOCK_VIEW, preload_ids, ARRAY_SIZE(preload_ids),
			NULL, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
		lvgl_res_preload_scene_compact_default_init(SCENE_DIGITAL_CLOCK_VIEW, preload_ids, ARRAY_SIZE(preload_ids),
			NULL, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
#ifdef SCENE_AOD_CLOCK_VIEW
		lvgl_res_preload_scene_compact_default_init(SCENE_AOD_CLOCK_VIEW, preload_ids, ARRAY_SIZE(preload_ids),
			NULL, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);
#endif
		clock_preload_inited = 1;
	}

	//lvgl_res_set_pictures_regular(SCENE_CLOCK_VIEW, 0, 0, regular_id, 2, DEF_STY_FILE, DEF_RES_FILE, DEF_STR_FILE);

	if (lvgl_res_preload_scene_compact_default(SCENE_CLOCK_VIEW, CLOCK_VIEW, update, 0)) {
		return -ENOMEM;
	}

	return 0;
}

static void _clock_event_handler(lv_event_t * e)
{
	lv_event_code_t event = lv_event_get_code(e);
	view_data_t *view_data = lv_event_get_user_data(e);
	clock_view_data_t *data = view_data->user_data;
	const clock_view_presenter_t *presenter = &clock_view_presenter;
	uint32_t pressing_duration = 0;

	if (data->preview_invoked)
		return;

	if (event == LV_EVENT_PRESSED) {
		data->pressing_start = os_uptime_get_32();
	} else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESSING) {
		pressing_duration = os_uptime_get_32() - data->pressing_start;
	}

	if (pressing_duration > CLOCK_SWITCH_PRESSING_TIME) {
		if (presenter->invoke_preview) {
			data->preview_invoked = presenter->invoke_preview() ? false : true;
		}
	}
}

static int _clock_view_layout_update(view_data_t *view_data, bool first_layout)
{
	lv_obj_t *scr = lv_scr_act();

	clock_view_data_t *data = view_data->user_data;
	clock_view_tmp_res_t *resource;

	if (first_layout) {
		data = app_mem_malloc(sizeof(*data));
		if (data == NULL) {
			return -ENOMEM;
		}
		memset(data, 0, sizeof(*data));
		view_data->user_data = data;
	}

	resource = app_mem_malloc(sizeof(*resource));
	if (!resource) {
		SYS_LOG_ERR("clock_view malloc tmp resource failed\n");
		if (first_layout) app_mem_free(data);
		view_data->user_data = NULL;
		return -ENOMEM;
	}

	if (_clock_view_load_resource(data, resource, first_layout)) {
		SYS_LOG_ERR("load resource failed\n");
		if (first_layout) app_mem_free(data);
		app_mem_free(resource);
		view_data->user_data = NULL;
		return -ENOENT;
	}

	if (first_layout == false) {
		lv_img_cache_invalidate_src(NULL);
		goto out_exit;
	}

	data->obj_clock = watch_clock_create(scr);
	watch_clock_set_type(data->obj_clock, ANALOG_CLOCK, true);
	watch_clock_set_24hour(data->obj_clock, data->is_digital);

	data->img_bg.header.w = 360;
	data->img_bg.header.h = 360;
	data->img_bg.header.cf = LV_IMG_CF_TRUE_COLOR;
	sd_fmap("bg_565.rgb",(void **)&data->img_bg.data,&data->img_bg.data_size);

	lv_obj_set_size(data->obj_clock, data->scene.width, data->scene.height);
	if (data->img_bg.data) {
		lv_obj_set_style_bg_img_src(data->obj_clock, &data->img_bg, LV_PART_MAIN);
		lv_obj_set_style_bg_img_opa(data->obj_clock, LV_OPA_COVER, LV_PART_MAIN);
	} else {
		lv_obj_set_style_bg_color(data->obj_clock, data->scene.background, LV_PART_MAIN);
		lv_obj_set_style_bg_opa(data->obj_clock, LV_OPA_COVER, LV_PART_MAIN);
	}
	lv_obj_set_user_data(data->obj_clock, view_data);
	lv_obj_add_event_cb(data->obj_clock, _clock_event_handler, LV_EVENT_ALL, view_data);

	lv_coord_t pivot_x, pivot_y;


	pivot_x = (data->img_bg.header.w / 2) - resource->grp_hour.x;
	pivot_y = data->img_tm_hour[0].header.h - 1 - ((data->img_bg.header.h / 2) - resource->grp_hour.y);

	watch_clock_set_pointer_images(data->obj_clock, CLOCK_HOUR, 1, data->img_tm_hour, pivot_x, pivot_y);

	pivot_x = (data->img_bg.header.w / 2) - resource->grp_minute.x;
	pivot_y = data->img_tm_min[0].header.h - 1 - ((data->img_bg.header.h / 2) - resource->grp_minute.y);

	watch_clock_set_pointer_images(data->obj_clock, CLOCK_MIN, 1, data->img_tm_min, pivot_x, pivot_y);

	if (data->img_tm_sec) {
		pivot_x = (data->img_bg.header.w / 2) - resource->grp_second.x;
		pivot_y = data->img_tm_sec[0].header.h - 1 - ((data->img_bg.header.h / 2) - resource->grp_second.y);
		watch_clock_set_pointer_images(data->obj_clock, CLOCK_SEC, 1, data->img_tm_sec, pivot_x, pivot_y);
	}

out_exit:
	app_mem_free(resource);

	/* update values */
	_clock_view_paint(view_data, true);

	return 0;
}

static int _clock_view_delete(view_data_t *view_data)
{
	clock_view_data_t *data = view_data->user_data;
	int i;

	_clock_view_unload_resource(data, true);

	if (data) {
		lv_obj_del(data->obj_clock);
		for (i = 0; i < ARRAY_SIZE(data->sty_text); i++) {
			lv_style_reset(&data->sty_text[i]);
		}

		app_mem_free(data);
	}

	return 0;
}

static int _clock_view_paint(view_data_t *view_data, bool forced)
{
	const clock_view_presenter_t *presenter = &clock_view_presenter;
	clock_view_data_t *data = view_data->user_data;
	struct rtc_time time;

	if (data->obj_clock == NULL) {
		return 0;
	}

	if (forced) {
		lv_obj_invalidate(lv_scr_act());
		if (data->img_bg.data) {
			lv_obj_set_style_bg_img_src(data->obj_clock, &data->img_bg, LV_PART_MAIN);
			lv_obj_set_style_bg_img_opa(data->obj_clock, LV_OPA_COVER, LV_PART_MAIN);
		} else {
			lv_obj_set_style_bg_color(data->obj_clock, data->scene.background, LV_PART_MAIN);
			lv_obj_set_style_bg_opa(data->obj_clock, LV_OPA_COVER, LV_PART_MAIN);
		}
	}

	/* update data via presenter*/
	presenter->get_time(&time);

	SYS_LOG_DBG("time %02u:%02u:%02u.%03u\n", time.tm_hour, time.tm_min, time.tm_sec, time.tm_ms);

	watch_clock_set_time(data->obj_clock, time.tm_hour, time.tm_min, time.tm_sec * 1000 + time.tm_ms);

	return 0;
}

static int _clock_view_focus_changed(view_data_t *view_data, bool focused)
{
	/* data is allocate in preload() */
	clock_view_data_t *data = view_data->user_data;

	if (focused) {
		if (data) {
			if (!lvgl_res_scene_is_loaded(SCENE_CLOCK_VIEW)) {
				SYS_LOG_INF("preload clock_view\n");
				_clock_view_preload(view_data, true);
			} else {
				if (data->obj_info[INFO_ANIM]) {
					SYS_LOG_INF("_clock_view anim_img_start no need to preload\n");
					anim_img_start(data->obj_info[INFO_ANIM], true);
				}
			}
		}
	} else {
		if (data) {
			if (data->obj_info[INFO_ANIM]) {
				anim_img_stop(data->obj_info[INFO_ANIM]);
				anim_img_clean(data->obj_info[INFO_ANIM]);
			}
		}

		_clock_view_unload_resource(data, false);
	}

	return 0;
}

int _clock_view_handler(uint16_t view_id, uint8_t msg_id, void *msg_data)
{
	view_data_t *view_data = msg_data;

	assert(view_id == CLOCK_VIEW);

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _clock_view_preload(view_data, false);
	case MSG_VIEW_LAYOUT:
		return _clock_view_layout_update(view_data, true);
	case MSG_VIEW_DELETE:
		return _clock_view_delete(view_data);
	case MSG_VIEW_PAINT:
	case MSG_VIEW_RESUME_DISPLAY:
		return _clock_view_paint(view_data, false);
	case MSG_VIEW_FOCUS:
		return _clock_view_focus_changed(view_data, true);
	case MSG_VIEW_DEFOCUS:
		return _clock_view_focus_changed(view_data, false);
	case MSG_VIEW_UPDATE:
		return _clock_view_layout_update(view_data, false);
	default:
		return 0;
	}
}

VIEW_DEFINE(clock, _clock_view_handler, NULL, NULL, CLOCK_VIEW,
		NORMAL_ORDER, UI_VIEW_LVGL, DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);

static bool clock_view_actived = false;

int clock_view_create(void)
{
	if (clock_view_actived) 
       return 0;
	clock_view_actived = true;
	printk("clock_view_create \n");
	return _clock_view_layout_update(&globle_view_data, true);
}

int clock_view_suspend(void)
{
	return 0;
}
static int resume_cnt;
int clock_view_resume(void)
{
	resume_cnt = 36;
	return _clock_view_paint(&globle_view_data, true);
}

int clock_view_repaint(void)
{
	int ret = 0;
	static int timestamp = 0;
	if (!clock_view_actived) {
		return ret;
	}
	if(k_cyc_to_ms_floor32(k_cycle_get_32() - timestamp) >= CONFIG_CLOCK_DEF_REFR_PERIOD) {
		if(resume_cnt >= 0) {
			resume_cnt--;
			ret = _clock_view_paint(&globle_view_data, true);
		} else {
			ret = _clock_view_paint(&globle_view_data, false);
		}
		timestamp  = k_cycle_get_32();
	} 	
	return ret;
}


int clock_view_delete(void)
{
	clock_view_actived = false;
	printk("clock_view_delete \n");
	return _clock_view_delete(&globle_view_data);
}
