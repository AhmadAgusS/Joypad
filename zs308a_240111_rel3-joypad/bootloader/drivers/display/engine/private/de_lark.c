/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <spicache.h>
#include <drivers/display.h>
#include <drivers/display/display_controller.h>
#include <drivers/display/display_engine.h>
#include <assert.h>
#include <string.h>
#include <sys/byteorder.h>
#include <tracing/tracing.h>

#include "../de_common.h"
#include "../de_device.h"
#include "de_lark.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(de, CONFIG_DISPLAY_ENGINE_LOG_LEVEL);

#define SUPPORTED_OUTPUT_PIXEL_FORMATS \
	(PIXEL_FORMAT_ARGB_8888 | PIXEL_FORMAT_RGB_565)
#define SUPPORTED_INPUT_PIXEL_FORMATS                                          \
	(SUPPORTED_OUTPUT_PIXEL_FORMATS | PIXEL_FORMAT_ARGB_6666 |             \
	 PIXEL_FORMAT_ABGR_6666 | PIXEL_FORMAT_BGR_565 | PIXEL_FORMAT_RGB_565_LE | \
	 PIXEL_FORMAT_A8 | PIXEL_FORMAT_A4_LE | PIXEL_FORMAT_A1_LE)
#define SUPPORTED_ROTATE_PIXEL_FORMATS (PIXEL_FORMAT_ARGB_8888 | PIXEL_FORMAT_RGB_565)

#define DE  ((DE_Type *)DE_REG_BASE)
#define DE_INVALID_ADDR  0x02000000

#define A1_FONT_STRIDE(size) ROUND_UP(size, 8)
#define A4_FONT_STRIDE(size) ROUND_UP(size, 2)
#define A8_FONT_STRIDE(size) (size)

#ifdef CONFIG_DISPLAY_ENGINE_GAMMA_LUT
static int de_config_gamma(const uint8_t *gamma_lut, int len);

static const uint8_t dts_gamma_lut[] = CONFIG_DISPLAY_ENGINE_GAMMA_LUT;
#endif

static void de_reset(void)
{
	acts_reset_peripheral_assert(RESET_ID_DE);
	acts_clock_peripheral_enable(CLOCK_ID_DE);
	acts_reset_peripheral_deassert(RESET_ID_DE);

#ifdef CONFIG_DISPLAY_ENGINE_GAMMA_LUT
	de_config_gamma(dts_gamma_lut, ARRAY_SIZE(dts_gamma_lut));
#endif

	DE->MEM_OPT |= DE_MEM_BURST_32;
	DE->IRQ_CTL = /*DE_IRQ_DEV_FIFO_HF |*/ DE_IRQ_WB_FTC | DE_IRQ_DEV_FTC;
}

static int de_open(const struct device *dev, uint32_t flags)
{
	struct de_data *data = dev->data;
	int inst = -EMFILE;

	k_mutex_lock(&data->mutex, K_FOREVER);

	inst = de_alloc_instance(flags);
	if (inst < 0)
		goto out_unlock;

	if (data->open_count++ == 0) {
		clk_set_rate(CLOCK_ID_DE, MHZ(100));
		de_reset();
	}

out_unlock:
	k_mutex_unlock(&data->mutex);
	return inst;
}

static int de_close(const struct device *dev, int inst)
{
	struct de_data *data = dev->data;
	int ret = -EBUSY;

	ret = de_instance_poll(inst, -1);
	if (ret < 0)
		return ret;

	k_mutex_lock(&data->mutex, K_FOREVER);

	de_free_instance(inst);

	if (--data->open_count == 0) {
		acts_clock_peripheral_disable(CLOCK_ID_DE);
	}

	k_mutex_unlock(&data->mutex);

	return 0;
}

