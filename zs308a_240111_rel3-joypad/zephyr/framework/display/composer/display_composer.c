/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_MODULE_CUSTOMER

#include <assert.h>
#include <string.h>
#include <sys/slist.h>
#include <zephyr.h>
#include <drivers/display/display_engine.h>
#ifdef CONFIG_TRACING
#  include <tracing/tracing.h>
#endif

#include <display/display_composer.h>
#include <board_cfg.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(composer, LOG_LEVEL_INF);

#define CONFIG_COMPOSER_POST_NO_WAIT
#define CONFIG_COMPOSER_POST_DE_CONTINUOUS (0)

#ifdef CONFIG_PANEL_FULL_SCREEN_OPT_AREA
static const ui_region_t post_clip_areas[] = CONFIG_PANEL_FULL_SCREEN_OPT_AREA;

#  define CONFIG_COMPOSER_NUM_CLIP_AREAS ARRAY_SIZE(post_clip_areas)
#else
#  define CONFIG_COMPOSER_NUM_CLIP_AREAS (1)
#endif /* CONFIG_PANEL_FULL_SCREEN_OPT_AREA */

/* 3 frames */
#define NUM_POST_ENTRIES  (CONFIG_COMPOSER_NUM_CLIP_AREAS * 3)
#define NUM_POST_LAYERS   (2)

typedef struct post_entry {
	uint32_t flags;

	display_layer_t ovls[NUM_POST_LAYERS];
	display_buffer_t bufs[NUM_POST_LAYERS];

	graphic_buffer_t *graphic_bufs[NUM_POST_LAYERS];
	display_composer_post_cleanup_t cleanup_cb[NUM_POST_LAYERS];
	void *cleanup_data[NUM_POST_LAYERS];
} post_entry_t;

typedef struct display_composer {
	/* supported maximum layers */
	uint8_t max_layers;

	uint8_t post_inprog : 1;
	uint8_t disp_noram : 1;
	uint8_t disp_has_vsync : 1; /* test really has vsyn signal */
	uint8_t __pad : 5;
	uint8_t disp_active : 1;

	/* post entries */
	uint8_t free_idx;
	uint8_t post_idx;
	uint8_t complete_idx;
	uint8_t post_cnt;

#if CONFIG_COMPOSER_POST_DE_CONTINUOUS
	uint8_t de_posting_cnt; /* de post count */
#endif

	post_entry_t post_entries[NUM_POST_ENTRIES];

	struct k_spinlock post_lock;
#ifndef CONFIG_COMPOSER_POST_NO_WAIT
	struct k_sem post_sem;
#endif

	/* display engine device */
	const struct device *de_dev;
	int de_inst;

	/* display (panel) device */
	const struct device *disp_dev;
	/* display device callback */
	struct display_callback disp_cb;
	/* display write pixel format */
	uint32_t disp_pixel_format;
	uint16_t disp_x_res;
	uint16_t disp_y_res;

	/* user display callback */
	const struct display_callback *user_cb;

#ifdef CONFIG_DISPLAY_COMPOSER_DEBUG_FPS
	uint32_t frame_timestamp;
	uint16_t frame_cnt;
#endif

#ifdef CONFIG_DISPLAY_COMPOSER_DEBUG_VSYNC
	uint32_t vsync_timestamp; /* measure in cycles */
	uint32_t vsync_print_timestamp; /* measure in cycles */
#endif
} display_composer_t;

/* static prototypes */
static void _composer_display_vsync_handler(const struct display_callback *callback, uint32_t timestamp);
static void _composer_display_complete_handler(const struct display_callback *callback);
static void _composer_display_pm_notify_handler(const struct display_callback *callback, uint32_t pm_action);
static uint8_t _composer_num_free_entries_get(display_composer_t *composer);
static post_entry_t *_composer_find_free_entry(display_composer_t *composer);
static int _composer_post_top_entry(display_composer_t *composer, bool require_not_first);
static void _composer_cleanup_entry(display_composer_t *composer, post_entry_t *entry);
static void _composer_dump_entry(post_entry_t *entry);

static int _composer_post_entry_noram(display_composer_t *composer);
static void _composer_de_complete_handler(int status, uint16_t cmd_seq, void *user_data);

