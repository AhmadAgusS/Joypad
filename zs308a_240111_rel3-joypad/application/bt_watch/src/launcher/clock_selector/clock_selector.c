/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <property_manager.h>
#include <ui_manager.h>
#include <view_manager.h>
#include <view_stack.h>
#include <lvgl/lvgl_res_loader.h>
#include <assert.h>
#include "launcher_app.h"
#include "clock_selector.h"
#include "clock_selector_view.h"
#include "clock_selector_subview.h"

static const clock_dsc_t clock_dsc[] = {
	{ "analog", SCENE_CLOCK_VIEW, CONFIG_CLOCK_DEF_REFR_PERIOD },
	{ "digital", SCENE_DIGITAL_CLOCK_VIEW, 1000 },

#ifdef SCENE_AOD_CLOCK_VIEW
	{ "aod", SCENE_AOD_CLOCK_VIEW, 60000 },
#endif
};

const clock_dsc_t * clocksel_get_clock_dsc(uint8_t index)
{
	if (index < ARRAY_SIZE(clock_dsc))
		return &clock_dsc[index];

	return NULL;
}

const clock_dsc_t * clocksel_get_aod_clock_dsc(void)
{
#ifdef SCENE_AOD_CLOCK_VIEW
	return &clock_dsc[ARRAY_SIZE(clock_dsc) - 1];
#else
	return NULL;
#endif
}

#ifdef CONFIG_CLOCK_SELECTOR_USE_VIEW_SLIDING
#define MAX_CLOCKSEL_SUBVIEWS 5

typedef struct clocksel_ctx {
	int8_t focus_idx; /* focused view index */
	int8_t map_clock[MAX_CLOCKSEL_SUBVIEWS]; /* mapped clock of every subview */
	uint8_t num_clock;
} clocksel_ctx_t;

static int8_t _clocksel_subview_get_clock_id(uint16_t view_id);
static const char * _clocksel_subview_get_clock_name(uint8_t id);
static void _clocksel_subview_set_clock_id(uint8_t id);

static const clocksel_subview_presenter_t clocksel_subview_presenter = {
	.get_clock_id = _clocksel_subview_get_clock_id,
	.get_clock_name = _clocksel_subview_get_clock_name,
	.set_clock_id = _clocksel_subview_set_clock_id,
};

static const uint16_t clocksel_subview_ids[MAX_CLOCKSEL_SUBVIEWS] = {
	CLOCK_SELECTOR_SUBVIEW_0,
	CLOCK_SELECTOR_SUBVIEW_1,
	CLOCK_SELECTOR_SUBVIEW_2,
	CLOCK_SELECTOR_SUBVIEW_3,
	CLOCK_SELECTOR_SUBVIEW_4,
};

static const void * clocksel_subview_presenters[MAX_CLOCKSEL_SUBVIEWS] = {
	&clocksel_subview_presenter,
	&clocksel_subview_presenter,
	&clocksel_subview_presenter,
	&clocksel_subview_presenter,
	&clocksel_subview_presenter,
};

static void _clocksel_scroll_cb(uint16_t view_id);

static view_group_dsc_t clocksel_group_dsc = {
	.vlist = clocksel_subview_ids,
	.plist = clocksel_subview_presenters,
	.num = MAX_CLOCKSEL_SUBVIEWS,
	.scroll_cb = _clocksel_scroll_cb,
};

static clocksel_ctx_t clocksel_ctx;

static int8_t _find_subview_index(uint16_t view_id)
{
	uint8_t count = MIN(clocksel_ctx.num_clock, ARRAY_SIZE(clocksel_subview_ids));
	int8_t idx;

	for (idx = count - 1; idx >= 0; idx--) {
		if (clocksel_subview_ids[idx] == view_id)
			break;
	}

	return idx;
}

static void _reset_subview_clock_index(int8_t focus_idx, int8_t map_clock_idx)
{
	int8_t view_idx, clock_idx, cnt;

	clocksel_ctx.focus_idx = focus_idx;

	if (clocksel_ctx.num_clock <= ARRAY_SIZE(clocksel_subview_ids)) {
		return; /* already set, do not change */
	}

	clocksel_ctx.map_clock[focus_idx] = map_clock_idx;

	view_idx = focus_idx - 1;
	clock_idx = map_clock_idx - 1;
	for (cnt = 2; cnt >= 0; cnt--, clock_idx--, view_idx--) {
		if (view_idx < 0) {
			view_idx += ARRAY_SIZE(clocksel_subview_ids);
		}

		if (clocksel_ctx.map_clock[view_idx] != clock_idx) {
			clocksel_ctx.map_clock[view_idx] = clock_idx;
			if (clock_idx >= 0) {
				ui_view_paint(clocksel_subview_ids[view_idx]);
			}
		}
	}

	view_idx = focus_idx + 1;
	clock_idx = map_clock_idx + 1;
	for (cnt = 2; cnt >= 0; cnt--, clock_idx++, view_idx++) {
		if (view_idx >= ARRAY_SIZE(clocksel_subview_ids)) {
			view_idx -= ARRAY_SIZE(clocksel_subview_ids);
		}

		int8_t idx = (clock_idx < clocksel_ctx.num_clock) ? clock_idx : -1;
		if (clocksel_ctx.map_clock[view_idx] != idx) {
			clocksel_ctx.map_clock[view_idx] = idx;
			if (idx >= 0) {
				ui_view_paint(clocksel_subview_ids[view_idx]);
			}
		}
	}
}

