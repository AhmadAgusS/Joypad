/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <string.h>
#include <native_window.h>

#ifndef UI_ABS
#define UI_ABS(x) ((x) >= 0 ? (x) : (-(x)))
#endif

extern int input_driver_register(input_drv_t* input_drv);

static bool pointer_scan_read(input_drv_t* drv, input_dev_data_t* data)
{
    /* get the pointer location and pressed state */
    native_window_get_pointer_state(data);
    return false;
}

static void pointer_scan_enable(input_drv_t* drv, bool enable)
{
}

int input_pointer_device_init(void)
{
    input_drv_t input_drv;

    input_drv.type = INPUT_DEV_TYPE_POINTER;
    input_drv.enable = pointer_scan_enable;
    input_drv.read_cb = pointer_scan_read;
    input_drv.input_dev = NULL;

    return input_driver_register(&input_drv);
}