/* static variables */
static display_composer_t display_composer __in_section_unique(ram.noinit.display_composer);

static inline display_composer_t *_composer_get(void)
{
	return &display_composer;
}

int display_composer_init(void)
{
	display_composer_t *composer = _composer_get();
	union {
		struct display_capabilities panel;
		struct display_engine_capabilities engine;
	} capabilities;

	memset(composer, 0, sizeof(*composer));
	composer->disp_active = 1;

	composer->disp_dev = device_get_binding(CONFIG_LCD_DISPLAY_DEV_NAME);
	if (composer->disp_dev == NULL) {
		SYS_LOG_ERR("cannot find display " CONFIG_LCD_DISPLAY_DEV_NAME);
		return -ENODEV;
	}

	composer->de_dev = device_get_binding(CONFIG_DISPLAY_ENGINE_DEV_NAME);
	if (composer->de_dev) {
		composer->de_inst = display_engine_open(composer->de_dev,
				DISPLAY_ENGINE_FLAG_HIGH_PRIO | DISPLAY_ENGINE_FLAG_POST);
	} else {
		composer->de_inst = -1;
	}

	if (composer->de_inst >= 0) {
		display_engine_get_capabilities(composer->de_dev, &capabilities.engine);
		composer->max_layers = MIN(capabilities.engine.num_overlays, NUM_POST_LAYERS);
	} else {
		composer->max_layers = 1;
	}

	SYS_LOG_INF("supported layer num %d", composer->max_layers);

#ifndef CONFIG_COMPOSER_POST_NO_WAIT
	k_sem_init(&composer->post_sem, NUM_POST_ENTRIES, NUM_POST_ENTRIES);
#endif

	display_get_capabilities(composer->disp_dev, &capabilities.panel);
	composer->disp_pixel_format = capabilities.panel.current_pixel_format;
	composer->disp_x_res = capabilities.panel.x_resolution;
	composer->disp_y_res = capabilities.panel.y_resolution;
	if (!(capabilities.panel.screen_info & SCREEN_INFO_VSYNC)) {
		SYS_LOG_ERR("vsync unsupported\n");
	}

	if (capabilities.panel.screen_info & SCREEN_INFO_ZERO_BUFFER) {
		SYS_LOG_WRN("panel no ram\n");
		composer->disp_noram = 1;
		assert(composer->de_inst >= 0);
		display_engine_register_callback(composer->de_dev, composer->de_inst,
			_composer_de_complete_handler, composer);
	}

	composer->disp_cb.vsync = _composer_display_vsync_handler;
	composer->disp_cb.complete = _composer_display_complete_handler;
	composer->disp_cb.pm_notify = _composer_display_pm_notify_handler;
	display_register_callback(composer->disp_dev, &composer->disp_cb);

	SYS_LOG_INF("composer initialized\n");
	return 0;
}

void display_composer_destroy(void)
{
	display_composer_t *composer = _composer_get();

	if (composer->disp_dev == NULL) {
		return;
	}

	composer->user_cb = NULL;

#if 0
	display_unregister_callback(composer->disp_dev, &composer->disp_cb);

	if (composer->de_inst >= 0) {
		display_engine_close(composer->de_dev, composer->de_inst);
	}
#endif

	SYS_LOG_INF("composer finalized\n");
}

void display_composer_register_callback(const struct display_callback *callback)
{
	display_composer_t *composer = _composer_get();

	composer->user_cb = callback;
}

uint32_t display_composer_get_vsync_period(void)
{
	display_composer_t *composer = _composer_get();
	struct display_capabilities capabilities;

	if (composer->disp_dev == NULL) {
		return -ENODEV;
	}

	capabilities.vsync_period = 0;
	display_get_capabilities(composer->disp_dev, &capabilities);

	/* fallback to refresh rate 60 Hz */
	return capabilities.vsync_period ? capabilities.vsync_period : 16667;
}

int display_composer_get_geometry(
		uint16_t *width, uint16_t *height, uint32_t *pixel_format)
{
	display_composer_t *composer = _composer_get();

	if (width) {
		*width = composer->disp_x_res;
	}

	if (height) {
		*height = composer->disp_y_res;
	}

	if (pixel_format) {
		*pixel_format = composer->disp_pixel_format;
	}

	return 0;
}

