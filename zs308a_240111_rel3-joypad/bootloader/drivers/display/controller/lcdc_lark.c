/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <spicache.h>
#include <drivers/cfg_drv/dev_config.h>
#include <drivers/display.h>
#include <drivers/display/display_engine.h>
#include <drivers/display/display_controller.h>
#include <assert.h>
#include <string.h>

#include "lcdc_lark.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(lcdc, CONFIG_DISPLAY_LOG_LEVEL);

#define USE_LCDC_TE 0
#define LCDC_HAS_TE_INT 0

#define LCDC     ((LCDC_Type *)LCDC_REG_BASE)
#define DMACHAN  ((LCDC_DMACHAN_CTL_Type *)(DMA_REG_BASE + 0x100 + CONFIG_LCDC_DMA_CHAN_ID * 0x100))
#define DMALINE  ((LCDC_DMALINE_CTL_Type *)(DMA_LINE0_REG_BASE + 0x20 * CONFIG_LCDC_DMA_LINE_ID))

struct lcdc_data {
	/* pointer to current video port passed in interface enable() */
	const struct display_videoport *port;
	/* pointer to current video mode passed in interface set_mode() */
	const struct display_videomode *mode;
	/* enum display_controller_source_type */
	uint8_t source_type;

	display_controller_complete_t complete_fn;
	void *complete_fn_arg;

#if LCDC_HAS_TE_INT
	display_controller_vsync_t vsync_fn;
	void *vsync_fn_arg;
#endif
};

static int lcdc_config_port(const struct device *dev, const struct display_videoport *port)
{
	uint32_t lcd_ctl = LCD_EN | LCD_CLK_EN | LCD_HOLD_OPT;

	switch (port->major_type) {
	case DISPLAY_PORT_CPU:
		lcd_ctl |= (port->minor_type == DISPLAY_CPU_I8080) ?
				LCD_IF_SEL_CPU_I8080 : LCD_IF_SEL_CPU_M6880;
		lcd_ctl |= LCD_IF_CE_SEL(port->cpu_mode.cs) | LCD_IF_MLS_SEL(port->cpu_mode.lsb_first);
		LCDC->CPU_CTL = LCD_CPU_AHB_F565_16BIT | LCD_CPU_AHB_CSX(1);
		LCDC->CPU_CLK = LCD_CPU_CLK(port->cpu_mode.pclk_high_duration,
				port->cpu_mode.pclk_low_duration, port->cpu_mode.pclk_low_duration);
		break;

	case DISPLAY_PORT_SPI:
		lcd_ctl |= LCD_IF_SEL_SPI;
		lcd_ctl |= LCD_IF_CE_SEL(port->spi_mode.cs) | LCD_IF_MLS_SEL(port->spi_mode.lsb_first);
		LCDC->SPI_CTL = LCD_SPI_CDX(1) | LCD_SPI_AHB_CSX(1) | LCD_SPI_AHB_F565_16BIT |
				LCD_SPI_TYPE_SEL(port->minor_type) |
				LCD_SPI_SCLK_POL((port->spi_mode.cpol == port->spi_mode.cpha) ? 0 : 1) |
				LCD_SPI_DELAY_CHAIN_SEL(port->spi_mode.rx_delay_chain_ns) |
				LCD_SPI_DCP_SEL(port->spi_mode.dcp_mode) |
				LCD_SPI_DUAL_LANE_SEL(port->spi_mode.dual_lane);
		break;

	default:
		return -ENOTSUP;
	}

	LCDC->CTL = lcd_ctl;

	return 0;
}

