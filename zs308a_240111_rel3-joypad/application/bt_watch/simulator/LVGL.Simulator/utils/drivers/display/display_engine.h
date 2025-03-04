/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for display engine drivers
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_DISPLAY_DISPLAY_ENGINE_H_
#define ZEPHYR_INCLUDE_DRIVERS_DISPLAY_DISPLAY_ENGINE_H_

#include "display_graphics.h"

/**
 * @brief Display Engine Interface
 * @defgroup display_engine_interface Display Engine Interface
 * @ingroup display_interfaces
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* open flags of display engine */
#define DISPLAY_ENGINE_FLAG_HIGH_PRIO  BIT(0)
#define DISPLAY_ENGINE_FLAG_POST       BIT(1) /* For display post */

/**
 * @enum display_engine_ctrl_type
 * @brief Enumeration with possible display engine ctrl command
 *
 */
enum display_engine_ctrl_type {
	DISPLAY_ENGINE_CTRL_DISPLAY_PORT = 0, /* configure display video port */
	DISPLAY_ENGINE_CTRL_DISPLAY_MODE,     /* configure display video mode */
	/* prepare display refreshing.
	 * arg1 is the callback function, arg2 must point to the display device structure
	 */
	DISPLAY_ENGINE_CTRL_DISPLAY_PREPARE_CB,
	/* start display refreshing
	 * arg1 is the callback function, arg2 must point to the display device structure
	 */
	DISPLAY_ENGINE_CTRL_DISPLAY_START_CB,
};

/**
 * @struct display_engine_capabilities
 * @brief Structure holding display engine capabilities
 *
 * @var uint8_t display_capabilities::num_overlays
 * Maximum number of overlays supported
 *
 * @var uint8_t display_capabilities::support_blend_fg
 * Blending constant fg color supported
 *
 * @var uint8_t display_capabilities::support_blend_b
 * Blending constant bg color supported
 *
 * @var uint32_t display_capabilities::supported_input_pixel_formats
 * Bitwise or of input pixel formats supported by the display engine
 *
 * @var uint32_t display_capabilities::supported_output_pixel_formats
 * Bitwise or of output pixel formats supported by the display engine
 *
 * @var uint32_t display_capabilities::supported_rotate_pixel_formats
 * Bitwise or of rotation pixel formats supported by the display engine
 *
 */
struct display_engine_capabilities {
	uint8_t num_overlays;
	uint8_t support_blend_fg : 1;
	uint8_t support_blend_bg : 1;
	uint32_t supported_input_pixel_formats;
	uint32_t supported_output_pixel_formats;
	uint32_t supported_rotate_pixel_formats;
};

/**
 * @struct display_engine_rotation
 * @brief Structure holding display engine rotation configuration
 *
 */
typedef struct display_engine_rotation {
	/* dst buffer */
	uint32_t dst_address; /* dst address at (0, row_start) */
	uint16_t dst_pitch;   /* dst bytes per row, must not less than outer_diameter * bytes_per_pixel(pixel_format) */

	/* src buffer */
	uint16_t src_pitch;   /* src bytes per row, must not less than outer_diameter * bytes_per_pixel(pixel_format) */
	uint32_t src_address; /* src address mapping to dest address at (0, row_start) */

	/* diplay pixel format both of src and dest */
	uint32_t pixel_format;

	/* line range [line_start, line_end) to do rotation */
	uint16_t line_start;
	uint16_t line_end;

	/* fill the inner hole with color  */
	display_color_t fill_color;
	uint16_t fill_enable : 1;

	/* 
	 * 1) the center both of the outer and inner circle is (outer_diameter/2, outer_diameter/2)
	 * 2) only the ring area decided by outer_radius_sq and inner_radius_sq will be rotated.
	 * 3) the pixles in the square area "outer_diameter x outer_diameter" but not inside the
	 *    ring area will be filled with fill_color if fill_enable set.
	 * 4) set inner_radius_sq to 0 and outer_radius_sq to (outer_diameter/2)^2 to
	 *    rotate the whole circular image area.
	 */
	/* width in pixels both of the src and dest, which decide a square area contains the circular image content */
	uint16_t outer_diameter;  /* outer circle diameter in pixels */
	uint32_t outer_radius_sq; /* outer circle radius square in .2 fixedpoint format */
	uint32_t inner_radius_sq; /* inner circle radius square in .2 fixedpoint format */

	/* assistant variables */
	uint32_t dst_dist_sq;   /* distance square between (0, row_start) and center (width/2, width/2) in .2 fixedpoint */
	int32_t src_coord_x;     /* src X coord in .12 fixedpoint mapping to dest coord (0, row_start) */
	int32_t src_coord_y;     /* src Y coord in .12 fixedpoint mapping to dest coord (0, row_start) */
	int32_t src_coord_dx_ax; /* src X coord diff in .12 fixedpoint along the dest X-axis */
	int32_t src_coord_dy_ax; /* src Y coord diff in .12 fixedpoint along the dest X-axis */
	int32_t src_coord_dx_ay; /* src X coord diff in .12 fixedpoint along the dest Y-axis */
	int32_t src_coord_dy_ay; /* src Y coord diff in .12 fixedpoint along the dest Y-axis */
} display_engine_rotation_t;


/**
 * @brief Display Engine driver API
 * API which a display engine driver should expose
 */
struct display_engine_driver_api {
	display_engine_control_api control;
	display_engine_open_api open;
	display_engine_close_api close;
	display_engine_get_capabilities_api get_capabilities;
	display_engine_register_callback_api register_callback;
	display_engine_fill_api fill;
	display_engine_blit_api blit;
	display_engine_blend_api blend;
	display_engine_rotate_api rotate;
	display_engine_compose_api compose;
	display_engine_poll_api poll;
};


#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_DRIVERS_DISPLAY_DISPLAY_ENGINE_H_ */
