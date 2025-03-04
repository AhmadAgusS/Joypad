/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ATS3085L_DVB_WATCH_EXT_NOR_MAX_CONF_H_
#define ATS3085L_DVB_WATCH_EXT_NOR_MAX_CONF_H_

 /* panel config */
#define CONFIG_PANEL_HOR_RES 360
#define CONFIG_PANEL_VER_RES 360
#define CONFIG_PANEL_ROUND_SHAPE 1
/* psram size */
#define CONFIG_PSRAM_SIZE 4096
/* lvgl config */
#define CONFIG_LVGL_VDB_SIZE 10800
#define CONFIG_LV_MEM_SIZE_KILOBYTES 28
/* ui mem config */
#define CONFIG_UI_MEM_BLOCK_SIZE 259200
#define CONFIG_UI_MEM_NUMBER_BLOCKS 5
#define CONFIG_UI_RES_MEM_POOL_SIZE 480000
/* font cache config */
#define CONFIG_BITMAP_PER_FONT_CACHE_SIZE 23552
#define CONFIG_BITMAP_FONT_CACHE_POOL_SIZE 95000
/* view cache config */
#define CONFIG_VIEW_CACHE_LEVEL 1
#define CONFIG_VIEW_SCROLL_MEM_LOWEST 1
/* resource path config */
#define CONFIG_APP_UI_DISK "../../boards/ats3085l_dvb_watch_ext_nor_max/res/"
#define CONFIG_APP_FONT_DISK "../../boards/ats3085l_dvb_watch_ext_nor_max/fonts/"

#endif /* ATS3085L_DVB_WATCH_EXT_NOR_MAX_CONF_H_ */