void de_dump(void)
{
	int i;

	printk("de regs:\n");
	printk("\t ctl                  0x%08x\n", DE->CTL);
	printk("\t gate_ctl             0x%08x\n", DE->GAT_CTL);
	printk("\t reg_ud               0x%08x\n", DE->REG_UD);
	printk("\t irq_ctl              0x%08x\n", DE->IRQ_CTL);
	printk("\t bg_size              0x%08x\n", DE->BG_SIZE);
	printk("\t bg_color             0x%08x\n", DE->BG_COLOR);
	printk("\t mem_opt              0x%08x\n", DE->MEM_OPT);
	printk("\t en                   0x%08x\n", DE->EN);
	printk("\t ctl2                 0x%08x\n", DE->CTL2);

	for (i = 0; i < ARRAY_SIZE(DE->LAYER_CTL); i++) {
		printk("\t l%d_pos               0x%08x\n", i, DE->LAYER_CTL[i].POS);
		printk("\t l%d_size              0x%08x\n", i, DE->LAYER_CTL[i].SIZE);
		printk("\t l%d_addr              0x%08x\n", i, DE->LAYER_CTL[i].ADDR);
		printk("\t l%d_stride            0x%08x\n", i, DE->LAYER_CTL[i].STRIDE);
		printk("\t l%d_length            0x%08x\n", i, DE->LAYER_CTL[i].LENGTH);
		printk("\t l%d_color_gain        0x%08x\n", i, DE->LAYER_CTL[i].COLOR_GAIN);
		printk("\t l%d_color_offset      0x%08x\n", i, DE->LAYER_CTL[i].COLOR_OFFSET);
		printk("\t l%d_def_color         0x%08x\n", i, DE->LAYER_CTL[i].DEF_COLOR);
	}

	printk("\t alpha_ctl            0x%08x\n", DE->ALPHA_CTL);
	printk("\t alpha_pos            0x%08x\n", DE->ALPHA_POS);
	printk("\t alpha_size           0x%08x\n", DE->ALPHA_SIZE);
	printk("\t sta                  0x%08x\n", DE->STA);
	printk("\t gamma_ctl            0x%08x\n", DE->GAMMA_CTL);
	printk("\t dither_ctl           0x%08x\n", DE->DITHER_CTL);
	printk("\t wb_addr              0x%08x\n", DE->WB_MEM_ADR);
	printk("\t wb_stride            0x%08x\n", DE->WB_MEM_STRIDE);
	printk("\t color_fill_pos       0x%08x\n", DE->COLOR_FILL_POS);
	printk("\t color_fill_size      0x%08x\n", DE->COLOR_FILL_SIZE);
	printk("\t fill_color           0x%08x\n", DE->FILL_COLOR);
	printk("\t a148_color           0x%08x\n", DE->A148_COLOR);
	printk("\t a14_ctl              0x%08x\n", DE->A14_CTL);

	printk("\t rt_ctl               0x%08x\n", DE->RT_CTL);
	printk("\t rt_img_size          0x%08x\n", DE->RT_IMG_SIZE);
	printk("\t rt_src_addr          0x%08x\n", DE->RT_SRC_ADDR);
	printk("\t rt_src_stride        0x%08x\n", DE->RT_SRC_STRIDE);
	printk("\t rt_dst_addr          0x%08x\n", DE->RT_DST_ADDR);
	printk("\t rt_dst_stride        0x%08x\n", DE->RT_DST_STRIDE);
	printk("\t rt_start_height      0x%08x\n", DE->RT_START_HEIGHT);
	printk("\t rt_sw_x_xy           0x%08x\n", DE->RT_SW_X_XY);
	printk("\t rt_sw_y_xy           0x%08x\n", DE->RT_SW_Y_XY);
	printk("\t rt_sw_x0             0x%08x\n", DE->RT_SW_X0);
	printk("\t rt_sw_y0             0x%08x\n", DE->RT_SW_Y0);
	printk("\t rt_sw_first_dist     0x%08x\n", DE->RT_SW_FIRST_DIST);
	printk("\t rt_r1m2              0x%08x\n", DE->RT_R1M2);
	printk("\t rt_r0m2              0x%08x\n", DE->RT_R0M2);
	printk("\t rt_fill_color        0x%08x\n", DE->RT_FILL_COLOR);
	printk("\t rt_result_x0         0x%08x\n", DE->RT_RESULT_X0);
	printk("\t rt_result_y0         0x%08x\n", DE->RT_RESULT_Y0);
	printk("\t rt_result_first_dist 0x%08x\n", DE->RT_RESULT_FIRST_DIST);
	printk("\t rt_result_src_addr   0x%08x\n", DE->RT_RESULT_SRC_ADDR);
}

#ifdef CONFIG_DISPLAY_ENGINE_GAMMA_LUT
/* gamma correction look-up table: First R0-255, then G0-255, last B0-255 */
static int de_config_gamma(const uint8_t *gamma_lut, int len)
{
	int i;

	if (len != 0x300) { /* 256 * 3 */
		return -EINVAL;
	}

	DE->GAT_CTL |= DE_GAMMA_AHB_GATING_EN;

	for (i = 0; i < len; i += 4) {
		DE->GAMMA_CTL = DE_GAMMA_RAM_INDEX(i / 4);
		DE->PATH_GAMMA_RAM = sys_get_le32(&gamma_lut[i]);
	}

	DE->GAT_CTL &= ~DE_GAMMA_AHB_GATING_EN;
	return 0;
}
#endif /* CONFIG_DISPLAY_ENGINE_GAMMA_LUT */