int display_composer_set_blanking(bool blanking_on)
{
	display_composer_t *composer = _composer_get();
	int res = 0;

	if (composer->disp_dev == NULL) {
		return -ENODEV;
	}

	if (blanking_on) {
		res = display_blanking_on(composer->disp_dev);
	} else {
		res = display_blanking_off(composer->disp_dev);
	}

	return res;
}

int display_composer_set_brightness(uint8_t brightness)
{
	display_composer_t *composer = _composer_get();

	if (composer->disp_dev == NULL) {
		return -ENODEV;
	}

	return display_set_brightness(composer->disp_dev, brightness);
}

int display_composer_set_contrast(uint8_t contrast)
{
	display_composer_t *composer = _composer_get();

	if (composer->disp_dev == NULL) {
		return -ENODEV;
	}

	return display_set_contrast(composer->disp_dev, contrast);
}

void display_composer_round(ui_region_t *region)
{
	display_composer_t *composer = _composer_get();

	if (composer->disp_noram == 0) {
		/* Some LCD driver IC require even pixel alignment, so set even area if possible */
		if (!(composer->disp_x_res & 0x1)) {
			region->x1 &= ~0x1;
			region->x2 |= 0x1;
		}

		if (!(composer->disp_y_res & 0x1)) {
			region->y1 &= ~0x1;
			region->y2 |= 0x1;
		}
	} else {
		/* force full-screen refresh */
		region->x1 = 0;
		region->y1 = 0;
		region->x2 = composer->disp_x_res - 1;
		region->y2 = composer->disp_y_res - 1;
	}
}

static void _composer_display_vsync_handler(const struct display_callback *callback, uint32_t timestamp)
{
	display_composer_t *composer = CONTAINER_OF(callback, display_composer_t, disp_cb);

	if (!composer->disp_has_vsync) {
		composer->disp_has_vsync = 1;
		LOG_INF("has vsync\n");
	}

	if (!composer->disp_noram && composer->post_cnt > 0 && !composer->post_inprog) {
		_composer_post_top_entry(composer, false);
	}

	if (composer->user_cb && composer->user_cb->vsync) {
		composer->user_cb->vsync(composer->user_cb, timestamp);
	}

#ifdef CONFIG_DISPLAY_COMPOSER_DEBUG_VSYNC
	if (timestamp - composer->vsync_print_timestamp >= sys_clock_hw_cycles_per_sec() * 8u) {
		LOG_INF("vsync %u us\n", k_cyc_to_us_near32(timestamp - composer->vsync_timestamp));
		composer->vsync_print_timestamp = timestamp;
	}

	composer->vsync_timestamp = timestamp;
#endif
}

static void _composer_complete_one_entry(display_composer_t *composer)
{
	post_entry_t *entry = &composer->post_entries[composer->complete_idx];

	_composer_cleanup_entry(composer, entry);
	if (++composer->complete_idx >= NUM_POST_ENTRIES) {
		composer->complete_idx = 0;
	}

	--composer->post_cnt;
}

static void _composer_display_complete_handler(const struct display_callback *callback)
{
	display_composer_t *composer = CONTAINER_OF(callback, display_composer_t, disp_cb);
	post_entry_t *entry = &composer->post_entries[composer->complete_idx];

	composer->post_inprog = 0;

#if CONFIG_COMPOSER_POST_DE_CONTINUOUS
	if (entry->flags & POST_PATH_BY_DE) {
		--composer->de_posting_cnt;
	}

	if (--composer->post_cnt > 0) {
		if (composer->de_posting_cnt == 0) {
			_composer_post_top_entry(composer, true);
		} else {
			composer->post_inprog = 1;
		}
	}
#else
	if (--composer->post_cnt > 0) {
		_composer_post_top_entry(composer, true);
	}
#endif /* CONFIG_COMPOSER_POST_DE_CONTINUOUS */

	_composer_cleanup_entry(composer, entry);
	if (++composer->complete_idx >= NUM_POST_ENTRIES) {
		composer->complete_idx = 0;
	}

	if (composer->post_cnt == 0 && composer->user_cb && composer->user_cb->complete) {
		composer->user_cb->complete(composer->user_cb);
	}
}