static int lcdc_config_dma_pixel_format(const struct device *dev,
		uint32_t pixel_format, uint8_t *bytes_per_pixel)
{
	struct lcdc_data *data = dev->data;
	uint8_t bytes = 2;

	if (data->port->major_type == DISPLAY_PORT_SPI) {
		LCDC->SPI_CTL &= ~LCD_SPI_SDT_MASK;

		switch (pixel_format) {
		case PIXEL_FORMAT_RGB_565:
			LCDC->SPI_CTL |= LCD_SPI_SDT_RGB565;
			break;
		case PIXEL_FORMAT_BGR_565:
			LCDC->SPI_CTL |= LCD_SPI_SDT_BGR565;
			break;
		case PIXEL_FORMAT_ARGB_8888:
			LCDC->SPI_CTL |= LCD_SPI_SDT_ARGB8888;
			bytes = 4;
			break;
		case PIXEL_FORMAT_RGB_888:
			LCDC->SPI_CTL |= LCD_SPI_SDT_RGB888;
			bytes = 3;
			break;
		case PIXEL_FORMAT_ARGB_6666:
			LCDC->SPI_CTL |= LCD_SPI_SDT_RGB666;
			bytes = 3;
			break;
		default:
			return -EINVAL;
		}
	} else if (data->port->major_type == DISPLAY_PORT_CPU) {
		LCDC->CPU_CTL &= ~LCD_CPU_SDT_MASK;

		switch (pixel_format) {
		case PIXEL_FORMAT_RGB_565:
			LCDC->CPU_CTL |= LCD_CPU_SDT_RGB565;
			break;
		case PIXEL_FORMAT_BGR_565:
			LCDC->CPU_CTL |= LCD_CPU_SDT_BGR565;
			break;
		case PIXEL_FORMAT_ARGB_8888:
			LCDC->CPU_CTL |= LCD_CPU_SDT_ARGB8888;
			bytes = 4;
			break;
		case PIXEL_FORMAT_RGB_888:
			LCDC->CPU_CTL |= LCD_CPU_SDT_RGB888;
			bytes = 3;
			break;
		case PIXEL_FORMAT_ARGB_6666:
			LCDC->CPU_CTL |= LCD_CPU_SDT_RGB666;
			bytes = 3;
			break;
		default:
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}

	if (bytes_per_pixel) {
		*bytes_per_pixel = bytes;
	}

	return 0;
}

static int lcdc_config_mode(const struct device *dev, const struct display_videomode *mode)
{
	clk_set_rate(CLOCK_ID_LCD, KHZ(mode->pixel_clk));

	LCDC->CTL &= ~(LCD_TE_EN | LCD_TE_MODE_MASK | LCD_IF_FORMAT_MASK);

#if USE_LCDC_TE
	if (mode->flags & DISPLAY_FLAGS_TE_HIGH) {
		LCDC->CTL |= LCD_TE_EN | LCD_TE_MODE_RISING_EDGE;
	} else if (mode->flags & DISPLAY_FLAGS_TE_LOW) {
		LCDC->CTL |= LCD_TE_EN | LCD_TE_MODE_FALLING_EDGE;
	}
#endif

	switch (mode->pixel_format) {
	case PIXEL_FORMAT_RGB_565:
		LCDC->CTL |= LCD_IF_FORMAT_RGB565;
		break;
	case PIXEL_FORMAT_BGR_565:
		LCDC->CTL |= LCD_IF_FORMAT_RGB565;
		break;
	case PIXEL_FORMAT_ARGB_8888:
		LCDC->CTL |= LCD_IF_FORMAT_RGB888;
		break;
	case PIXEL_FORMAT_RGB_888:
		LCDC->CTL |= LCD_IF_FORMAT_RGB888;
		break;
	case PIXEL_FORMAT_ARGB_6666:
		LCDC->CTL |= LCD_IF_FORMAT_RGB666;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int lcdc_config_source(const struct device *dev,
		enum display_controller_source_type source_type)
{
	struct lcdc_data *data = dev->data;

#if CONFIG_LCDC_DMA_CHAN_ID < 0
	if (source_type == DISPLAY_CONTROLLER_SOURCE_DMA)
		return -EINVAL;
#endif

	switch (data->port->major_type) {
	case DISPLAY_PORT_CPU:
		LCDC->CPU_CTL &= ~(LCD_CPU_INPUT_SRC_MASK | LCD_CPU_RS_MASK | LCD_CPU_HWA_EN);

		switch (source_type) {
		case DISPLAY_CONTROLLER_SOURCE_ENGINE:
			LCDC->CPU_CTL |= LCD_CPU_INPUT_SRC_SEL_DE | LCD_CPU_FEND_IRQ_EN | LCD_CPU_RS_HIGH;
			break;
		case DISPLAY_CONTROLLER_SOURCE_DMA:
			LCDC->CPU_CTL |= LCD_CPU_INPUT_SRC_SEL_DMA | LCD_CPU_FEND_IRQ_EN | LCD_CPU_RS_HIGH;
			break;
		case DISPLAY_CONTROLLER_SOURCE_MCU:
			LCDC->CPU_CTL |= LCD_CPU_INPUT_SRC_SEL_AHB;
			break;
		default:
			return -EINVAL;
		}
		break;

	case DISPLAY_PORT_SPI:
		LCDC->SPI_CTL &= ~(LCD_SPI_INPUT_SRC_MASK | LCD_SPI_CDX_MASK | LCD_SPI_HWA_EN);

		switch (source_type) {
		case DISPLAY_CONTROLLER_SOURCE_ENGINE:
			LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_DE | LCD_SPI_FTC_IRQ_EN | LCD_SPI_CDX(1);
			break;
		case DISPLAY_CONTROLLER_SOURCE_DMA:
			LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_DMA | LCD_SPI_FTC_IRQ_EN | LCD_SPI_CDX(1);
			break;
		case DISPLAY_CONTROLLER_SOURCE_MCU:
			LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_AHB | LCD_SPI_CDX(1);
			break;
		default:
			return -EINVAL;
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int lcdc_enable(const struct device *dev, const struct display_videoport *port)
{
	struct lcdc_data *data = dev->data;

	if (port == NULL) {
		return -EINVAL;
	}

	/* initially use the CMU_LCDCLK default setting: HOSC 1/1 */
	//clk_set_rate(CLOCK_ID_LCD, MHZ(50));
	acts_reset_peripheral_assert(RESET_ID_LCD);
	acts_clock_peripheral_enable(CLOCK_ID_LCD);
	acts_reset_peripheral_deassert(RESET_ID_LCD);

	if (lcdc_config_port(dev, port)) {
		return -EINVAL;
	}

	/* FIXME: just save the pointer ? */
	data->port = port;
	data->mode = NULL;
	data->source_type = 0;

	return 0;
}

static int lcdc_disable(const struct device *dev)
{
	acts_clock_peripheral_disable(CLOCK_ID_LCD);
	return 0;
}

static int lcdc_set_mode(const struct device *dev,
		const struct display_videomode *mode)
{
	struct lcdc_data *data = dev->data;

	if (mode == NULL) {
		return -EINVAL;
	}

	if (lcdc_config_mode(dev, mode)) {
		return -EINVAL;
	}

	/* FIXME: just save the pointer ? */
	data->mode = mode;

	return 0;
}

static int lcdc_set_source(const struct device *dev,
		enum display_controller_source_type source_type, const struct device *source_dev)
{
	struct lcdc_data *data = dev->data;

	if (data->port == NULL) {
		return -EINVAL;
	}

	if (source_type == data->source_type) {
		return 0;
	}

	if (lcdc_config_source(dev, source_type)) {
		return -EINVAL;
	}

	data->source_type = source_type;
	return 0;
}

#define SPI_AHB_CMD_MASK  (LCD_SPI_INPUT_SRC_MASK | LCD_SPI_AHB_CSX_MASK | LCD_SPI_CDX_MASK)
#define SPI_AHB_DATA_MASK (LCD_SPI_INPUT_SRC_MASK | LCD_SPI_AHB_F565_MASK | LCD_SPI_AHB_CSX_MASK | LCD_SPI_CDX_MASK | LCD_SPI_AHB_CFG_DATA | LCD_SPI_RWL(0X3f) | LCD_SPI_RDLC_MASK)

#define CPU_AHB_CMD_MASK (LCD_CPU_INPUT_SRC_MASK | LCD_CPU_AHB_F565_MASK | LCD_CPU_RS_MASK | LCD_CPU_AHB_DATA_MASK | LCD_CPU_AHB_CSX_MASK)
#define CPU_AHB_DATA_MASK CPU_AHB_CMD_MASK

static int lcdc_read_config(const struct device *dev,
		int32_t cmd, void *buf, uint32_t len)
{
	struct lcdc_data *lcdc_data = dev->data;

	if (lcdc_data->port == NULL) {
		return -EINVAL;
	}

	if (lcdc_data->port->major_type == DISPLAY_PORT_SPI) {

	} else if (lcdc_data->port->major_type == DISPLAY_PORT_CPU) {
		uint32_t lcdc_cpu_ctl = LCDC->CPU_CTL; /* save REG CPU_CTL */

		LCDC->CPU_CTL &= ~CPU_AHB_CMD_MASK;
		LCDC->CPU_CTL |= LCD_CPU_AHB_DATA_SEL_CFG | LCD_CPU_RS_LOW;
		LCDC->DATA = cmd;
		LCDC->CPU_CTL |= LCD_CPU_AHB_CSX(1);

		LCDC->CPU_CTL &= ~CPU_AHB_CMD_MASK;
		LCDC->CPU_CTL |= LCD_CPU_AHB_DATA_SEL_CFG | LCD_CPU_RS_HIGH;

		do {
			*(uint8_t *)buf = LCDC->DATA & 0xff;
			buf = (uint8_t *)buf + 1;
		} while (--len > 0);

		LCDC->CPU_CTL |= LCD_CPU_AHB_CSX(1);

		/* recovery REG CPU_CTL */
		LCDC->CPU_CTL = lcdc_cpu_ctl;
	}

	return 0;
}

static int lcdc_write_config(const struct device *dev,
		int32_t cmd, const void *buf, uint32_t len)
{
	struct lcdc_data *lcdc_data = dev->data;

	if (lcdc_data->port == NULL) {
		return -EINVAL;
	}

	if (lcdc_data->port->major_type == DISPLAY_PORT_SPI) {
		uint32_t lcdc_spi_ctl = LCDC->SPI_CTL;

		if (lcdc_data->port->minor_type < DISPLAY_SPI_QUAD) {
			if (cmd >= 0) {
				/* make sure source has selected AHB successfully */
				LCDC->SPI_CTL &= ~SPI_AHB_DATA_MASK;
				LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_AHB | LCD_SPI_CDX(0);

				LCDC->DATA = cmd;
				LCDC->SPI_CTL |= LCD_SPI_AHB_CSX(1);
			}

			if (len > 0) {
				LCDC->SPI_CTL &= ~SPI_AHB_DATA_MASK;
				LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_AHB | LCD_SPI_AHB_CFG_DATA | LCD_SPI_CDX(1) | LCD_SPI_RWL(0);

				do {
					LCDC->DATA = *(uint8_t *)buf;
					buf = (uint8_t *)buf + 1;
				} while (--len > 0);

				LCDC->SPI_CTL |= LCD_SPI_AHB_CSX(1);
			}
		} else {
			const uint8_t *buf8 = buf;
			uint32_t tmp_data[8];
			int i = 0;

			if (len > 32)
				return -EDOM;

			LCDC->SPI_CTL &= ~SPI_AHB_DATA_MASK;
			LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_AHB | LCD_SPI_AHB_CFG_DATA | LCD_SPI_CDX(0) | LCD_SPI_RWL(len);

			/* Transfer sequence:
			 * 1) DATA, DATA_1, ..., DATA_7
			 * 2) In every DATA: alwasy (effective) MSB first:
			 *   if 32 bit in DATA, then 31...0
			 *   if 24 bit in DATA, then 23...0
			 *   if 16 bit in DATA, then 15...0
			 *   if 8 bit in DATA, then  7...0
			 */
			for (; len >= 4; len -= 4) {
				tmp_data[i++] = ((uint32_t)buf8[0] << 24) | ((uint32_t)buf8[1] << 16) |
						((uint32_t)buf8[2] << 8) | buf8[3];
				buf8 += 4;
			}

			if (len > 0) {
				tmp_data[i] = *buf8++;
				while (--len > 0) {
					tmp_data[i] = (tmp_data[i] << 8) | (*buf8++);
				}

				i++;
			}

			LCDC->QSPI_CMD = cmd;
			for (i -= 1; i > 0; i--) {
				LCDC->DATA_1[i - 1] = tmp_data[i];
			}
			LCDC->DATA = tmp_data[0];

			LCDC->SPI_CTL |= LCD_SPI_AHB_CSX(1);
		}

		/* restore REG SPI_CTL */
		LCDC->SPI_CTL = lcdc_spi_ctl;
	} else if (lcdc_data->port->major_type == DISPLAY_PORT_CPU) {
		uint32_t lcdc_cpu_ctl = LCDC->CPU_CTL; /* save REG CPU_CTL */

		if (cmd >= 0) {
			LCDC->CPU_CTL &= ~CPU_AHB_CMD_MASK;
			LCDC->CPU_CTL |= LCD_CPU_AHB_DATA_SEL_CFG | LCD_CPU_RS_LOW;
			LCDC->DATA = cmd;
			LCDC->CPU_CTL |= LCD_CPU_AHB_CSX(1);
		}

		if (len > 0) {
			LCDC->CPU_CTL &= ~CPU_AHB_CMD_MASK;
			LCDC->CPU_CTL |= LCD_CPU_AHB_DATA_SEL_CFG | LCD_CPU_RS_HIGH;
			do {
				LCDC->DATA = *(uint8_t *)buf;
				buf = (uint8_t *)buf + 1;
			} while (--len > 0);

			LCDC->CPU_CTL |= LCD_CPU_AHB_CSX(1);
		}

		/* restore REG CPU_CTL */
		LCDC->CPU_CTL = lcdc_cpu_ctl;
	}

	return 0;
}

static int lcdc_write_pixels_by_mcu(const struct device *dev, int32_t cmd,
		const struct display_buffer_descriptor *desc, const void *buf)
{
	struct lcdc_data *lcdc_data = dev->data;
	uint32_t pixel_format = desc->pixel_format;
	bool f565_24bit = false;
	int i, j;

	if (pixel_format == 0) {
		pixel_format = lcdc_data->mode->pixel_format;
	} else if (pixel_format != lcdc_data->mode->pixel_format) {
		if (pixel_format == PIXEL_FORMAT_ARGB_8888 && (
				lcdc_data->mode->pixel_format == PIXEL_FORMAT_RGB_565 ||
				lcdc_data->mode->pixel_format == PIXEL_FORMAT_BGR_565)) {
			f565_24bit = true;
		} else {
			return -EINVAL;
		}
	}

	pm_device_busy_set(dev);

	if (lcdc_data->port->major_type == DISPLAY_PORT_SPI) {
		uint32_t lcdc_spi_ctl = LCDC->SPI_CTL; /* save LCD_SPI_CTL */

		LCDC->SPI_CTL &= ~SPI_AHB_DATA_MASK;
		LCDC->SPI_CTL |= LCD_SPI_INPUT_SRC_SEL_AHB | LCD_SPI_CDX(1) |
				(f565_24bit ? LCD_SPI_AHB_F565_24BIT : LCD_SPI_AHB_F565_16BIT);

		if (lcdc_data->port->minor_type < DISPLAY_SPI_QUAD) {
			if (pixel_format == PIXEL_FORMAT_RGB_565 ||
				pixel_format == PIXEL_FORMAT_BGR_565) { // rgb565
				for (j = desc->height; j > 0; j--) {
					for (i = desc->width; i > 0; i--) {
						LCDC->DATA = *(uint16_t *)buf;
						buf = (uint16_t *)buf + 1;
					}

					buf = (uint16_t *)buf + (desc->pitch - desc->width);
				}
			} else { /* argb_8888 */
				for (j = desc->height; j > 0; j--) {
					for (i = desc->width; i > 0; i--) {
						LCDC->DATA = *(uint32_t *)buf;
						buf = (uint32_t *)buf + 1;
					}

					buf = (uint32_t *)buf + (desc->pitch - desc->width);
				}
			}

			LCDC->SPI_CTL |= LCD_SPI_AHB_CSX(1);
		} else {
			if (pixel_format == PIXEL_FORMAT_RGB_565 ||
				pixel_format == PIXEL_FORMAT_BGR_565) { // rgb565
				for (j = desc->height; j > 0; j--) {
					for (i = desc->width; i > 0; i--) {
						LCDC->SPI_CTL &= ~LCD_SPI_AHB_CSX_MASK;
						LCDC->QSPI_CMD = cmd;
						LCDC->DATA = *(uint16_t *)buf;
						LCDC->SPI_CTL |= LCD_SPI_AHB_CSX(1);

						buf = (uint16_t *)buf + 1;
					}

					buf = (uint16_t *)buf + (desc->pitch - desc->width);
				}
			} else { /* argb_8888 */
				for (j = desc->height; j > 0; j--) {
					for (i = desc->width; i > 0; i--) {
						LCDC->SPI_CTL &= ~LCD_SPI_AHB_CSX_MASK;
						LCDC->QSPI_CMD = cmd;
						LCDC->DATA = *(uint32_t *)buf;
						LCDC->SPI_CTL |= LCD_SPI_AHB_CSX(1);

						buf = (uint32_t *)buf + 1;
					}

					buf = (uint32_t *)buf + (desc->pitch - desc->width);
				}
			}
		}

		/* restore LCD_SPI_CTL */
		LCDC->SPI_CTL = lcdc_spi_ctl;
	} else if (lcdc_data->port->major_type == DISPLAY_PORT_CPU) {
		uint32_t lcdc_cpu_ctl = LCDC->CPU_CTL;  /* save LCD_CPU_CTL */

		LCDC->CPU_CTL &= ~CPU_AHB_CMD_MASK;
		LCDC->CPU_CTL |= LCD_CPU_AHB_DATA_SEL_IMG | LCD_CPU_RS_HIGH |
				(f565_24bit ? LCD_CPU_AHB_F565_24BIT : LCD_CPU_AHB_F565_16BIT);

		if (pixel_format == PIXEL_FORMAT_RGB_565 ||
			pixel_format == PIXEL_FORMAT_BGR_565) { // rgb565
			for (j = desc->height; j > 0; j--) {
				for (i = desc->width; i > 0; i--) {
					LCDC->DATA = *(uint16_t *)buf;
					buf = (uint16_t *)buf + 1;
				}

				buf = (uint16_t *)buf + (desc->pitch - desc->width);
			}
		} else { /* argb_8888 */
			for (j = desc->height; j > 0; j--) {
				for (i = desc->width; i > 0; i--) {
					LCDC->DATA = *(uint32_t *)buf;
					buf = (uint32_t *)buf + 1;
				}

				buf = (uint32_t *)buf + (desc->pitch - desc->width);
			}
		}

		/* restore LCD_CPU_CTL */
		LCDC->CPU_CTL = lcdc_cpu_ctl;
	}

	pm_device_busy_clear(dev);

	/* notify transfer completed */
	if (lcdc_data->complete_fn) {
		lcdc_data->complete_fn(lcdc_data->complete_fn_arg);
	}

	return 0;
}

static int lcdc_write_pixels_by_de(const struct device *dev,
		int32_t cmd, const struct display_buffer_descriptor *desc)
{
	struct lcdc_data *data = dev->data;

	pm_device_busy_set(dev);

	LCDC->TPL = (uint32_t)desc->width * desc->height - 1;

	switch (data->port->major_type) {
	case DISPLAY_PORT_SPI:
		LOG_DBG("start spi-if\n");
		LCDC->QSPI_CMD = cmd;
		LCDC->SPI_CTL |= LCD_SPI_START;
		break;
	case DISPLAY_PORT_CPU:
	default:
		LOG_DBG("start cpu-if\n");
		LCDC->CPU_CTL |= LCD_CPU_START;
		break;
	}

	return 0;
}

#if CONFIG_LCDC_DMA_CHAN_ID >= 0

static void lcdc_spi_write_pixels_by_dma(const struct device *dev,
				int32_t cmd, const void *buf)
{
	uint32_t spi_ctl = LCDC->SPI_CTL;

	LCDC->QSPI_CMD = cmd;
	LCDC->QSPI_IMG_SIZE = LCDC->DISP_SIZE;

	if ((uint32_t)buf & 0x3) {//HALF WORD
		spi_ctl |= LCD_SPI_HWA_EN;
	} else {
		spi_ctl &= ~LCD_SPI_HWA_EN;
	}

	LCDC->SPI_CTL = spi_ctl | LCD_SPI_START;
}

static void lcdc_cpu_write_pixels_by_dma(const struct device *dev,
				const void *buf)
{
	uint32_t cpu_ctl = LCDC->CPU_CTL;

	if ((uint32_t)buf & 0x3) {//HALF WORD
		cpu_ctl |= LCD_CPU_HWA_EN;
	} else {
		cpu_ctl &= ~LCD_CPU_HWA_EN;
	}

	LCDC->CPU_CTL = cpu_ctl | LCD_CPU_START;
}

static int lcdc_write_pixels_by_dma(const struct device *dev, int32_t cmd,
		const struct display_buffer_descriptor *desc, const void *buf)
{
	struct lcdc_data *data = dev->data;
	uint32_t pixel_format = desc->pixel_format;
	uint32_t dma_len;
	uint16_t dma_bytes_per_line;
	uint8_t bytes_per_pixel;
	uint8_t hwa = 0;

	if (pixel_format == 0) {
		pixel_format = data->mode->pixel_format;
	}

	if (lcdc_config_dma_pixel_format(dev, pixel_format, &bytes_per_pixel)) {
		LOG_ERR("invalid pixel format 0x%x", pixel_format);
		return -EINVAL;
	}

	if (buf_is_psram(buf)) {
		buf = cache_to_uncache((void *)buf);
	}

	hwa = ((uint32_t)buf & 0x3) ? 1 : 0;
	if (hwa && pixel_format != PIXEL_FORMAT_RGB_565 && pixel_format != PIXEL_FORMAT_BGR_565) {
		LOG_ERR("unaligned addr %p", buf);
		return -EINVAL;
	}

	dma_bytes_per_line = (desc->width + hwa) * bytes_per_pixel;
	if ((dma_bytes_per_line & 0x3) || (desc->width + hwa > desc->pitch)) {
		LOG_ERR("unaligned width %u, pitch %u, hwa %d", desc->width, desc->pitch, hwa);
		return -EINVAL;
	}

	dma_len = (uint32_t)dma_bytes_per_line * desc->height;
	if (dma_len >= 0x100000) {
		LOG_ERR("too many pixels, exceed 0x100000 bytes\n");
		return -EDOM;
	}

	/* start DMA */
	assert(DMACHAN->START == 0);

	DMALINE->COUNT = desc->height;
	DMALINE->LENGTH = dma_bytes_per_line;
	DMALINE->STRIDE = desc->pitch * bytes_per_pixel;

	DMACHAN->BC = dma_len;
	DMACHAN->SADDR0 = (uint32_t)buf & ~0x3;
	DMACHAN->DADDR0 = (uint32_t)&LCDC->DATA;
	DMACHAN->CTL = BIT(15) | (0x16 << 8) | /* Dest Select LCD FIFO (Constant) */
			BIT(24) | (CONFIG_LCDC_DMA_LINE_ID << 25); /* stride mode */

	pm_device_busy_set(dev);

	DMACHAN->START = 0x1;

	LCDC->DISP_SIZE = LCD_DISP_SIZE(desc->width, desc->height);

	if (data->port->major_type == DISPLAY_PORT_SPI) {
		lcdc_spi_write_pixels_by_dma(dev, cmd, buf);
	} else { /* DISPLAY_PORT_CPU */
		lcdc_cpu_write_pixels_by_dma(dev, buf);
	}

	return 0;
}
#endif /* CONFIG_LCDC_DMA_CHAN_ID >= 0 */

static int lcdc_write_pixels(const struct device *dev, int32_t cmd,
		const struct display_buffer_descriptor *desc, const void *buf)
{
	struct lcdc_data *data = dev->data;

	if (data->port == NULL || data->mode == NULL) {
		return -EINVAL;
	}

	switch (data->source_type) {
#if CONFIG_LCDC_DMA_CHAN_ID >= 0
	case DISPLAY_CONTROLLER_SOURCE_DMA:
		return lcdc_write_pixels_by_dma(dev, cmd, desc, buf);
#endif
	case DISPLAY_CONTROLLER_SOURCE_ENGINE:
		return lcdc_write_pixels_by_de(dev, cmd, desc);
	case DISPLAY_CONTROLLER_SOURCE_MCU:
		return lcdc_write_pixels_by_mcu(dev, cmd, desc, buf);
	default:
		return -EINVAL;
	}
}

static int lcdc_read_pixels(const struct device *dev, int32_t cmd,
		const struct display_buffer_descriptor *desc, void *buf)
{
	return -ENOTSUP;
}

static int lcdc_control(const struct device *dev, int cmd, void *arg1, void *arg2)
{
	struct lcdc_data *data = dev->data;

	switch (cmd) {
	case DISPLAY_CONTROLLER_CTRL_COMPLETE_CB:
		data->complete_fn_arg = arg2;
		data->complete_fn = arg1;
		break;

#if LCDC_HAS_TE_INT
	case DISPLAY_CONTROLLER_CTRL_VSYNC_CB:
		data->vsync_fn_arg = arg2;
		data->vsync_fn = arg1;
		break;
#endif

	default:
		return -EINVAL;
	}

	return 0;
}

void lcdc_dump(void)
{
	int i;

	printk("lcdc regs:\n");
	printk("\t LCD_CTL            0x%08x\n", LCDC->CTL);
	printk("\t LCD_DISP_SIZE      0x%08x\n", LCDC->DISP_SIZE);
	printk("\t LCD_CPU_CTL        0x%08x\n", LCDC->CPU_CTL);
	printk("\t LCD_DATA           0x%08x\n", LCDC->DATA);
	printk("\t LCD_CPU_CLK        0x%08x\n", LCDC->CPU_CLK);
	printk("\t LCD_TPL            0x%08x\n", LCDC->TPL);
	printk("\t LCD_SPI_CTL        0x%08x\n", LCDC->SPI_CTL);
	printk("\t LCD_QSPI_CMD       0x%08x\n", LCDC->QSPI_CMD);
	printk("\t LCD_QSPI_CMD1      0x%08x\n", LCDC->QSPI_CMD1);
	printk("\t LCD_QSPI_SYNC_TIM  0x%08x\n", LCDC->QSPI_SYNC_TIM);
	printk("\t LCD_QSPI_IMG_SIZE  0x%08x\n", LCDC->QSPI_IMG_SIZE);
	printk("\t DE_INTERFACE_CTL   0x%08x\n", LCDC->DE_INTERFACE_CTL);
	printk("\t LCD_PENDING        0x%08x\n", LCDC->PENDING);
	printk("\t LCD_QSPI_DTAS      0x%08x\n", LCDC->QSPI_DTAS);

	for (i = 1; i <= 7; i++) {
		printk("\t LCD_DATA_%d        0x%08x\n", i, LCDC->DATA_1[i]);
	}

#if CONFIG_LCDC_DMA_CHAN_ID >= 0
	printk("\t DMA_CTL         0x%08x\n", DMACHAN->CTL);
	printk("\t DMA_START       0x%08x\n", DMACHAN->START);
	printk("\t DMA_SADDR0      0x%08x\n", DMACHAN->SADDR0);
	printk("\t DMA_SADDR1      0x%08x\n", DMACHAN->SADDR1);
	printk("\t DMA_DADDR0      0x%08x\n", DMACHAN->DADDR0);
	printk("\t DMA_DADDR1      0x%08x\n", DMACHAN->DADDR1);
	printk("\t DMA_BC          0x%08x\n", DMACHAN->BC);
	printk("\t DMA_RC          0x%08x\n", DMACHAN->RC);
	printk("\t DMA_LINE_LENGTH 0x%08x\n", DMALINE->LENGTH);
	printk("\t DMA_LINE_COUNT  0x%08x\n", DMALINE->COUNT);
	printk("\t DMA_LINE_STRIDE 0x%08x\n", DMALINE->STRIDE);
	printk("\t DMA_LINE_REMAIN 0x%08x\n", DMALINE->REMAIN);
#endif
}

static void lcdc_isr(const void *arg)
{
	const struct device *dev = arg;
	struct lcdc_data *data = dev->data;
	uint32_t pending = LCDC->PENDING;

	LCDC->PENDING = pending;

	if (pending & LCD_STAT_FTC) {
		pm_device_busy_clear(dev);

		if (data->complete_fn)
			data->complete_fn(data->complete_fn_arg);
	}
}

DEVICE_DECLARE(lcdc);

static int lcdc_init(const struct device *dev)
{
	IRQ_CONNECT(IRQ_ID_LCD, 0, lcdc_isr, DEVICE_GET(lcdc), 0);
	irq_enable(IRQ_ID_LCD);
	return 0;
}

#ifdef CONFIG_PM_DEVICE
static int lcdc_pm_control(const struct device *dev, enum pm_device_action action)
{
	int ret = 0;

	switch (action) {
	case PM_DEVICE_ACTION_SUSPEND:
	case PM_DEVICE_ACTION_FORCE_SUSPEND:
	case PM_DEVICE_ACTION_TURN_OFF:
		ret = pm_device_is_busy(dev);
		if (ret) {
			LOG_WRN("lcdc busy (action=%d)", action);
		}
		break;
	default:
		break;
	}

	return ret;
}
#endif /* CONFIG_PM_DEVICE */

static const struct display_controller_driver_api lcdc_api = {
	.control = lcdc_control,
	.enable = lcdc_enable,
	.disable = lcdc_disable,
	.set_mode = lcdc_set_mode,
	.set_source = lcdc_set_source,
	.read_config = lcdc_read_config,
	.write_config = lcdc_write_config,
	.read_pixels = lcdc_read_pixels,
	.write_pixels = lcdc_write_pixels,
};

static struct lcdc_data lcdc_drv_data;

DEVICE_DEFINE(lcdc, CONFIG_LCDC_DEV_NAME, &lcdc_init,
			lcdc_pm_control, &lcdc_drv_data, NULL, POST_KERNEL,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &lcdc_api);
