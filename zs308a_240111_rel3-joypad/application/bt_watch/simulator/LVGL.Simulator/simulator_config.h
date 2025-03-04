/**
 * @file simulator_config.h
 * Configuration file for bt watch simulator
 *
 */

#ifndef SIMULATOR_CONF_H_
#define SIMULATOR_CONF_H_

#define CONFIG_SIMULATOR 1
#define CONFIG_BOARD_ATS3085_DVB_WATCH 1

/***********************************************
 * general config
 ***********************************************/
#define CONFIG_LVGL 1
#define CONFIG_LV_COLOR_DEPTH 16
#define CONFIG_LV_COLOR_DEPTH_16 1
#define CONFIG_LV_MEM_CUSTOM 1
#define CONFIG_LV_DISP_TOP_LAYER 1

#define CONFIG_INPUT_MANAGER 1
#define CONFIG_INPUT_POINTER 1
#define CONFIG_INPUT_KEYPAD 1
#define CONFIG_RES_MANAGER 1
#define CONFIG_BITMAP_FONT 1
#define CONFIG_BITMAP_FONT_SUPPORT_EMOJI 1
#define CONFIG_LVGL_RES_PRELOAD_STACKSIZE 1536
#define CONFIG_LVGL_RES_PRELOAD_PRIORITY  5

#define CONFIG_UI_MANAGER 1
#define CONFIG_UI_SERVICE 1
#define CONFIG_UISRV_STACKSIZE 3072
#define CONFIG_UISRV_PRIORITY 1
#define CONFIG_UI_VIEW_OVERLAY_OPA 128
#define CONFIG_UI_CUSTOM_GESTURE_DETECTION 1
#define CONFIG_UI_MEMORY_MANAGER 1
#define CONFIG_SURFACE_DOUBLE_BUFFER 1
#define CONFIG_VIEW_STACK_LEVEL 5

#define CONFIG_APP_USED_MEM_POOL 1
#define CONFIG_RAM_POOL_PAGE_NUM 14
#define CONFIG_SYS_HEAP_ALLOC_LOOPS 3
#define CONFIG_LOG_PRINTK 0
#define CONFIG_NUM_MBOX_ASYNC_MSGS 50
#define CONFIG_NUM_COOP_PRIORITIES 15

#define CONFIG_LAUNCHER_APP 1
#define CONFIG_APP_PRIORITY 10
#define CONFIG_APP_STACKSIZE 2048
#define CONFIG_MAIN_STACK_SIZE 2048
#define CONFIG_CLOCK_DEF_REFR_PERIOD 100
#define CONFIG_CLOCK_SELECTOR_USE_VIEW_SLIDING
#define CONFIG_APP_FAT_DISK "../../udisk/"

/***********************************************
 * board specific config
 ***********************************************/
#if defined(CONFIG_BOARD_ATS3085_DVB_WATCH)
#include "boards/ats3085_dvb_watch_conf.h"
#elif defined(CONFIG_BOARD_ATS3085C_DVB_WATCH_EXT_NOR)
#include "boards/ats3085c_dvb_watch_ext_nor_conf.h"
#elif defined(CONFIG_ATS3085L_DVB_WATCH_EXT_NOR_MAX)
#include "boards/ats3085l_dvb_watch_ext_nor_max_conf.h"
#else /* default board config */
/* panel config */
#define CONFIG_PANEL_HOR_RES 454
#define CONFIG_PANEL_VER_RES 454
#define CONFIG_PANEL_ROUND_SHAPE 1
/* psram config */
#define CONFIG_PSRAM_SIZE 8192
/* lvgl config */
#define CONFIG_LVGL_VDB_SIZE 103058 /* 50% framebuffer */
#define CONFIG_LV_MEM_SIZE_KILOBYTES 96
/* ui mem config */
#define CONFIG_UI_MEM_BLOCK_SIZE 412232
#define CONFIG_UI_MEM_NUMBER_BLOCKS 10
#define CONFIG_UI_RES_MEM_POOL_SIZE 2200000
/* font cache config */
#define CONFIG_BITMAP_PER_FONT_CACHE_SIZE  98304
#define CONFIG_BITMAP_FONT_CACHE_POOL_SIZE 300000
/* view cache config */
#define CONFIG_VIEW_CACHE_LEVEL 2
#define CONFIG_VIEW_SCROLL_MEM_DEFAULT 1
/* resource path config */
#define CONFIG_APP_UI_DISK "../../boards/ats3085_dvb_watch/res/"
#define CONFIG_APP_FONT_DISK "../../boards/ats3085_dvb_watch/fonts/"
#endif /* defined(CONFIG_BOARD_ATS3085_DVB_WATCH) */

#endif /*SIMULATOR_CONF_H_*/
