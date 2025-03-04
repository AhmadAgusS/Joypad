/*
 * Copyright (c) 2018 Intel Corporation.
 * Copyright (c) 2020 Peter Bigot Consulting, LLC
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <drivers/display/display_graphics.h>

uint8_t display_format_get_bits_per_pixel(uint32_t pixel_format)
{
    switch (pixel_format) {
    case PIXEL_FORMAT_ARGB_8888:
        return 32;
    case PIXEL_FORMAT_RGB_565:
    case PIXEL_FORMAT_BGR_565:
        return 16;
    case PIXEL_FORMAT_RGB_888:
    case PIXEL_FORMAT_ARGB_6666:
        return 24;
    case PIXEL_FORMAT_A8:
        return 8;
    case PIXEL_FORMAT_A4:
    case PIXEL_FORMAT_A4_LE:
        return 4;
    case PIXEL_FORMAT_A1:
    case PIXEL_FORMAT_A1_LE:
    case PIXEL_FORMAT_MONO01:
    case PIXEL_FORMAT_MONO10:
        return 1;
    default:
        return 0;
    }
}