static void _composer_de_complete_handler(int status, uint16_t cmd_seq, void *user_data)
{
	display_composer_t *composer = user_data;

	/* TODO: recovery the display */
	assert(status == 0);

	_composer_complete_one_entry(composer);

	if (composer->post_cnt == 0 && composer->user_cb && composer->user_cb->complete) {
		composer->user_cb->complete(composer->user_cb);
	}
}

static void _composer_drop_all_entries(display_composer_t *composer)
{
	k_spinlock_key_t key = k_spin_lock(&composer->post_lock);

	while (composer->post_cnt > 0) {
		_composer_complete_one_entry(composer);
	}

	composer->post_inprog = 0;
	composer->post_idx = composer->free_idx;
	composer->complete_idx =composer->free_idx;

	if (composer->user_cb && composer->user_cb->complete) {
		composer->user_cb->complete(composer->user_cb);
	}

	k_spin_unlock(&composer->post_lock, key);
}

static void _composer_display_pm_notify_handler(const struct display_callback *callback, uint32_t pm_action)
{
	display_composer_t *composer = CONTAINER_OF(callback, display_composer_t, disp_cb);

	if (pm_action == PM_DEVICE_ACTION_EARLY_SUSPEND ||
		pm_action == PM_DEVICE_ACTION_LOW_POWER) {
		uint8_t wait_post_cnt = composer->disp_noram ? 1 : 0;
		int try_cnt = 500;

		composer->disp_active = 0; /* lock the post temporarily */

		if (composer->user_cb && composer->user_cb->pm_notify) {
			composer->user_cb->pm_notify(composer->user_cb, pm_action);
		}

		while (composer->post_cnt > wait_post_cnt && try_cnt-- > 0) {
			SYS_LOG_INF("post cnt %d", composer->post_cnt);
			os_sleep(2);
		}

		if (!composer->disp_noram && composer->post_cnt > 0) {
			SYS_LOG_ERR("wait timeout, drop all post entries");
			_composer_drop_all_entries(composer);
		}

		composer->disp_active = (pm_action != PM_DEVICE_ACTION_EARLY_SUSPEND);
	} else {
		composer->disp_active = 1;

		if (composer->user_cb && composer->user_cb->pm_notify) {
			composer->user_cb->pm_notify(composer->user_cb, pm_action);
		}
	}
}

static uint8_t _composer_num_free_entries_get(display_composer_t *composer)
{
	return NUM_POST_ENTRIES - composer->post_cnt;
}

static post_entry_t *_composer_find_free_entry(display_composer_t *composer)
{
	post_entry_t *entry;

#ifdef CONFIG_COMPOSER_POST_NO_WAIT
	if (composer->post_cnt >= NUM_POST_ENTRIES)
		return NULL;
#else
	if (k_sem_take(&composer->post_sem, k_is_in_isr() ? K_NO_WAIT : K_FOREVER))
		return NULL;
#endif

	entry = &composer->post_entries[composer->free_idx];
	if (++composer->free_idx >= NUM_POST_ENTRIES)
		composer->free_idx = 0;

	return entry;
}

static int _composer_post_top_entry(display_composer_t *composer, bool require_not_first)
{
	post_entry_t *entry = &composer->post_entries[composer->post_idx];
	int res;

	if (require_not_first && (entry->flags & FIRST_POST_IN_FRAME)) {
		return -EINVAL;
	}

	composer->post_inprog = 1;

	/* do compose for DE as many as possible, so DE has chance to preconfig regs */
	do {
		display_layer_t *ovls = entry->ovls;
		uint8_t num_layers = ovls[1].buffer ? 2 : 1;

		if (entry->flags & POST_PATH_BY_DE) {
			res = display_engine_compose(composer->de_dev, composer->de_inst,
					NULL, ovls, num_layers, false);
#if CONFIG_COMPOSER_POST_DE_CONTINUOUS
			composer->de_posting_cnt++;
#endif
		} else {
			res = display_write(composer->disp_dev, ovls[0].frame.x, ovls[0].frame.y,
					&ovls[0].buffer->desc, (void *)ovls[0].buffer->addr);
		}

		assert(res >= 0);
		if (res < 0) {
			SYS_LOG_ERR("post failed\n");
			_composer_dump_entry(entry);
			return res;
		}

		if (++composer->post_idx >= NUM_POST_ENTRIES)
			composer->post_idx = 0;

#if CONFIG_COMPOSER_POST_DE_CONTINUOUS
		if (composer->de_posting_cnt >= composer->post_cnt)
			break;

		entry = &composer->post_entries[composer->post_idx];
		if ((entry->flags & (POST_PATH_BY_DE | FIRST_POST_IN_FRAME)) != POST_PATH_BY_DE)
			break;
#else
		break;
#endif
	} while (1);

	return 0;
}

