﻿/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for display graphics
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_DISPLAY_DISPLAY_GRAPHICS_H_
#define ZEPHYR_INCLUDE_DRIVERS_DISPLAY_DISPLAY_GRAPHICS_H_

/**
 * @brief Display Graphics
 * @defgroup display_graphics Display Graphics
 * @ingroup display_interfaces
 * @{
 */

#include <sys/util.h>
#include <drivers/display.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum display_blending_type
 * @brief Enumeration with possible display blending type
 *
 */
enum display_blending_type {
	/* no blending */
	DISPLAY_BLENDING_NONE     = 0x0,
	/* ONE / ONE_MINUS_SRC_ALPHA */
	DISPLAY_BLENDING_PREMULT  = 0x1,
	/* SRC_ALPHA / ONE_MINUS_SRC_ALPHA */
	DISPLAY_BLENDING_COVERAGE = 0x2
};

/**
 * @enum display_transform_type
 * @brief Enumeration with possible display transform type
 *
 * IMPORTANT NOTE:
 * DISPLAY_TRANSFORM_ROT_90 is applied CLOCKWISE and AFTER DISPLAY_TRANSFORM_FLIP_{H|V}.
 */
enum {
	/* flip source image horizontally (around the vertical axis) */
	DISPLAY_TRANSFORM_FLIP_H    = 0x01,
	/* flip source image vertically (around the horizontal axis)*/
	DISPLAY_TRANSFORM_FLIP_V    = 0x02,
	/* rotate source image 90 degrees clockwise */
	DISPLAY_TRANSFORM_ROT_90    = 0x04,
	/* rotate source image 180 degrees */
	DISPLAY_TRANSFORM_ROT_180   = 0x03,
	/* rotate source image 270 degrees clockwise */
	DISPLAY_TRANSFORM_ROT_270   = 0x07,
	/* don't use. see system/window.h */
	DISPLAY_TRANSFORM_RESERVED  = 0x08,
};

/**
 * @struct display_color
 * @brief Structure holding display color
 *
 */
typedef struct display_color {
	union {
		struct {
			uint8_t b; /* blue component */
			uint8_t g; /* green component */
			uint8_t r; /* red component */
			uint8_t a; /* alpha component */
		};

		uint32_t full; /* argb8888 full value */
	};
} display_color_t;

/**
 * @struct display_rect
 * @brief Structure holding display retangle
 *
 */
typedef struct display_rect {
	int16_t x; /* x coord */
	int16_t y; /* y coord */
	int16_t w; /* row width in pixel */
	int16_t h; /* column height in pixel */
} display_rect_t;

/**
 * @struct display_buffer
 * @brief Structure holding display buffer
 *
 */
typedef struct display_buffer {
	struct display_buffer_descriptor desc; /* buffer descriptor */
	uint32_t addr; /* buffer address */
} display_buffer_t;

/**
 * @struct display_layer
 * @brief Structure holding display layer for composition
 *
 */
typedef struct display_layer {
	/* display buffer */
	const display_buffer_t *buffer;

	/*
	 * plane color applied to the whole layer:
	 * 1) For buffer == NULL, use color.argb as the default pixel color value
	 * 1) For buffer with format A1/4/8, use color.rgb as the pixel rgb value
	 * 2) For buffer with other formats, use color.a as the global a value
	 */
	display_color_t color;

	/* where to composite the buffer onto the display, the origin is the top-left corner
	 * of the screen.
	 */
	display_rect_t frame;

	/* blending to apply during composition */
	uint8_t blending;
	/* reserved for future */
	uint8_t _pad[3];
} display_layer_t;

/**
 * @brief Query display format is opaque
 *
 * @param pixel_format Display format, see@enum display_pixel_format
 *
 * @return the query result
 */
bool display_format_is_opaque(uint32_t pixel_format);

/**
 * @brief Query display format bits per pixel
 *
 * @param pixel_format Display format, see@enum display_pixel_format
 *
 * @return the query result
 */
uint8_t display_format_get_bits_per_pixel(uint32_t pixel_format);

/**
 * @brief Construct display color
 *
 * @param r r component
 * @param g g component
 * @param b b component
 * @param a a component
 *
 * @return display color structure
 */
static inline display_color_t display_color_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	display_color_t color = { .r = r, .g = g, .b = b, .a = a, };

	return color;
}

/**
 * @brief Construct display color from 32-bit hex value
 *
 * @param c 32-bit color value argb-8888
 *
 * @return display color structure
 */
static inline display_color_t display_color_hex(uint32_t c)
{
	display_color_t color = { .full = c, };

	return color;
}