static void _set_drag_attr(uint8_t view_id, uint8_t attr)
{
	if (view_set_drag_attribute(view_id, attr, false))
		ui_view_set_drag_attribute(view_id, attr, false);
}

static void _reset_subview_drag_attr(bool set)
{
	int8_t focus_idx = clocksel_ctx.focus_idx;
	uint8_t left_attr = set ? UI_DRAG_MOVERIGHT : 0;
	uint8_t right_attr = set ? UI_DRAG_MOVELEFT : 0;

	if (!set)
		_set_drag_attr(clocksel_subview_ids[focus_idx], 0);

	if (clocksel_ctx.map_clock[focus_idx] > 0) {
		if (focus_idx > 0)
			_set_drag_attr(clocksel_subview_ids[focus_idx - 1], left_attr);
		else
			_set_drag_attr(clocksel_subview_ids[ARRAY_SIZE(clocksel_subview_ids) - 1], left_attr);
	}

	if (clocksel_ctx.map_clock[focus_idx] < ARRAY_SIZE(clock_dsc) - 1) {
		if (focus_idx < ARRAY_SIZE(clocksel_subview_ids) - 1)
			_set_drag_attr(clocksel_subview_ids[focus_idx + 1], right_attr);
		else
			_set_drag_attr(clocksel_subview_ids[0], right_attr);
	}
}

static int8_t _clocksel_subview_get_clock_id(uint16_t view_id)
{
	int8_t idx = _find_subview_index(view_id);

	assert(idx >= 0);
	return clocksel_ctx.map_clock[idx];
}

static const char * _clocksel_subview_get_clock_name(uint8_t id)
{
	return clock_dsc[id].name;
}

static void _clocksel_subview_set_clock_id(uint8_t id)
{
	launcher_app_t *app = launcher_app_get();

	SYS_LOG_INF("select clock %d", id);

	if (id != app->clock_id && id < clocksel_ctx.num_clock) {
#ifdef CONFIG_PROPERTY
		property_set_int(CFG_CLOCK_ID, id);
		property_flush(CFG_CLOCK_ID);
#endif

		launcher_apply_clock_id(id);
	}

	view_stack_pop();
}

static void _clocksel_scroll_cb(uint16_t view_id)
{
	int8_t idx = _find_subview_index(view_id);

	assert(idx >= 0);

	_reset_subview_drag_attr(false);
	_reset_subview_clock_index(idx, clocksel_ctx.map_clock[idx]);
	_reset_subview_drag_attr(true);
}

int clocksel_ui_enter(void)
{
	launcher_app_t *app = launcher_app_get();
	int res = 0;
	uint8_t subview_cnt;
	uint8_t i;

	SYS_LOG_INF("clocksel view enter");

	lvgl_res_clear_regular_pictures(clock_dsc[app->clock_id].scene, DEF_STY_FILE);
	lvgl_res_unload_scene_compact(clock_dsc[app->clock_id].scene);

	/* initial clock index */
	clocksel_ctx.num_clock = ARRAY_SIZE(clock_dsc);
	subview_cnt = MIN(clocksel_ctx.num_clock, ARRAY_SIZE(clocksel_subview_ids));

	if (clocksel_ctx.num_clock == subview_cnt) {
		clocksel_ctx.focus_idx = app->clock_id;
		for (i = 0; i < subview_cnt; i++) {
			clocksel_ctx.map_clock[i] = i;
		}
	} else {
		_reset_subview_clock_index(2, app->clock_id);
	}

	clocksel_group_dsc.num = subview_cnt;
	clocksel_group_dsc.idx = clocksel_ctx.focus_idx;

	res = view_stack_push_group(&clocksel_group_dsc);
	if (res == 0) {
		_reset_subview_drag_attr(true);
	}

	return res;
}

#else /* CONFIG_CLOCK_SELECTOR_USE_VIEW_SLIDING */

static uint32_t _clock_selector_view_get_clock_id(void);
static const char *_clock_selector_view_get_clock_name(uint32_t id);
static void _clock_selector_view_set_clock_id(uint32_t id);

static const clock_selector_view_presenter_t clock_selector_view_presenter = {
	.get_clock_id = _clock_selector_view_get_clock_id,
	.get_clock_name = _clock_selector_view_get_clock_name,
	.set_clock_id = _clock_selector_view_set_clock_id,
};

static uint32_t _clock_selector_view_get_clock_id(void)
{
	launcher_app_t *app = launcher_app_get();

	return app->clock_id;
}

static const char *_clock_selector_view_get_clock_name(uint32_t id)
{
	return clock_dsc[id].name;
}

static void _clock_selector_view_set_clock_id(uint32_t id)
{
	launcher_app_t *app = launcher_app_get();

	SYS_LOG_INF("select clock %d", id);

	if (id != app->clock_id && id < ARRAY_SIZE(clock_dsc)) {
#ifdef CONFIG_PROPERTY
		property_set_int(CFG_CLOCK_ID, id);
		property_flush(CFG_CLOCK_ID);
#endif

		launcher_apply_clock_id(id);
	}

	view_stack_pop();
}

int clocksel_ui_enter(void)
{
	SYS_LOG_INF("clocksel view enter");

	return view_stack_push_view(CLOCK_SELECTOR_VIEW, &clock_selector_view_presenter);
}

#endif /* CONFIG_CLOCK_SELECTOR_USE_VIEW_SLIDING */