static int _composer_post_entry_noram(display_composer_t *composer)
{
	post_entry_t *entry = &composer->post_entries[composer->post_idx];
	display_layer_t *ovls = entry->ovls;
	uint8_t num_layers = ovls[1].buffer ? 2 : 1;
	int res;

	res = display_engine_compose(composer->de_dev, composer->de_inst,
			NULL, ovls, num_layers, false);

	assert(res >= 0);

	if (++composer->post_idx >= NUM_POST_ENTRIES)
		composer->post_idx = 0;

	return res;
}

static void _composer_cleanup_entry(display_composer_t *composer, post_entry_t *entry)
{
	for (int i = 0; i < ARRAY_SIZE(entry->graphic_bufs); i++) {
		if (entry->graphic_bufs[i]) {
			graphic_buffer_unref(entry->graphic_bufs[i]);
		}

		if (entry->cleanup_cb[i]) {
			entry->cleanup_cb[i](entry->cleanup_data[i]);
		}
	}

	memset(entry, 0, sizeof(*entry));

#ifndef CONFIG_COMPOSER_POST_NO_WAIT
	k_sem_give(&composer->post_sem);
#endif
}

static void _composer_dump_entry(post_entry_t *entry)
{
	display_layer_t *ovls = entry->ovls;
	uint8_t num_layers = ovls[1].buffer ? 2 : 1;
	uint8_t i;

	for (i = 0; i < num_layers; i++) {
		SYS_LOG_INF("L-%d: ptr=0x%08x fmt=0x%02x stride=%u color=0x%08x blend=0x%x frame=(%d,%d-%d,%d)%s",
			i, entry->bufs[i].addr, entry->bufs[i].desc.pixel_format, entry->bufs[i].desc.pitch,
			ovls[i].color.full, ovls[i].blending, ovls[i].frame.x,
			ovls[i].frame.y, ovls[i].frame.w, ovls[i].frame.h,
			i == num_layers - 1 ? "\n" : "");
	}
}

int display_composer_simple_post(graphic_buffer_t *buffer,
		ui_region_t *crop, uint16_t x, uint16_t y)
{
	ui_layer_t layer;

	memset(&layer, 0, sizeof(layer));
	layer.buffer = buffer;

	if (crop) {
		memcpy(&layer.crop, crop, sizeof(*crop));
	} else {
		layer.crop.x2 = graphic_buffer_get_width(buffer) - 1;
		layer.crop.y2 = graphic_buffer_get_height(buffer) - 1;
	}

	layer.frame.x1 = x;
	layer.frame.y1 = y;
	layer.frame.x2 = x + ui_region_get_width(&layer.crop) - 1;
	layer.frame.y2 = y + ui_region_get_height(&layer.crop) - 1;

	return display_composer_post(&layer, 1, FIRST_POST_IN_FRAME | LAST_POST_IN_FRAME);
}