static int de_config_layer(int layer_idx, display_layer_t *ovl)
{
	static const uint32_t ly_argb_8888[] = { DE_CTL_L0_FORMAT_ARGB8888, DE_CTL_L1_FORMAT_ARGB8888 };
	static const uint32_t ly_rgb_565[]   = { DE_CTL_L0_FORMAT_RGB565,   DE_CTL_L1_FORMAT_RGB565 };
	static const uint32_t ly_bgr_565[]   = { DE_CTL_L0_FORMAT_BGR565,   DE_CTL_L1_FORMAT_BGR565 };
	static const uint32_t ly_rgb_565_le[] = { DE_CTL_L0_FORMAT_RGB565_SWAP, DE_CTL_L1_FORMAT_RGB565_SWAP };
	static const uint32_t ly_argb_6666[] = { DE_CTL_L0_FORMAT_ARGB6666, DE_CTL_L1_FORMAT_ARGB6666 };
	static const uint32_t ly_abgr_6666[] = { DE_CTL_L0_FORMAT_ABGR6666, DE_CTL_L1_FORMAT_ABGR6666 };
	static const uint32_t ly_rgb_888[]   = { DE_CTL_L0_FORMAT_BGR888,   DE_CTL_L1_FORMAT_BGR888 };
	static const uint32_t ly_ax[]        = { DE_CTL_L0_FORMAT_AX,       DE_CTL_L1_FORMAT_AX };

	const display_buffer_t *buffer = ovl->buffer;
	uint32_t format, addr;
	uint16_t bytes_per_line;
	uint8_t no_stride_en, halfword_en, bpp;

	if (buffer == NULL) {
		DE->LAYER_CTL[layer_idx].DEF_COLOR = ovl->color.full;

		/* HW Bug:
		 * de still reads the memory though not used, so just give it
		 * an invalid address which do not cause the bus halt.
		 */
		addr = DE_INVALID_ADDR;
		no_stride_en = 1;
		halfword_en = 0;
		format = DE_CTL_L1_FORMAT_RGB565 | DE_CTL_L1_COLOR_FILL_EN;
		bytes_per_line = ovl->frame.w * 2;
	} else {
		switch (buffer->desc.pixel_format) {
		case PIXEL_FORMAT_ARGB_8888:
			bpp = 32;
			format = ly_argb_8888[layer_idx];
			break;
		case PIXEL_FORMAT_RGB_565:
			bpp = 16;
			format = ly_rgb_565[layer_idx];
			break;
		case PIXEL_FORMAT_BGR_565:
			bpp = 16;
			format = ly_bgr_565[layer_idx];
			break;
		case PIXEL_FORMAT_ARGB_6666:
			bpp = 24;
			format = ly_argb_6666[layer_idx];
			break;
		case PIXEL_FORMAT_ABGR_6666:
			bpp = 24;
			format = ly_abgr_6666[layer_idx];
			break;
		case PIXEL_FORMAT_RGB_565_LE:
			bpp = 16;
			format = ly_rgb_565_le[layer_idx];
			break;
		case PIXEL_FORMAT_RGB_888:
			bpp = 24;
			format = ly_rgb_888[layer_idx];
			break;
		case PIXEL_FORMAT_A8:
			bpp = 8;
			format = ly_ax[layer_idx];
			DE->A148_COLOR = DE_TYPE_A8 | (ovl->color.full & 0xffffff);
			break;
		case PIXEL_FORMAT_A4_LE:
			bpp = 4;
			format = ly_ax[layer_idx];
			DE->A148_COLOR = DE_TYPE_A4 | (ovl->color.full & 0xffffff);
			DE->A14_CTL = DE_A14_FONT(2, 2, ovl->frame.w);
			break;
		case PIXEL_FORMAT_A1_LE:
			bpp = 1;
			format = ly_ax[layer_idx];
			DE->A148_COLOR = DE_TYPE_A1 | (ovl->color.full & 0xffffff);
			DE->A14_CTL = DE_A14_FONT(8, 8, ovl->frame.w);
			break;
		default:
			LOG_ERR("unsupported format %d\n", buffer->desc.pixel_format);
			return -EINVAL;
		}

		addr = buffer->addr;
		bytes_per_line = buffer->desc.pitch * bpp / 8;
		halfword_en = (addr & 0x3) ? 1 : 0;
		no_stride_en = !halfword_en && (buffer->desc.pitch == ovl->frame.w);

		/* Keep default value.
		 *
		 * DE->LAYER_CTL[layer_idx].COLOR_GAIN = DE_L_COLOR_GAIN(0x80, 0x80, 0x80);
		 * DE->LAYER_CTL[layer_idx].COLOR_OFFSET = DE_L_COLOR_GAIN(0, 0, 0);
		 */
	}

	if (layer_idx == 0) {
		DE->CTL |= DE_CTL_L0_EN | format | DE_CTL_L0_NO_STRIDE_EN(no_stride_en) | DE_CTL_L0_HALFWORD_EN(halfword_en);
	} else {
		DE->CTL |= DE_CTL_L1_EN | format | DE_CTL_L1_NO_STRIDE_EN(no_stride_en) | DE_CTL_L1_HALFWORD_EN(halfword_en);
	}

	DE->LAYER_CTL[layer_idx].ADDR = addr & ~0x3;
	DE->LAYER_CTL[layer_idx].POS = DE_L_POS(ovl->frame.x, ovl->frame.y);
	DE->LAYER_CTL[layer_idx].SIZE = DE_L_SIZE(ovl->frame.w, ovl->frame.h);
	DE->LAYER_CTL[layer_idx].STRIDE = bytes_per_line; /* only used by stride mode */
	DE->LAYER_CTL[layer_idx].LENGTH = bytes_per_line * ovl->frame.h; /* only used by no-stride mode */

	return 0;
}

