/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <view_manager.h>
#include <app_ui.h>
#include <simulator_config.h>

#include "bt_watch_simulator.h"

extern void _launcher_app_loop(void* p1, void* p2, void* p3);

struct app_entry_t __app_entry_table[] = {
    {
        .name = "main",
        .stack = 0x10,
        .stack_size = 0,
        .priority = APP_PRIORITY,
        .attribute = RESIDENT_APP,
        .p1 = 0,
        .p2 = 0,
        .p3 = 0,
        .thread_loop = NULL,
    },
    {
        .name = "launcher",
        .stack = 0x11,
        .stack_size = 0,
        .priority = CONFIG_APP_PRIORITY,
        .attribute = DEFAULT_APP | FOREGROUND_APP,
        .p1 = 0,
        .p2 = 0,
        .p3 = 0,
        .thread_loop = _launcher_app_loop,
    },
};

struct app_entry_t * __app_entry_end = NULL;

extern void _ui_service_main_loop(void* parama1, void* parama2, void* parama3);

//extern void media_service_main_loop(void* parama1, void* parama2, void* parama3);

struct service_entry_t __service_entry_table[] = {
    {
        .name = "ui_service",
        .stack = 0x12,
        .stack_size = 0x12,
        .priority = CONFIG_UISRV_PRIORITY,
        .attribute = BACKGROUND_APP,
        .p1 = 0,
        .p2 = 0,
        .p3 = 0,
        .thread_loop = _ui_service_main_loop,
    },
};

struct service_entry_t *__service_entry_end = NULL;

extern int _clock_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _main_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _sport_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _heart_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _message_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _list_view_handler(uint16_t view_id, uint8_t msg_id, void * msg_data);
extern int _clocksel_subview_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _music_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _bp_view_handler(uint16_t view_id, uint8_t msg_id, void *msg_data);
extern int _spo2_view_handler(uint16_t view_id, uint8_t msg_id, void *msg_data);
extern int _alipay_main_view_handler(uint16_t view_id, uint8_t msg_id, void *msg_data);
extern int _alipay_bind_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _alipay_pay_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);
extern int _alipay_unbind_view_handler(uint16_t view_id, uint8_t msg_id, void* msg_data);

view_entry_t __view_entry_table[] = {
    {
        .app_id = "main",
        .proc = _main_view_handler,
        .get_state = NULL,
        .key_map = NULL,        
        .id = MAIN_VIEW,
        .default_order = HIGH_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clock_view",
        .proc = _clock_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = CLOCK_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "sport_view",
        .proc = _sport_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = SPORT_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "heart_view",
        .proc = _heart_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = HEART_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "message",
        .proc = _message_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = MSG_VIEW,
        .default_order = HIGH_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "longview",
        .proc = NULL,
        .get_state = NULL,
        .key_map = NULL,
        .id = TEST_LONG_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT * 2,
    },
    {
        .app_id = "applist",
        .proc = _list_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = APPLIST_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "music_view",
        .proc = _music_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = MUSIC_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clocksel_subview_0",
        .proc = _clocksel_subview_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = CLOCK_SELECTOR_SUBVIEW_0,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clocksel_subview_1",
        .proc = _clocksel_subview_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = CLOCK_SELECTOR_SUBVIEW_1,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clocksel_subview_2",
        .proc = _clocksel_subview_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = CLOCK_SELECTOR_SUBVIEW_2,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clocksel_subview_3",
        .proc = _clocksel_subview_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = CLOCK_SELECTOR_SUBVIEW_3,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clocksel_subview_4",
        .proc = _clocksel_subview_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = CLOCK_SELECTOR_SUBVIEW_4,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "health_bp",
        .proc = _bp_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = HEALTH_BP_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "clocksel_subview_4",
        .proc = _spo2_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = HEALTH_SPO2_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "alipay_main_view",
        .proc = _alipay_main_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = ALIPAY_MAIN_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "alipay_bind_view",
        .proc = _alipay_bind_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = ALIPAY_BIND_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "alipay_pay_view",
        .proc = _alipay_pay_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = ALIPAY_PAY_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
    {
        .app_id = "alipay_unbind_view",
        .proc = _alipay_unbind_view_handler,
        .get_state = NULL,
        .key_map = NULL,
        .id = ALIPAY_UNBIND_VIEW,
        .default_order = NORMAL_ORDER,
        .type = UI_VIEW_LVGL,
        .width = DEF_UI_VIEW_WIDTH,
        .height = DEF_UI_VIEW_HEIGHT,
    },
};

view_entry_t *__view_entry_end = NULL;

int mem_manager_init(void);

void bt_watch_main(void);

static void _bt_watch_simlulator_handler(void *p1, void *p2, void *p3)
{
    bt_watch_main();
}

void bt_watch_simlulator_main(void)
{
    __app_entry_end = &__app_entry_table[ARRAY_SIZE(__app_entry_table)];
    __service_entry_end = &__service_entry_table[ARRAY_SIZE(__service_entry_table)];
    __view_entry_end = &__view_entry_table[ARRAY_SIZE(__view_entry_table)];

    mem_manager_init();

	os_thread_create(NULL, 0, _bt_watch_simlulator_handler, NULL, NULL,
			 NULL, 0, 0, 0);
}
