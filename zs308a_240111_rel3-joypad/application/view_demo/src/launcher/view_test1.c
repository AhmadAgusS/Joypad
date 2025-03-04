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
#include <ui_surface.h>
#include <app_ui.h>
#include "gui.h"

extern const uint8_t pic_a4_360x576[103680UL + 1];
static uint8_t pic_a4[360 * 360 / 2] __aligned(4);

static int _test_view1_preload(view_data_t *view_data)
{
	memcpy(pic_a4, pic_a4_360x576, sizeof(pic_a4));
	mem_dcache_clean(pic_a4, sizeof(pic_a4));
	mem_dcache_sync();

	return ui_view_layout(TEST_VIEW_1);
}

static int _test_view1_layout(view_data_t *view_data)
{
	gui_disp_t * disp = view_data->display;
	uint8_t flags = SURFACE_FIRST_DRAW | SURFACE_LAST_DRAW;
	graphic_buffer_t *draw_buf = NULL;
	uint8_t *draw_ptr = NULL;
	ui_region_t rect;
	int res;

	rect.x1 = ((DEF_UI_WIDTH - 360) / 2) & ~0x1;
	rect.y1 = ((DEF_UI_HEIGHT - 360) / 2) & ~0x1;
	rect.x2 = rect.x1 + 359;
	rect.y2 = rect.y1 + 359;

	surface_begin_frame(disp->surface);
	surface_begin_draw(disp->surface, flags, &draw_buf);

	draw_ptr = (uint8_t *)graphic_buffer_get_bufptr(draw_buf, rect.x1, rect.y1);

	res = gui_dma2d_fill_mask(draw_ptr, graphic_buffer_get_pixel_format(draw_buf),
			graphic_buffer_get_stride(draw_buf), pic_a4, PIXEL_FORMAT_A4_LE,
			360, 360, 360, display_color_make(255, 0, 0, 255));
	if (res) { /* memset instead */
		uint16_t len = ui_region_get_width(&rect) * graphic_buffer_get_bits_per_pixel(draw_buf) / 8;
		uint16_t bytes_per_line = graphic_buffer_get_bytes_per_line(draw_buf);

		for (int i = ui_region_get_height(&rect); i > 0; i--) {
			memset(draw_ptr, 0x80, len);
			mem_dcache_clean(draw_ptr, len);
			draw_ptr += bytes_per_line;
		}

		mem_dcache_sync();
	} else {
		gui_dma2d_poll();
	}

	surface_end_draw(disp->surface, &rect);
	surface_end_frame(disp->surface);
	return 0;
}

static int _test_view1_handler(uint16_t view_id, uint8_t msg_id, void *msg_data)
{
	view_data_t *view_data = msg_data;

	assert(view_id == TEST_VIEW_1);

	switch (msg_id) {
	case MSG_VIEW_PRELOAD:
		return _test_view1_preload(view_data);
	case MSG_VIEW_LAYOUT:
		return _test_view1_layout(view_data);
	case MSG_VIEW_FOCUS:
	case MSG_VIEW_DEFOCUS:
	case MSG_VIEW_DELETE:
	default:
		return 0;
	}
}

VIEW_DEFINE(test_view1, _test_view1_handler, NULL,
		NULL, TEST_VIEW_1, NORMAL_ORDER, UI_VIEW_USER,
		DEF_UI_VIEW_WIDTH, DEF_UI_VIEW_HEIGHT);