static int de_apply_overlay_cfg(struct de_data *data, struct de_overlay_cfg *cfg, uint8_t cmd)
{
	display_buffer_t *target = &cfg->target;
	display_layer_t *ovls = cfg->ovls;
	int top_idx = cfg->num_ovls - 1;
	uint32_t target_format;
	uint16_t target_length_per_line;
	uint8_t target_bytes_per_pixel;

	sys_trace_u32x3(SYS_TRACE_ID_DE_DRAW, cmd, cfg->target_rect.w, cfg->target_rect.h);

	/* trigger mode */
	DE->CTL = DE_CTL_TRANSFER_MODE_TRIGGER;
	DE->GAT_CTL = DE_LAYER_GATING_EN | DE_OUTPUT_GATING_EN | DE_PATH_GATING_EN;

	if (target->addr > 0) {
		target_bytes_per_pixel = display_format_get_bits_per_pixel(target->desc.pixel_format) / 8;

		uint16_t bytes_per_line = target->desc.pitch * target_bytes_per_pixel;
		uint8_t no_stride_en = (target->desc.pitch == target->desc.width);

		DE->GAMMA_CTL = 0;
		DE->CTL |= DE_CTL_OUT_MODE_WB | DE_CTL_WB_NO_STRIDE_EN(no_stride_en);
		DE->WB_MEM_ADR = target->addr;
		DE->WB_MEM_STRIDE = bytes_per_line;

		target_format = target->desc.pixel_format;
	} else {
		DE->CTL |= DE_CTL_OUT_MODE_DISPLAY;
#ifdef CONFIG_DISPLAY_ENGINE_GAMMA_LUT
		DE->GAMMA_CTL = DE_GAMMA_EN;
#endif

		if (data->prepare_fn) {
			data->prepare_fn(data->prepare_fn_arg, &cfg->target_rect);
		}

		target_format = data->display_format;
		target_bytes_per_pixel = data->display_bytes_per_pixel;
	}

	switch (target_format) {
	case PIXEL_FORMAT_ARGB_8888:
		DE->CTL |= DE_CTL_OUT_FORMAT_RGB888_WB_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		DE->CTL |= DE_CTL_OUT_FORMAT_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
	default:
		DE->CTL |= DE_CTL_OUT_FORMAT_RGB565;
#if defined(CONFIG_DISPLAY_ENGINE_DITHER_4X4)
		DE->DITHER_CTL = DE_DITHER_EN | DE_DITHER_STRENGTH_4X4;
#elif defined(CONFIG_DISPLAY_ENGINE_DITHER_8X8)
		DE->DITHER_CTL = DE_DITHER_EN | DE_DITHER_STRENGTH_8X8;
#endif
		break;
	}

	/* set default bg */
	DE->BG_SIZE = DE_BG_SIZE(cfg->target_rect.w, cfg->target_rect.h);
	DE->BG_COLOR = 0;

	/* Hardware demand */
	target_length_per_line = target_bytes_per_pixel * cfg->target_rect.w;
	if (target_length_per_line >= 32 * 4) {
		DE->MEM_OPT = (DE->MEM_OPT & ~DE_MEM_BURST_LEN_MASK) | DE_MEM_BURST_32;
	} else if (target_length_per_line >= 16 * 4) {
		DE->MEM_OPT = (DE->MEM_OPT & ~DE_MEM_BURST_LEN_MASK) | DE_MEM_BURST_16;
	} else {
		DE->MEM_OPT = (DE->MEM_OPT & ~DE_MEM_BURST_LEN_MASK) | DE_MEM_BURST_8;
	}

	/* 1 ovl at least */
	assert(top_idx >= 0);

	/* 1 fill color layer */
	if (ovls[top_idx].buffer == NULL &&
		(top_idx == 0 || ovls[top_idx].blending == DISPLAY_BLENDING_NONE)) {
		DE->CTL |= DE_CTL_OUT_COLOR_FILL_EN;
		DE->FILL_COLOR = ovls[top_idx].color.full;
		DE->COLOR_FILL_POS = DE_COLOR_FILL_POS(
				ovls[top_idx].frame.x, ovls[top_idx].frame.y);
		DE->COLOR_FILL_SIZE = DE_COLOR_FILL_SIZE(
				ovls[top_idx].frame.w, ovls[top_idx].frame.h);

		if (--top_idx < 0)
			goto out;
	}

	/* 1 background layer */
	if (ovls[0].buffer == NULL) {
		DE->BG_COLOR = ovls[0].color.full;
		ovls++;
		if (--top_idx < 0)
			goto out;
	}

	/* 2 normal ovls */
	assert(top_idx < 2);

	DE->ALPHA_CTL = 0;

	if (top_idx > 0 && ovls[top_idx].blending != DISPLAY_BLENDING_NONE) {
		uint32_t alpha_mode = (ovls[top_idx].blending == DISPLAY_BLENDING_PREMULT) ?
				DE_ALPHA_PREMULTIPLIED : DE_ALPHA_COVERAGE;
		/* don't double use the color alpha */
		uint8_t alpha = ovls[top_idx].buffer ? ovls[top_idx].color.a : 255;
		display_rect_t alpha_rect;

		memcpy(&alpha_rect, &ovls[top_idx].frame, sizeof(alpha_rect));
		display_rect_intersect(&alpha_rect, &ovls[top_idx - 1].frame);

		if (display_rect_is_empty(&alpha_rect) == false) {
			DE->ALPHA_CTL = DE_ALPHA_EN | alpha_mode | DE_ALPHA_PLANE_ALPHA(alpha);
			DE->ALPHA_POS = DE_ALPHA_POS(alpha_rect.x, alpha_rect.y);
			DE->ALPHA_SIZE = DE_ALPHA_SIZE(alpha_rect.w, alpha_rect.h);
		}
	}

	for (; top_idx >= 0; top_idx--) {
		if (de_config_layer(top_idx, &ovls[top_idx]))
			break;
	}

	assert(top_idx == -1);

out:
	/* sequence:
	 * 1) modify configuration registers
	 * 2) modify REG_UD=1
	 * 3) modify EN=1
	 * 4) make sure EN is really 1 (read and compare with 1)
	 * 5) REG_UD becomes 0
	 * 6) hw START
	 **/
	DE->REG_UD = 1;
	DE->EN = 1;
	while (DE->EN == 0);

	k_work_schedule(&data->timeout_work,
			K_MSEC(CONFIG_DISPLAY_ENGINE_COMMAND_TIMEOUT_MS));

	/* however, set write back start */
	DE->CTL2 = 0x1F;//DE_CTL2_WB_START;

	if (target->addr == 0 && data->start_fn) {
		data->start_fn(data->start_fn_arg);
	}

	return 0;
}

static int de_apply_rotate_cfg(struct de_data *data, struct display_engine_rotation *cfg)
{
	uint32_t rt_ctl;

	sys_trace_u32x3(SYS_TRACE_ID_DE_DRAW, DE_CMD_ROTATE,
			cfg->outer_diameter, cfg->line_end - cfg->line_start);

	DE->GAT_CTL = DE_ROTATE_GATING_EN;
	DE->EN = 1;

	DE->RT_IMG_SIZE = RT_IMG_WIDTH(cfg->outer_diameter) | RT_END_HEIGHT(cfg->line_end);
	DE->RT_SRC_ADDR = cfg->src_address;
	DE->RT_SRC_STRIDE = cfg->src_pitch;
	DE->RT_DST_ADDR = cfg->dst_address;
	DE->RT_DST_STRIDE = cfg->dst_pitch;
	DE->RT_START_HEIGHT = cfg->line_start;
	DE->RT_SW_X_XY = RT_SW_DELTA_XY(cfg->src_coord_dx_ax, cfg->src_coord_dy_ax);
	DE->RT_SW_Y_XY = RT_SW_DELTA_XY(cfg->src_coord_dx_ay, cfg->src_coord_dy_ay);
	DE->RT_SW_X0 = cfg->src_coord_x;
	DE->RT_SW_Y0 = cfg->src_coord_y;
	DE->RT_SW_FIRST_DIST = cfg->dst_dist_sq;
	DE->RT_R1M2 = cfg->outer_radius_sq;
	DE->RT_R0M2 = cfg->inner_radius_sq;

	if (cfg->pixel_format == PIXEL_FORMAT_RGB_565) {
		DE->RT_FILL_COLOR = RT_COLOR_RGB_565(cfg->fill_color.r,
				cfg->fill_color.g, cfg->fill_color.b);
		rt_ctl = RT_EN | RT_IRQ_EN | RT_FILTER_BILINEAR | RT_FORMAT_RGB565 |
				(cfg->fill_enable ? RT_COLOR_FILL_EN : 0);
	} else {
		DE->RT_FILL_COLOR = cfg->fill_color.full;
		rt_ctl = RT_EN | RT_IRQ_EN | RT_FILTER_BILINEAR | RT_FORMAT_ARGB8888 |
				(cfg->fill_enable ? RT_COLOR_FILL_EN : 0);
	}

	k_work_schedule(&data->timeout_work,
			K_MSEC(CONFIG_DISPLAY_ENGINE_COMMAND_TIMEOUT_MS));

	DE->RT_CTL = rt_ctl;

	return 0;
}

