/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <memory/mem_cache.h>
#include <assert.h>
#include <ui_manager.h>
#include <view_manager.h>
#include <display/sw_draw.h>
#include <app_ui.h>
#include "gui.h"

extern const uint8_t pic_argb8888_204x204[];
static uint8_t pic_argb8888[204 * 204 * 4] __aligned(4);

static int _test_view2_preload(view_data_t *view_data)
{
	memcpy(pic_argb8888, pic_argb8888_204x204, sizeof(pic_argb8888));
	mem_dcache_clean(pic_argb8888, sizeof(pic_argb8888));
	mem_dcache_sync();

	return ui_view_layout(TEST_VIEW_2);
}

static int _test_view2_layout(view_data_t *view_data)
{
	gui_disp_t * disp = view_data->display;
	uint8_t flags = SURFACE_FIRST_DRAW | SURFACE_LAST_DRAW;
	graphic_buffer_t *draw_buf = NULL;
	uint8_t *draw_ptr = NULL;
	uint32_t pixel_format;
	ui_region_t rect;
	int res;

	rect.x1 = ((DEF_UI_WIDTH - 204) / 2) & ~0x1;
	rect.y1 = ((DEF_UI_HEIGHT - 204) / 2) & ~0x1;
	rect.x2 = rect.x1 + 203;
	rect.y2 = rect.y1 + 203;

	surface_begin_frame(disp->surface);
	surface_begin_draw(disp->surface, flags, &draw_buf);

	pixel_format = graphic_buffer_get_pixel_format(draw_buf);
	draw_ptr = (void *)graphic_buffer_get_bufptr(draw_buf, rect.x1, rect.y1);

	res = gui_dma2d_blend(draw_ptr, pixel_format, graphic_buffer_get_stride(draw_buf),
			pic_argb8888, PIXEL_FORMAT_ARGB_8888, 204, 204, 204, 255);
	if (res) {
		if (pixel_format == PIXEL_FORMAT_RGB_565) {
			sw_blend_argb8888_over_rgb565(draw_ptr, pic_argb8888,
					graphic_buffer_get_stride(draw_buf), 204, 204, 204);
		} else if (pixel_format == PIXEL_FORMAT_ARGB_8888) {
			sw_blend_argb8888_over_argb8888(draw_ptr, pic_argb8888,
					graphic_buffer_get_stride(draw_buf), 204, 204, 204);
		}

		mem_dcache_clean(draw_ptr, graphic_buffer_get_bytes_per_line(draw_buf) * 204);
		mem_dcache_sync();
	} else {
		gui_dma2d_poll();
	}

	surface_end_draw(disp->surface, &rect);
	surface_end_frame(disp->surface);
	return 0;
}

static int _test_view2_handler(uint16_t view_id, uint8_t msg_id, void *msg_data)
{
	view_data_t *view_data = msg_data;

	assert(view_id == TEST_VIEW_2);

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _test_view2_preload(view_data);
	case MSG_VIEW_LAYOUT:
		return _test_view2_layout(view_data);
	case MSG_VIEW_FOCUS:
	case MSG_VIEW_DEFOCUS:
	case MSG_VIEW_DELETE:
	default:
		return 0;
	}
}

VIEW_DEFINE(test_view2, _test_view2_handler, NULL,
		NULL, TEST_VIEW_2, NORMAL_ORDER, UI_VIEW_USER,
		DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