static int _composer_post_inner(const ui_layer_t *layers, int num_layers, uint32_t post_flags)
{
	display_composer_t *composer = _composer_get();
	post_entry_t *entry = NULL;
	int res = -EINVAL;
	int i;

	/* Get the free entry */
	entry = _composer_find_free_entry(composer);
	if (entry == NULL) {
		goto fail_cleanup_cb;
	}

	entry->flags = post_flags;

	/* Validate the buffer */
	for (i = 0; i < num_layers; i++) {
		if (layers[i].buffer) {
			entry->bufs[i].addr = (uint32_t)graphic_buffer_get_bufptr(
					layers[i].buffer, layers[i].crop.x1, layers[i].crop.y1);

			entry->bufs[i].desc.pitch = graphic_buffer_get_stride(layers[i].buffer);
			entry->bufs[i].desc.pixel_format =
					graphic_buffer_get_pixel_format(layers[i].buffer);
			entry->bufs[i].desc.buf_size = entry->bufs[i].desc.pitch * entry->bufs[i].desc.height *
					display_format_get_bits_per_pixel(entry->bufs[i].desc.pixel_format) / 8;
		} else {
			entry->bufs[i].addr = 0;
			entry->bufs[i].desc.pitch = 0;
			entry->bufs[i].desc.pixel_format = 0;
		}

		entry->bufs[i].desc.width = ui_region_get_width(&layers[i].frame);
		entry->bufs[i].desc.height = ui_region_get_height(&layers[i].frame);

		entry->ovls[i].buffer = (entry->bufs[i].addr > 0) ? &entry->bufs[i] : NULL;
		entry->ovls[i].frame.x = layers[i].frame.x1;
		entry->ovls[i].frame.y = layers[i].frame.y1;
		entry->ovls[i].frame.w = entry->bufs[i].desc.width;
		entry->ovls[i].frame.h = entry->bufs[i].desc.height;
		entry->ovls[i].color.full = layers[i].color.full;
		entry->ovls[i].blending = layers[i].blending;

		assert(layers[i].frame.x1 >= 0 && layers[i].frame.y1 >= 0);
		assert(entry->ovls[i].frame.w > 0 && layers[i].frame.x2 < composer->disp_x_res);
		assert(entry->ovls[i].frame.h > 0 && layers[i].frame.y2 < composer->disp_y_res);

		if (!layers[i].buf_resident) {
			entry->graphic_bufs[i] = layers[i].buffer;
			if (layers[i].buffer)
				graphic_buffer_ref(layers[i].buffer);
		}

		entry->cleanup_cb[i] = layers[i].cleanup_cb;
		entry->cleanup_data[i] = layers[i].cleanup_data;
	}

	k_spinlock_key_t key = k_spin_lock(&composer->post_lock);
	composer->post_cnt++;

	if (composer->disp_noram) {
		_composer_post_entry_noram(composer);
	} else if (!composer->post_inprog) {
		_composer_post_top_entry(composer, true);
	}
	k_spin_unlock(&composer->post_lock, key);

	return 0;
fail_cleanup_cb:
	for (i = 0; i < num_layers; i++) {
		if (layers[i].cleanup_cb)
			layers[i].cleanup_cb(layers[i].cleanup_data);
	}
	return res;
}

static int _composer_post_part(const ui_layer_t *layers, int num_layers,
		uint32_t *post_flags, const ui_region_t *window)
{
	ui_layer_t tmp_layers[NUM_POST_LAYERS];
	int i, j = 0;

	for (i = 0; i < num_layers; i++) {
		if (window->y1 <= layers[i].frame.y2 && window->y2 >= layers[i].frame.y2) {
			tmp_layers[j].cleanup_cb = layers[i].cleanup_cb;
			tmp_layers[j].cleanup_data = layers[i].cleanup_data;
		} else {
			tmp_layers[j].cleanup_cb = NULL;
		}

		if (ui_region_intersect(&tmp_layers[j].frame, &layers[i].frame, window) == false) {
			if (tmp_layers[j].cleanup_cb) {
				tmp_layers[j].cleanup_cb(tmp_layers[j].cleanup_data);
				tmp_layers[j].cleanup_cb = NULL;
			}

			continue;
		}

		tmp_layers[j].buffer = layers[i].buffer;
		tmp_layers[j].buf_resident = layers[i].buf_resident;
		tmp_layers[j].color = layers[i].color;
		tmp_layers[j].blending = layers[i].blending;
		if (layers[i].buffer) {
			tmp_layers[j].crop.x1 = layers[i].crop.x1 + tmp_layers[j].frame.x1 - layers[i].frame.x1;
			tmp_layers[j].crop.y1 = layers[i].crop.y1 + tmp_layers[j].frame.y1 - layers[i].frame.y1;
		}

		j++;
	}

	if (j > 0 && _composer_post_inner(tmp_layers, j, *post_flags) == 0) {
		*post_flags &= ~FIRST_POST_IN_FRAME;
		return 0;
	}

	return -EINVAL;
}