#ifdef CONFIG_DISPLAY_ENGINE_COMPOSE_ON_VSYNC
static void de_display_vync_cb(const struct display_callback * callback, uint32_t timestamp)
{
	struct de_data *data = CONTAINER_OF(callback, struct de_data, display_cb);
	struct de_command_entry *entry;

	if (data->cmd_node) {
		entry = CONTAINER_OF(data->cmd_node, struct de_command_entry, node);
		if (entry->cmd == DE_CMD_COMPOSE_VSYNC) {
			de_apply_overlay_cfg(data, &entry->ovl_cfg, entry->cmd);
		}
	}
}
#endif /* CONFIG_DISPLAY_ENGINE_COMPOSE_ON_VSYNC */

#ifdef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
static void de_process_next_cmd(struct de_data *data)
#else
static void de_apply_work_handler(struct k_work *work)
#endif
{
	struct de_command_entry *entry;
	sys_snode_t *node = NULL;

#ifndef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
	struct de_data *data = CONTAINER_OF(work, struct de_data, apply_work);
	k_mutex_lock(&data->mutex, K_FOREVER);
#endif

#ifdef CONFIG_DISPLAY_ENGINE_HIHG_PRIO_INSTANCE
	if (data->cmd_node) {
		node = sys_slist_peek_head(&data->high_cmd_list);
		if (node == data->cmd_node) {
			sys_slist_remove(&data->high_cmd_list, NULL, data->cmd_node);
		} else {
			assert(data->cmd_node == sys_slist_peek_head(&data->cmd_list));
			sys_slist_remove(&data->cmd_list, NULL, data->cmd_node);
		}

		data->cmd_num--;
	}

	node = sys_slist_peek_head(&data->high_cmd_list);
	if (!node) {
		node = sys_slist_peek_head(&data->cmd_list);
	}
#else /* CONFIG_DISPLAY_ENGINE_HIHG_PRIO_INSTANCE */
	if (data->cmd_node) {
		assert(data->cmd_node == sys_slist_peek_head(&data->cmd_list));
		sys_slist_remove(&data->cmd_list, NULL, data->cmd_node);

		data->cmd_num--;
	}

	node = sys_slist_peek_head(&data->cmd_list);
#endif /* CONFIG_DISPLAY_ENGINE_HIHG_PRIO_INSTANCE */

#ifndef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
	k_mutex_unlock(&data->mutex);
#endif

	/* free command entry */
	if (data->cmd_node) {
		entry = CONTAINER_OF(data->cmd_node, struct de_command_entry, node);
		de_instance_notify(entry, data->cmd_status);
		de_instance_free_entry(entry);

		data->cmd_node = NULL;
	}

	if (node) {
		entry = CONTAINER_OF(node, struct de_command_entry, node);

		switch (entry->cmd) {
		case DE_CMD_ROTATE:
			de_apply_rotate_cfg(data, &entry->rot_cfg);
			break;
#ifdef CONFIG_DISPLAY_ENGINE_COMPOSE_ON_VSYNC
		case DE_CMD_COMPOSE_VSYNC:
			break;
#endif
		default:
			de_apply_overlay_cfg(data, &entry->ovl_cfg, entry->cmd);
			break;
		}

		data->cmd_node = node;
	}
}

static void de_prepare_next_cmd(struct de_data *data)
{
#ifdef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
	de_process_next_cmd(data);
#else
	k_work_submit(&data->apply_work);
#endif
}

static void de_complete_cmd(struct de_data *data, int status)
{
	sys_trace_end_call(SYS_TRACE_ID_DE_DRAW);

	DE->EN = 0;
	DE->RT_CTL = RT_STAT_COMPLETE;
	DE->GAT_CTL = 0;

	data->cmd_status = status;
	de_prepare_next_cmd(data);
}

static void de_append_cmd(struct de_data *data, struct de_command_entry *entry, bool high_prio)
{
#ifdef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
	unsigned int key = irq_lock();
#else
	k_mutex_lock(&data->mutex, K_FOREVER);
#endif

#ifdef CONFIG_DISPLAY_ENGINE_HIHG_PRIO_INSTANCE
	if (high_prio) {
		sys_slist_append(&data->high_cmd_list, &entry->node);
	} else {
		sys_slist_append(&data->cmd_list, &entry->node);
	}
#else
	sys_slist_append(&data->cmd_list, &entry->node);
#endif

	if (++data->cmd_num == 1) {
#ifdef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
		de_process_next_cmd(data);
#else
		k_work_submit(&data->apply_work);
#endif
	}

#ifdef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
	irq_unlock(key);
#else
	k_mutex_unlock(&data->mutex);
#endif
}

