/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ATS3085C_DVB_WATCH_EXT_NOR_CONF_H_
#define ATS3085C_DVB_WATCH_EXT_NOR_CONF_H_

 /* panel config */
#define CONFIG_PANEL_HOR_RES 360
#define CONFIG_PANEL_VER_RES 360
#define CONFIG_PANEL_ROUND_SHAPE 1
/* psram size */
#define CONFIG_PSRAM_SIZE 6144
/* lvgl config */
#define CONFIG_LVGL_VDB_SIZE 10800
#define CONFIG_LV_MEM_SIZE_KILOBYTES 64
/* ui mem config */
#define CONFIG_UI_MEM_BLOCK_SIZE 259200
#define CONFIG_UI_MEM_NUMBER_BLOCKS 10
#define CONFIG_UI_RES_MEM_POOL_SIZE 1100000
/* font cache config */
#define CONFIG_BITMAP_PER_FONT_CACHE_SIZE 49152
#define CONFIG_BITMAP_FONT_CACHE_POOL_SIZE 230000
/* view cache config */
#define CONFIG_VIEW_CACHE_LEVEL 2
#define CONFIG_VIEW_SCROLL_MEM_DEFAULT 1
/* resource path config */
#define CONFIG_APP_UI_DISK "../../boards/ats3085c_dvb_watch_ext_nor/res/"
#define CONFIG_APP_FONT_DISK "../../boards/ats3085c_dvb_watch_ext_nor/fonts/"

#endif /* ATS3085C_DVB_WATCH_EXT_NOR_CONF_H_ */
