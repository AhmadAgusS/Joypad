/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system app main
 */
#include <mem_manager.h>
#include <msg_manager.h>
#include <thread_timer.h>
#include <sys_event.h>
#include "app_switch.h"
#include "system_app.h"
#include "app_ui.h"
#include <stream.h>
#include <property_manager.h>

#ifdef CONFIG_LVGL_USE_RES_MANAGER
#include <lvgl/lvgl_res_loader.h>
#endif

#ifdef CONFIG_LVGL_USE_BITMAP_FONT
#include <lvgl/lvgl_bitmap_font.h>
#endif

#ifdef CONFIG_LVGL
#include <lvgl/lvgl_view.h>
#endif
#ifdef CONFIG_UI_SERVICE
#include <ui_service.h>
#include <msgbox_cache.h>
#endif

#ifdef CONFIG_ACTLOG
#include <logging/act_log.h>
#endif

/*share stack for app thread */
char __aligned(ARCH_STACK_PTR_ALIGN) share_stack_area[CONFIG_APP_STACKSIZE];

extern int gui_view_system_init(void);
extern void system_library_version_dump(void);

void main_msg_proc(void *parama1, void *parama2, void *parama3)
{
	struct app_msg msg = {0};
	int result = 0;

	if (receive_msg(&msg, thread_timer_next_timeout())) {

		switch (msg.type) {
		case MSG_SYS_EVENT:
			if (msg.cmd == SYS_EVENT_BATTERY_TOO_LOW) {
				system_power_off();
			}else if (msg.cmd == SYS_EVENT_BATTERY_LOW) {
				send_async_msg(app_manager_get_current_app(), &msg);
			} else {
				sys_event_process(msg.cmd);
			}
			break;

		#ifdef CONFIG_INPUT_MANAGER
		case MSG_SR_INPUT:
			system_sr_input_event_handle(msg.ptr);
			break;
		#endif

		#ifdef CONFIG_INPUT_MANAGER
		case MSG_KEY_INPUT:
			/**input event means esd proecess finished*/
			system_input_event_handle(msg.value);
			break;
		#endif

		case MSG_INPUT_EVENT:
			break;

		case MSG_POWER_OFF:
			system_power_off();
			break;
		case MSG_REBOOT:
			system_power_reboot(msg.cmd);
			break;
		case MSG_NO_POWER:
			sys_event_notify(SYS_EVENT_POWER_OFF);
			break;

		case MSG_START_APP:
			SYS_LOG_INF("start %s\n", (char *)msg.ptr);
			app_switch((char *)msg.ptr, msg.reserve, true);
			break;

		case MSG_EXIT_APP:
			SYS_LOG_DBG("exit %s\n", (char *)msg.ptr);
			app_manager_exit_app((char *)msg.ptr, true);
			break;

		default:
			SYS_LOG_DBG(" error: %d\n", msg.type);
			break;
		}

		if (msg.callback)
			msg.callback(&msg, result, NULL);
	}

	thread_timer_handle_expired();
}

int main(void)
{
	system_library_version_dump();

	mem_manager_init();

#ifdef CONFIG_ACTLOG
	act_log_init();
#endif

	/* input manager must initialize before ui service */
	system_input_handle_init();

	system_init();

#ifdef CONFIG_LVGL_USE_RES_MANAGER
	lvgl_res_loader_init(DEF_UI_WIDTH, DEF_UI_HEIGHT);
#endif

#ifdef CONFIG_UI_SERVICE
#ifdef CONFIG_LVGL
	lvgl_view_system_init();
#else
	gui_view_system_init();
#endif
	ui_service_register_gesture_default_callback();
	//msgbox_cache_init(NULL, 0);
#endif

#ifdef CONFIG_LVGL_USE_BITMAP_FONT
	lvgl_bitmap_font_init(NULL);
	//lvgl_bitmap_font_cache_preset((void *)font_cache_preset, ARRAY_SIZE(font_cache_preset));
#endif

	system_app_launch_init();

	system_ready();
	system_app_launch(SYS_INIT_NORMAL_MODE);

	while (1) {
		main_msg_proc(NULL, NULL, NULL);
	}
}

extern char z_main_stack[CONFIG_MAIN_STACK_SIZE];

APP_DEFINE(main, z_main_stack, CONFIG_MAIN_STACK_SIZE,\
			APP_PRIORITY, RESIDENT_APP,\
			NULL, NULL, NULL,\
			NULL, NULL);

