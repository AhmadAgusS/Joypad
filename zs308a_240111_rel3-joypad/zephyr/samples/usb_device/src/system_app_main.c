/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/usb/usb_phy.h>
#include <usb/usb_device.h>
// #include "usb_cdc_acm/usb_cdc_acm.h"
#include "usb_hid/usb_hid_handler.h"

#define LOG_LEVEL LOG_LEVEL_WRN
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

int main(void)
{
	// if you would like to test cdc_acm: cancel comments of usb_phy_enter_b_idle, usb_phy_init, usb_cdc_acm_start
	// usb_phy_enter_b_idle();
	// usb_phy_init();
	// usb_cdc_acm_start();

	usb_hid_pre_init(NULL);

	while (1) {
		k_sleep(K_MSEC(1000));
	}
	return 0;
}