static int de_insert_overlay_cmd(const struct device *dev, int inst,
		display_buffer_t *target, const display_layer_t *ovls,
		uint8_t num_ovls, uint8_t cmd)
{
	struct de_data *data = dev->data;
	struct de_command_entry *entry;
	struct display_rect dst_rect;
	uint32_t dst_addr = 0;
	uint16_t bytes_per_line;
	uint8_t bpp, halfword_en;
	uint8_t has_ax_format = 0;
	int i;

	assert(num_ovls <= 4);
	assert(num_ovls < 3 || ovls[1].buffer != NULL);
	assert(num_ovls < 4 || ovls[2].buffer != NULL);

	/* validate layer parameters */
	for (int i = 0; i < num_ovls; i++) {
		if (!ovls[i].buffer) {
			continue;
		}

		if ((ovls[i].buffer->desc.pixel_format & SUPPORTED_INPUT_PIXEL_FORMATS) == 0) {
			LOG_WRN("L%d format %d unsupported", i, ovls[i].buffer->desc.pixel_format);
			return -EINVAL;
		}

		if (ovls[i].buffer->desc.pixel_format & (PIXEL_FORMAT_A8 | PIXEL_FORMAT_A4_LE | PIXEL_FORMAT_A1_LE)) {
			if (has_ax_format) {
				LOG_WRN("L%d only one layer can be Ax", i);
				return -EINVAL;
			}

			if (ovls[i].buffer->desc.pixel_format == PIXEL_FORMAT_A4_LE && (ovls[i].frame.w & 0x1)) {
				LOG_WRN("L%d frame w of A4 must be mutiple of 2", i);
				return -EINVAL;
			}

			if (ovls[i].buffer->desc.pixel_format == PIXEL_FORMAT_A1_LE && (ovls[i].frame.w & 0x7)) {
				LOG_WRN("L%d frame w of A1 must be mutiple of 8", i);
				return -EINVAL;
			}

			has_ax_format = 1;
		}

		if (ovls[i].buffer->desc.width != ovls[i].frame.w ||
			ovls[i].buffer->desc.height != ovls[i].frame.h) {
			LOG_WRN("L%d scaling (%dx%d->%dx%d) unsupported",
					i, ovls[i].buffer->desc.width, ovls[i].buffer->desc.height,
					ovls[i].frame.w, ovls[i].frame.h);
			return -EINVAL;
		}

		bytes_per_line = display_buffer_get_bytes_per_line(ovls[i].buffer);

		/* Only rgb565 support half word aligned.
		 * In this case, hardware implemention assumes the origin image is 4-byte aligned,
		 * and then cropped (1, 0, xx, xx) which lead to the half-word aligned address.
		 */
		halfword_en = (ovls[i].buffer->addr & 0x3) ? 1 : 0;
		if (halfword_en > 0) {
			if ((ovls[i].buffer->desc.pixel_format != PIXEL_FORMAT_RGB_565 &&
				 ovls[i].buffer->desc.pixel_format != PIXEL_FORMAT_BGR_565)) {
				LOG_WRN("L%d address 0x%x unaligned", i, ovls[i].buffer->addr);
				return -EINVAL;
			}
		}

		if (ovls[i].buffer->desc.pitch < ovls[i].buffer->desc.width + halfword_en) {
			LOG_DBG("L%d width %u should not greater than pitch %u (hwa=%u)", i,
					ovls[i].buffer->desc.width, ovls[i].buffer->desc.pitch, halfword_en);
		}

		if ((bytes_per_line & 0x3) && (halfword_en || ovls[i].buffer->desc.pitch != ovls[i].buffer->desc.width)) {
			LOG_WRN("L%d pitch %u unaligned (hwa=%u)", i, ovls[i].buffer->desc.pitch, halfword_en);
			return -EINVAL;
		}
	}

	/* compute target area */
	memcpy(&dst_rect, &ovls[0].frame, sizeof(dst_rect));
	for (i = 1; i < num_ovls; i++) {
		display_rect_merge(&dst_rect, &ovls[i].frame);
	}

	/* validate target parameters */
	if (target) {
		uint8_t min_w = 3;

		if ((target->desc.pixel_format & SUPPORTED_OUTPUT_PIXEL_FORMATS) == 0) {
			LOG_WRN("target format 0x%x unsupported", target->desc.pixel_format);
			return -EINVAL;
		}

		if (target->desc.pitch < target->desc.width) {
			LOG_WRN("target width %u should not greater than pitch %u",
					target->desc.width, target->desc.pitch);
			return -EINVAL;
		}

		bpp = display_format_get_bits_per_pixel(target->desc.pixel_format);
		bytes_per_line = target->desc.pitch * bpp / 8;
		dst_addr = target->addr + dst_rect.y * bytes_per_line + dst_rect.x * bpp / 8;
		if (buf_is_psram(dst_addr)) {
			dst_addr = (uint32_t)cache_to_uncache((void *)dst_addr);
			min_w = (target->desc.pixel_format != PIXEL_FORMAT_RGB_565) ? 4 : 8;
		}

		if (display_rect_get_width(&dst_rect) < min_w) {
			LOG_WRN("bg width less than %d", min_w);
			return -EINVAL;
		}

		/* Only rgb565 support half word aligned.
		 * In this case, hardware implemention assumes the origin target image is 4-byte aligned,
		 * and writing to the area (1, 0, xx, xx) which lead to the half-word aligned address.
		 */
		halfword_en = (dst_addr & 0x3) ? 1 : 0;
		if (target->desc.pixel_format != PIXEL_FORMAT_RGB_565) {
			if (halfword_en > 0) {
				 LOG_WRN("target address 0x%x unaligned", dst_addr);
				return -EINVAL;
			}

			if ((bytes_per_line & 0x3) && target->desc.pitch != target->desc.width) {
				LOG_WRN("target pitch %u unaligned (hwa=%u)", target->desc.pitch, halfword_en);
				return -EINVAL;
			}
		} else if (!(target->desc.width & 0x1)) {
			if (halfword_en || (target->desc.pitch != target->desc.width && (bytes_per_line & 0x3))) {
				LOG_WRN("target address 0x%x unaligned (pitch %u, width %u)",
						dst_addr, target->desc.pitch, target->desc.width);
				return -EINVAL;
			}
		}

		/* HW Bug: SPI-Controller only support word aligned access from DE */
		if (buf_is_psram_un(dst_addr)) {
			uint16_t bytes_to_copy = (target->desc.width * bpp / 8);

			if (halfword_en || (bytes_to_copy & 0x3) || (bytes_per_line & 0x3)) {
				LOG_WRN("target width %u & pitch %u not aligned for psram (hwa=%u)",
						target->desc.width, bytes_per_line, halfword_en);
				return -EINVAL;
			}
		}
	} else {
		if (data->display_format == 0) {
			LOG_WRN("display mode not configured");
			return -EINVAL;
		}

		if (display_rect_get_width(&dst_rect) < 2) {
			LOG_WRN("bg width less than 2");
			return -EINVAL;
		}
	}

	entry = de_instance_alloc_entry(inst);
	if (!entry)
		return -EBUSY;

	entry->cmd = cmd;
	entry->ovl_cfg.num_ovls = num_ovls;
	memcpy(entry->ovl_cfg.ovls, ovls, num_ovls * sizeof(*ovls));
	memcpy(&entry->ovl_cfg.target_rect, &dst_rect, sizeof(dst_rect));

	for (i = 0; i < num_ovls; i++) {
		if (ovls[i].buffer) {
			entry->ovl_cfg.ovls[i].buffer = &entry->ovl_cfg.bufs[i];
			memcpy(&entry->ovl_cfg.bufs[i], ovls[i].buffer, sizeof(*target));

			if (buf_is_psram(ovls[i].buffer->addr)) {
				entry->ovl_cfg.bufs[i].addr =
					(uint32_t)cache_to_uncache((void *)ovls[i].buffer->addr);
			}
		}

		display_rect_move(&entry->ovl_cfg.ovls[i].frame, -dst_rect.x, -dst_rect.y);
	}

	if (target) {
		entry->ovl_cfg.target.desc.pixel_format = target->desc.pixel_format;
		entry->ovl_cfg.target.desc.pitch = target->desc.pitch;
		entry->ovl_cfg.target.desc.width = dst_rect.w;
		entry->ovl_cfg.target.desc.height = dst_rect.h;
		entry->ovl_cfg.target.addr = dst_addr;
	} else {
		entry->ovl_cfg.target.addr = 0;
	}

	de_append_cmd(data, entry, de_instance_has_flag(inst, DISPLAY_ENGINE_FLAG_HIGH_PRIO));
	return entry->seq;
}

