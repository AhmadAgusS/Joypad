/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
	
#include <stdio.h>
#include <string.h>
#include <zephyr.h>
#include <sys/printk.h>
#include "sensor_port.h"

int main(void)
{
	printk("Sensor Test!\n");

	/* init sensor */
	sensor_init();
	
	/* dead-loop */
	while(1) {
		// poll sensor data
		sensor_poll();
		k_msleep(10);
	}
}