int display_composer_post(const ui_layer_t *layers, int num_layers, uint32_t post_flags)
{
	display_composer_t *composer = _composer_get();
	const ui_region_t * clip_areas;
	uint8_t num_parts = CONFIG_COMPOSER_NUM_CLIP_AREAS;
	uint8_t num_free_entries;
	int res, i;

#ifdef CONFIG_DISPLAY_COMPOSER_DEBUG_FPS
	if (post_flags & LAST_POST_IN_FRAME) {
		uint32_t timestamp = k_cycle_get_32();

		++composer->frame_cnt;
		if ((timestamp - composer->frame_timestamp) >= sys_clock_hw_cycles_per_sec()) {
			LOG_INF("post fps %u\n", composer->frame_cnt);
			composer->frame_cnt = 0;
			composer->frame_timestamp = timestamp;
		}
	}
#endif

	if (composer->disp_dev == NULL) {
		SYS_LOG_ERR("composer not initialized");
		goto fail_cleanup_cb;
	}

	if (composer->disp_has_vsync == 0 || composer->disp_active == 0) {
		SYS_LOG_DBG("display active %d, has_vsync %d",
				composer->disp_active, composer->disp_has_vsync);
		goto fail_cleanup_cb;
	}

	if (num_layers <= 0 || num_layers > composer->max_layers) {
		SYS_LOG_ERR("invalid layer num %d", num_layers);
		goto fail_cleanup_cb;
	}

	if (num_layers > 1 || layers[0].buffer == NULL ||
		graphic_buffer_get_pixel_format(layers[0].buffer) != composer->disp_pixel_format) {
		post_flags |= POST_PATH_BY_DE;

		if (composer->de_inst < 0) {
			SYS_LOG_ERR("DE path not unavailable");
			goto fail_cleanup_cb;
		}
	}

	num_free_entries = _composer_num_free_entries_get(composer);
#ifdef CONFIG_COMPOSER_POST_NO_WAIT
	if (num_free_entries < num_parts)
#else
	if (k_is_in_isr() && num_free_entries < num_parts)
#endif
	{
		SYS_LOG_WRN("drop 1 frame (%d)", num_free_entries);
		sys_trace_void(SYS_TRACE_ID_COMPOSER_OVERFLOW);
		goto fail_cleanup_cb;
	}

#ifdef CONFIG_PANEL_FULL_SCREEN_OPT_AREA
	clip_areas = post_clip_areas;
#else
	clip_areas = NULL;
#endif

	if (clip_areas && (post_flags & POST_FULL_SCREEN_OPT)) {
		uint32_t flags = post_flags & ~LAST_POST_IN_FRAME;
		uint8_t i;

		_composer_post_part(layers, num_layers, &flags, &clip_areas[0]);

		for (i = 1; i < num_parts - 1; i++)
			_composer_post_part(layers, num_layers, &flags, &clip_areas[i]);

		flags |= (post_flags & LAST_POST_IN_FRAME);

		res = _composer_post_part(layers, num_layers, &flags, &clip_areas[num_parts - 1]);
	} else {
		res = _composer_post_inner(layers, num_layers, post_flags);
	}

	return 0;
fail_cleanup_cb:
	for (i = 0; i < num_layers; i++) {
		if (layers[i].cleanup_cb)
			layers[i].cleanup_cb(layers[i].cleanup_data);
	}
	return -EINVAL;
}

int display_composer_flush(unsigned int timeout)
{
	display_composer_t *composer = _composer_get();
	uint8_t wait_post_cnt = composer->disp_noram ? 1 : 0;
	uint8_t n_frames;
	uint32_t uptime;

	if (composer->disp_dev == NULL) {
		return -ENODEV;
	}

	if (composer->post_cnt <= wait_post_cnt) {
		return 0;
	}

	n_frames = composer->post_cnt - wait_post_cnt;
	uptime = os_uptime_get_32();

	do {
		if (composer->disp_noram || composer->post_inprog) {
			k_msleep(2);
		} else {
			k_spinlock_key_t key = k_spin_lock(&composer->post_lock);
			composer->disp_cb.vsync(&composer->disp_cb, os_cycle_get_32());
			k_spin_unlock(&composer->post_lock, key);
		}

		if (composer->post_cnt <= wait_post_cnt) {
			return n_frames;
		}
	} while (os_uptime_get_32() - uptime < timeout);

	return -ETIME;
}