/**
 * @brief Construct display color from 16-bit hex value
 *
 * @param c 16-bit color value rgb-565
 *
 * @return display color structure
 */
static inline display_color_t display_color_hex16(uint16_t c)
{
	display_color_t color = {
		.a = 0xff,
		.r = (c & 0xf800) >> 8,
		.g = (c & 0x07f0) >> 3,
		.b = (c & 0x001f) << 3,
	};

	color.r = color.r | ((color.r >> 3) & 0x7);
	color.g = color.g | ((color.g >> 2) & 0x3);
	color.b = color.b | ((color.b >> 3) & 0x7);

	return color;
}

/**
 * @brief Query display buffer bytes per line
 *
 * @param buffer pointer to structure display_buffer
 *
 * @return the query result
 */
uint16_t display_buffer_get_bytes_per_line(const display_buffer_t *buffer);

/**
 * @brief Fill color to display buffer
 *
 * @param buffer pointer to structure display_buffer
 * @param color color to fill
 *
 * @retval 0 on success else negative errno code
 */
int display_buffer_fill_color(const display_buffer_t *buffer, display_color_t color);

/**
 * @brief Fill image data to display buffer
 *
 * @param buffer pointer to structure display_buffer
 * @param data image data to fill
 *
 * @retval 0 on success else negative errno code
 */
int display_buffer_fill_image(const display_buffer_t *buffer, const void *data);

/**
 * @brief Flush cache of display buffer after cpu write
 *
 * @param buffer pointer to structure display_buffer
 *
 * @retval N/A
 */
void display_buffer_cpu_write_flush(const display_buffer_t *buffer);

/**
 * @brief Invalidate cache of display buffer after device write
 *
 * @param buffer pointer to structure display_buffer
 *
 * @retval N/A
 */
void display_buffer_dev_write_flush(const display_buffer_t *buffer);

/**
 * @brief Set display rectangle
 *
 * @param rect Pointer to display rectangle structure
 *
 * @return N/A
 */
static inline void display_rect_set(display_rect_t *rect,
		int16_t x, int16_t y, int16_t width, int16_t height)
{
	rect->x = x;
	rect->y = y;
	rect->w = width;
	rect->h = height;
}

/**
 * @brief Move display rectangle
 *
 * @param rect Pointer to display rectangle structure
 * @param dx delta X to move
 * @param dy delta Y to move
 *
 * @return N/A
 */
static inline void display_rect_move(display_rect_t *rect, int16_t dx, int16_t dy)
{
	rect->x += dx;
	rect->y += dy;
}

/**
 * @brief Get display rectangle width
 *
 * @param rect Pointer to display rectangle structure
 *
 * @return the rectangle width
 */
static inline int16_t display_rect_get_width(const display_rect_t *rect)
{
	return rect->w;
}

/**
 * @brief Get display rectangle height
 *
 * @param rect Pointer to display rectangle structure
 *
 * @return the rectangle height
 */
static inline int16_t display_rect_get_height(const display_rect_t *rect)
{
	return rect->h;
}

/**
 * @brief Set display rectangle width
 *
 * @param rect Pointer to display rectangle structure
 * @param width rect width
 *
 * @return N/A
 */
static inline void display_rect_set_width(display_rect_t *rect, int16_t width)
{
	rect->w = width;
}

/**
 * @brief Set display rectangle height
 *
 * @param rect Pointer to display rectangle structure
 * @param height rect height
 *
 * @return N/A
 */
static inline void display_rect_set_height(display_rect_t *rect, int16_t height)
{
	rect->h = height;
}

/**
 * @brief Query display rectangle is valid
 *
 * @param rect Pointer to display rectangle structure
 *
 * @return the query result
 */
static inline bool display_rect_is_valid(const display_rect_t *rect)
{
	return (rect->w >= 0) && (rect->h >= 0);
}

/**
 * @brief Query display rectangle is empty
 *
 * @param rect Pointer to display rectangle structure
 *
 * @return the query result
 */
static inline bool display_rect_is_empty(const display_rect_t *rect)
{
	return (rect->w <= 0) || (rect->h <= 0);
}


/**
 * @brief Intersec two display rectangles
 *
 * @param dest Pointer to dest display rectangle
 * @param src Pointer to src display rectangle
 *
 * @return N/A
 */
void display_rect_intersect(display_rect_t *dest, const display_rect_t *src);

/**
 * @brief Merge two display rectangles
 *
 * @param dest Pointer to dest display rectangle
 * @param src Pointer to src display rectangle
 *
 * @return N/A
 */
void display_rect_merge(display_rect_t *dest, const display_rect_t *src);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_DRIVERS_DISPLAY_DISPLAY_GRAPHICS_H_ */

