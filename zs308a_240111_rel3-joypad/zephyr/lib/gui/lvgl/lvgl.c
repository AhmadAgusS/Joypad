/*
 * Copyright (c) 2018-2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <zephyr.h>
#include <lvgl.h>
#ifdef CONFIG_LVGL_USE_FILESYSTEM
#  include "lvgl_fs.h"
#endif
#ifndef CONFIG_UI_SERVICE
#  include "lvgl_display.h"
#  include "lvgl_pointer.h"
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(lvgl, CONFIG_LV_LOG_LEVEL);

#if defined(CONFIG_LV_USE_LOG) && !defined(CONFIG_LV_LOG_PRINTF)
static void lvgl_log(const char *buf)
{
	printk("%s", buf);
}
#endif

#ifndef CONFIG_UI_SERVICE
int lvgl_init()
{
	//ARG_UNUSED(dev);

#if defined(CONFIG_LV_USE_LOG) && !defined(CONFIG_LV_LOG_PRINTF)
	lv_log_register_print_cb(lvgl_log);
#endif

	lv_init();

#ifdef CONFIG_LVGL_USE_FILESYSTEM
	lvgl_fs_init();
#endif

	if (lvgl_display_init()) {
		return -ENODEV;
	}

	lvgl_pointer_init();
	return 0;
}

//SYS_INIT(lvgl_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#else /* CONFIG_UI_SERVICE */

int lvgl_init(void)
{
#if defined(CONFIG_LV_USE_LOG) && !defined(CONFIG_LV_LOG_PRINTF)
	lv_log_register_print_cb(lvgl_log);
#endif

	lv_init();

#ifdef CONFIG_LVGL_USE_FILESYSTEM
	lvgl_fs_init();
#endif

	return 0;
}

#endif /* CONFIG_UI_SERVICE */