static int de_fill(const struct device *dev, int inst,
		display_buffer_t *dest, display_color_t color)
{
	display_layer_t layer = {
		.buffer = NULL,
		.color = color,
		.frame = { 0, 0, dest->desc.width, dest->desc.height },
	};

	return de_insert_overlay_cmd(dev, inst, dest, &layer, 1, DE_CMD_FILL);
}

static int de_blit(const struct device *dev,
		int inst, display_buffer_t *dest,
		display_buffer_t *src, display_color_t src_color)
{
	display_layer_t layer = {
		.buffer = src,
		.color = src_color,
		.frame = { 0, 0, dest->desc.width, dest->desc.height },
	};

	if (dest->desc.width != src->desc.width || dest->desc.height != src->desc.height)
		return -EINVAL;

	return de_insert_overlay_cmd(dev, inst, dest, &layer, 1, DE_CMD_BLIT);
}

static inline int de_blend(const struct device *dev,
		int inst, display_buffer_t *dest,
		display_buffer_t *fg, display_color_t fg_color,
		display_buffer_t *bg, display_color_t bg_color)
{
	display_layer_t ovls[2] = {
		{
			.buffer = bg,
			.color = bg_color,
			.frame = { 0, 0, dest->desc.width, dest->desc.height },
		},
		{
			.buffer = fg,
			.color = fg_color,
			.blending = DISPLAY_BLENDING_COVERAGE,
			.frame = { 0, 0, dest->desc.width, dest->desc.height },
		},
	};

	if (bg == NULL || dest->desc.width != bg->desc.width || dest->desc.height != bg->desc.height) {
		return -EINVAL;
	}

	if (fg != NULL && (dest->desc.width != fg->desc.width || dest->desc.height != fg->desc.height)) {
		return -EINVAL;
	}

	if (fg != NULL && display_format_is_opaque(fg->desc.pixel_format) && fg_color.a == 255) {
		ovls[1].blending = DISPLAY_BLENDING_NONE;
	}

	return de_insert_overlay_cmd(dev, inst, dest, ovls, 2, fg ? DE_CMD_BLEND : DE_CMD_BLEND_FG);
}

static int de_compose(const struct device *dev, int inst,
		display_buffer_t *target, const display_layer_t *ovls,
		int num_ovls, bool wait_vsync)
{
	uint8_t cmd = target ? DE_CMD_COMPOSE_WB :
			DE_CMD_COMPOSE;

	if (num_ovls > MAX_NUM_OVERLAYS || num_ovls <= 0) {
		LOG_WRN("unsupported ovl num %d", num_ovls);
		return -EINVAL;
	}

#ifdef CONFIG_DISPLAY_ENGINE_COMPOSE_ON_VSYNC
	if (target == NULL && wait_vsync) {
		cmd = DE_CMD_COMPOSE_VSYNC;
	}
#endif

	return de_insert_overlay_cmd(dev, inst, target, ovls, num_ovls, cmd);
}

static int de_rotate(const struct device *dev,
		int inst, display_engine_rotation_t *rot_cfg)
{
	struct de_data *data = dev->data;
	struct de_command_entry *entry;

	if ((rot_cfg->pixel_format & SUPPORTED_ROTATE_PIXEL_FORMATS) == 0) {
		LOG_WRN("unsupported rotation format %u", rot_cfg->pixel_format);
		return -EINVAL;
	}

	if (rot_cfg->line_start >= rot_cfg->line_end ||
		rot_cfg->line_end > rot_cfg->outer_diameter) {
		return -EINVAL;
	}

	entry = de_instance_alloc_entry(inst);
	if (!entry)
		return -EBUSY;

	entry->cmd = DE_CMD_ROTATE;
	memcpy(&entry->rot_cfg, rot_cfg, sizeof(*rot_cfg));

	de_append_cmd(data, entry, de_instance_has_flag(inst, DISPLAY_ENGINE_FLAG_HIGH_PRIO));

	return entry->seq;
}

static int de_poll(const struct device *dev, int inst, int timeout_ms)
{
	return de_instance_poll(inst, timeout_ms);
}

static int de_register_callback(const struct device *dev,
		int inst, display_engine_instance_callback_t callback, void *user_data)
{
	struct de_data *data = dev->data;
	int res;

	k_mutex_lock(&data->mutex, K_FOREVER);
	res = de_instance_register_callback(inst, callback, user_data);
	k_mutex_unlock(&data->mutex);

	return res;
}

static void de_get_capabilities(const struct device *dev,
		struct display_engine_capabilities *capabilities)
{
	capabilities->num_overlays = MAX_NUM_OVERLAYS;
	capabilities->support_blend_fg = 1;
	capabilities->support_blend_bg = 0;
	capabilities->supported_output_pixel_formats = SUPPORTED_OUTPUT_PIXEL_FORMATS;
	capabilities->supported_input_pixel_formats = SUPPORTED_INPUT_PIXEL_FORMATS;
	capabilities->supported_rotate_pixel_formats = SUPPORTED_ROTATE_PIXEL_FORMATS;
}

static int de_control(const struct device *dev, int cmd, void *arg1, void *arg2)
{
	struct de_data *data = dev->data;

	switch (cmd) {
	case DISPLAY_ENGINE_CTRL_DISPLAY_PREPARE_CB:
		data->prepare_fn_arg = arg2;
		data->prepare_fn = arg1;
#ifdef CONFIG_DISPLAY_ENGINE_COMPOSE_ON_VSYNC
		/* arg2 must point to the display device structure */
		display_register_callback(arg2, &data->display_cb);
#endif
		break;
	case  DISPLAY_ENGINE_CTRL_DISPLAY_START_CB:
		data->start_fn_arg = arg2;
		data->start_fn = arg1;
		break;
	case DISPLAY_ENGINE_CTRL_DISPLAY_MODE:
		data->display_format = ((struct display_videomode *)arg1)->pixel_format;
		data->display_bytes_per_pixel = display_format_get_bits_per_pixel(data->display_format) / 8;
		break;
	case DISPLAY_ENGINE_CTRL_DISPLAY_PORT:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

void de_isr(const void *arg)
{
	const struct device *dev = arg;
	struct de_data *data = dev->data;
	uint32_t status = DE->STA;
	uint32_t rt_stat = DE->RT_CTL;
	bool completed = false;

	if (rt_stat & RT_STAT_COMPLETE) {
		LOG_DBG("de rt complete 0x%08x", rt_stat);
		completed = true;
	}

	if (status & (DE_STAT_WB_FTC | DE_STAT_DEV_FTC)) {
		LOG_DBG("de ovl complete 0x%08x", status);
		completed = true;
	}

	if (completed) {
		k_work_cancel_delayable(&data->timeout_work);
		de_complete_cmd(data, 0);
	}

	if (status & DE_STAT_DEV_FIFO_HF) {
		LOG_DBG("de halfull");
	}

	/* RGB and SPI_QUAD_SYNC have vsync signal*/
	if (status & DE_STAT_DEV_VSYNC) {
		/* TODO: refresh frames */
		LOG_DBG("vsync arrived");
	}

	/* update frames for those do not have vsync signal */
	if (status & DE_STAT_PRELINE) {
		/* TODO: refresh frames */
		LOG_DBG("preline arrived");
	}

	DE->STA = status;
}

static void de_timeout_work_handler(struct k_work *work)
{
	struct de_data *data = CONTAINER_OF(work, struct de_data, timeout_work);

	printk("de timeout\n");
	de_dump();

	de_complete_cmd(data, -ETIME);
}

int de_init(const struct device *dev)
{
	struct de_data *data = dev->data;

	/* set invalid value */
	data->display_format = 0;

	k_mutex_init(&data->mutex);
	sys_slist_init(&data->cmd_list);
#ifdef CONFIG_DISPLAY_ENGINE_HIHG_PRIO_INSTANCE
	sys_slist_init(&data->high_cmd_list);
#endif

#ifndef CONFIG_DISPLAY_ENGINE_PROCESS_COMMAND_IN_ISR
	k_work_init(&data->apply_work, de_apply_work_handler);
#endif

	k_work_init_delayable(&data->timeout_work, de_timeout_work_handler);

#ifdef CONFIG_DISPLAY_ENGINE_COMPOSE_ON_VSYNC
	data->display_cb.vsync = de_display_vync_cb;
	data->display_cb.complete = NULL;
#endif

	de_command_pools_init();
	return 0;
}

#ifdef CONFIG_PM_DEVICE
int de_pm_control(const struct device *dev, enum pm_device_action action)
{
	int ret = 0;

	switch (action) {
	case PM_DEVICE_ACTION_SUSPEND:
	case PM_DEVICE_ACTION_FORCE_SUSPEND:
	case PM_DEVICE_ACTION_TURN_OFF:
		ret = (DE->EN != 0) ? -EBUSY : 0;
		break;
	case PM_DEVICE_ACTION_RESUME:
		de_reset();
		break;
	default:
		break;
	}

	return ret;
}
#endif /* CONFIG_PM_DEVICE */

const struct display_engine_driver_api de_drv_api = {
	.control = de_control,
	.open = de_open,
	.close = de_close,
	.get_capabilities = de_get_capabilities,
	.register_callback = de_register_callback,
	.fill = de_fill,
	.blit = de_blit,
	.blend = de_blend,
	.compose = de_compose,
	.rotate = de_rotate,
	.poll = de_poll,
};

struct de_data de_drv_data;
