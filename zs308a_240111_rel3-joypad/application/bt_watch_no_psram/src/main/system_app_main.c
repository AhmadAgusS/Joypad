/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system app main
 */
#include <app_manager.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <thread_timer.h>
#include <sys_event.h>
#include "ui_mem.h"
#include "app_switch.h"
#include "system_app.h"
#include "app_ui.h"
#include <stream.h>
#include <property_manager.h>
#ifdef CONFIG_PLAYTTS
#include "tts_manager.h"
#endif

#ifdef CONFIG_BLUETOOTH
#include "mem_manager.h"
#include "bt_manager.h"
#endif

#ifdef CONFIG_RES_MANAGER
#include <lvgl/lvgl_res_loader.h>
#endif

#ifdef CONFIG_BITMAP_FONT
#include <lvgl/lvgl_bitmap_font.h>
#endif

#ifdef CONFIG_LVGL
#include <lvgl/lvgl_view.h>
#endif


#ifdef CONFIG_TOOL
#include "tool_app.h"
#endif

#ifdef CONFIG_SENSOR_MANAGER
#include <sensor_manager.h>
#endif

#ifndef CONFIG_SIMULATOR
#include <drivers/power_supply.h>
#endif

#ifdef CONFIG_ACTLOG
#include <logging/act_log.h>
#endif

#ifdef CONFIG_BT_LOG_SERVICE
#include <bt_log/log_service.h>
#endif

#ifdef CONFIG_ALIPAY_LIB
#include <alipay.h>
#include <alipay_vendor.h>
#include "alipay/alipay_ui.h"
#endif

/*share stack for app thread */
char __aligned(ARCH_STACK_PTR_ALIGN) share_stack_area[CONFIG_APP_STACKSIZE]
__in_section_unique(app.noinit.stack);

extern void system_library_version_dump(void);
extern void trace_init(void);
extern int trace_dma_print_set(unsigned int dma_enable);
extern int card_reader_mode_check(void);
extern bool alarm_wakeup_source_check(void);
extern int ota_app_init_sdcard(void);
extern int ota_app_init_bluetooth(void);
extern bool ota_is_already_done(void);
extern int vendor_app_init(void);
extern void tool_sysrq_init();

static bool att_enter_bqb_flag = false;
static uint8_t super_enter_bqb_mode = 0;
static uint8_t boot_sys_event = SYS_EVENT_NONE;
/*******************************************************************************
 * Msg box definition
 ******************************************************************************/
#ifdef CONFIG_ALARM_APP
extern void * alarm_msgbox_popup_cb(uint16_t msgbox_id, uint8_t msg_id, void *msg_data, void *user_data);
#endif
#ifdef CONFIG_OTA_BACKGROUND
extern void * ota_msgbox_popup_cb(uint16_t msgbox_id, uint8_t msg_id, void *msg_data, void *user_data);
#endif
#ifdef CONFIG_BT_CALL_APP
extern void * btcall_msgbox_popup_cb(uint16_t msgbox_id, uint8_t msg_id, void *msg_data, void *user_data);
#endif

extern void * lpower_msgbox_popup_cb(uint16_t msgbox_id, uint8_t msg_id, void *msg_data, void *user_data);

#ifdef CONFIG_SPP_TEST_SUPPORT
extern int spp_test_app_init(void);
#endif


static const bitmap_font_cache_preset_t font_cache_preset[] = {
	{DEF_FONT28_FILE,64*1024},
	{DEF_FONT32_FILE,84*1024},
#if CONFIG_PSRAM_SIZE > 4096
	{DEF_FONT28_EMOJI,72*1024},
#else
	{DEF_FONT28_EMOJI,36*1024},
#endif
};

void main_msg_proc(void *parama1, void *parama2, void *parama3)
{
	struct app_msg msg = {0};
	int result = 0;

	if (receive_msg(&msg, thread_timer_next_timeout())) {

		switch (msg.type) {
#ifndef CONFIG_SIMULATOR
		case MSG_BAT_CHARGE_EVENT:
			if ((msg.cmd == BAT_CHG_EVENT_DC5V_IN) 
				|| (msg.cmd == BAT_CHG_EVENT_DC5V_OUT)) {
				send_async_msg(app_manager_get_current_app(), &msg);
			}
			break;
#endif
		case MSG_SYS_EVENT:
			if (msg.cmd == SYS_EVENT_BATTERY_TOO_LOW) {
				system_power_off();
			}else if (msg.cmd == SYS_EVENT_BATTERY_LOW) {
				result = send_async_msg(app_manager_get_current_app(), &msg);
				if (!result) {
					boot_sys_event = SYS_EVENT_BATTERY_LOW;
				}
			} else {
				system_bluetooth_event_handle(&msg);
				sys_event_process(msg.cmd);
			}
			break;

		#ifdef CONFIG_INPUT_MANAGER
		case MSG_SR_INPUT:
			system_sr_input_event_handle(msg.ptr);
			break;
		#endif

		#ifdef CONFIG_PLAYTTS
		case MSG_TTS_EVENT:
			if (msg.cmd == TTS_EVENT_START_PLAY) {
				tts_manager_play_process();
			}
			break;
		#endif
		#ifdef CONFIG_INPUT_MANAGER
		case MSG_KEY_INPUT:
			/**input event means esd proecess finished*/
			system_input_event_handle(msg.value);
			break;
		#endif

		case MSG_INPUT_EVENT:
			system_key_event_handle(&msg);
			break;

		case MSG_HOTPLUG_EVENT:
			system_hotplug_event_handle(&msg);
			break;

		case MSG_VOLUME_CHANGED_EVENT:
			//system_app_volume_show(&msg);
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
		case MSG_BT_ENGINE_READY:
		#ifdef CONFIG_BT_LOG_SERVICE
			bt_log_service_start();
		#endif
		#ifdef CONFOG_ALARM_MANAGER
			if (alarm_wakeup_source_check()) {
				system_app_launch(SYS_INIT_ALARM_MODE);
			}
			else 
		#endif
			{
				system_app_launch(SYS_INIT_NORMAL_MODE);
			}

		#ifdef CONFIG_OTA_BACKEND_BLUETOOTH
			ota_app_init_bluetooth();
		#endif
		#ifdef CONFIG_BT_VENDOR
			vendor_app_init();
		#endif
		
		#ifdef CONFIG_ALIPAY_LIB
			ALIPAY_ble_init();
			alipay_ui_init();
		#endif

		#ifdef CONFIG_SENSOR_MANAGER
			sensor_manager_enable(ALGO_ACTIVITY_OUTPUT, 0);
			sensor_manager_enable(ALGO_HANDUP, 0);
		#endif

			if (att_enter_bqb_flag == true) {
				SYS_LOG_INF("ATT Goto BQB TEST");
				os_thread_priority_set(os_current_get(), 0);
			}
			if (boot_sys_event != SYS_EVENT_NONE) {
				msg.type = MSG_SYS_EVENT;
				msg.cmd = boot_sys_event;
				send_async_msg(app_manager_get_current_app(), &msg);
				boot_sys_event = SYS_EVENT_NONE;
			}
			
			#ifdef CONFIG_SPP_TEST_SUPPORT
			spp_test_app_init();
			#endif
			break;

		case MSG_START_APP:
			SYS_LOG_INF("start %s\n", (char *)msg.ptr);
			app_switch((char *)msg.ptr, msg.reserve, true);
			break;

		case MSG_EXIT_APP:
			SYS_LOG_DBG("exit %s\n", (char *)msg.ptr);
			app_manager_exit_app((char *)msg.ptr, true);
			break;
#ifdef CONFIG_BT_ANCS_AMS
		case MSG_BLE_ANCS_AMS_SERVICE:
            bt_manager_ble_ancs_ams_handle(msg.cmd,(void *)msg.value);
		    break;
#endif

#ifdef CONFIG_BT_ACTIONS_SUPER_SERVICE
        case MSG_APP_BLE_SUPER_BQB:
		    system_app_launch(SYS_INIT_NORMAL_MODE);
			if (super_enter_bqb_mode) {
			    super_enter_bqb_mode = 0;
				os_thread_priority_set(os_current_get(), 0);
			}
            break;
#endif

#ifdef CONFIG_TOOL
		case MSG_APP_TOOL_INIT:
			tool_sysrq_init();
			break;
#endif

        case MSG_BT_PAIRING_EVENT:
            SYS_LOG_INF(" MSG_BT_PAIRING_EVENT: cmd:%d value:%d", msg.cmd,msg.value);
            break;

#ifdef CONFIG_BT_MANAGER
        case MSG_BT_DEVICE_ERROR:
            bt_manager_reset_btdev();
            break;
#endif
		default:
			SYS_LOG_ERR(" error: %d\n", msg.type);
			break;
		}

		if (msg.callback)
			msg.callback(&msg, result, NULL);
	}

	thread_timer_handle_expired();
}

#ifdef CONFIG_SIMULATOR
int bt_watch_main(void)
#else
int main(void)
#endif
{
	bool play_welcome = true;
	bool init_bt_manager = true;
	uint16_t reboot_type = 0;
	uint8_t reason = 0;

	system_power_get_reboot_reason(&reboot_type, &reason);

	system_library_version_dump();

	mem_manager_init();
	
	ui_mem_init();

#ifdef CONFIG_ACTLOG
	act_log_init();
#endif

	/* input manager must initialize before ui service */
	system_input_handle_init();

	system_init();

#ifdef CONFIG_RES_MANAGER
	lvgl_res_loader_init(DEF_UI_WIDTH, DEF_UI_HEIGHT);
#endif

#ifdef CONFIG_BITMAP_FONT
	lvgl_bitmap_font_init(NULL);
	lvgl_bitmap_font_cache_preset((void *)font_cache_preset, ARRAY_SIZE(font_cache_preset));
#endif

#ifdef CONFIG_AUDIO
	system_audio_policy_init();
#endif

	system_tts_policy_init();

	system_event_map_init();

#ifdef CONFIG_PLAYTTS
	if (!ota_is_already_done()) {
		tts_manager_play("welcome.act", PLAY_IMMEDIATELY);
	}
#endif

	system_app_launch_init();

#ifdef CONFIG_ALARM_APP
	if (alarm_wakeup_source_check()) {
		play_welcome = false;
	}
#endif

	if ((reboot_type == REBOOT_TYPE_SF_RESET) && (reason == REBOOT_REASON_GOTO_BQB_ATT)) {
		play_welcome = false;
		att_enter_bqb_flag = true;
	}

	if (!play_welcome) {
	#ifdef CONFIG_PLAYTTS
		tts_manager_lock();
	#endif
	} else if(reason != REBOOT_REASON_OTA_FINISHED){
		bool enter_stub_tool = false;


		if (enter_stub_tool == false) {
		#ifdef CONFIG_CARD_READER_APP
			if (usb_hotplug_device_mode()
				&& !(reason == REBOOT_REASON_NORMAL)) {
				if (card_reader_mode_check() == NO_NEED_INIT_BT) {
					init_bt_manager = false;
				}
			}
		#endif
		}
	}


	system_ready();

#ifdef CONFIG_OTA_BACKEND_SDCARD
	ota_app_init_sdcard();
#endif

#ifdef CONFIG_ACTIONS_TRACE
#ifdef CONFIG_TOOL
	if (tool_get_dev_type() == TOOL_DEV_TYPE_UART0) {
		/* stub uart and trace both use uart0, forbidden trace dma mode */
		trace_dma_print_set(false);
	}
#endif
	trace_init();
#endif

#ifdef CONFIG_BT_CONTROLER_RF_FCC
	if ((reboot_type == REBOOT_TYPE_SF_RESET) && (reason == REBOOT_REASON_GOTO_BQB)) {
		SYS_LOG_INF("Goto BQB TEST");
	} else {
		system_enter_FCC_mode();
	}
#endif

#ifdef CONFIG_BT_ACTIONS_SUPER_SERVICE
	if ((reason == REBOOT_REASON_SUPER_BR_BQB) || (reason == REBOOT_REASON_SUPER_LE_BQB)) {
        init_bt_manager = false;
        if(reason == REBOOT_REASON_SUPER_LE_BQB)
            super_enter_bqb_mode = LE_TEST;
        else
            super_enter_bqb_mode = DUT_TEST;
    }
#endif

#ifdef CONFIG_BT_MANAGER

    if(super_enter_bqb_mode){
#ifdef CONFIG_BT_ACTIONS_SUPER_SERVICE
        bt_manager_ble_super_enter_bqb(super_enter_bqb_mode);
#endif
    }
    else{
        if (init_bt_manager) {
            bt_manager_init();
        }
    }
#else
	system_app_launch(SYS_INIT_NORMAL_MODE);
#endif

#if 0
	lv_obj_t * scr = lv_scr_act();

	lv_obj_t * label = lv_label_create(scr);

	lv_obj_set_pos(label, 100, 100);
	lv_obj_set_style_text_color(label, lv_color_make(255, 0, 0), LV_PART_MAIN);
//	lv_obj_set_style_text_font(label, lv_color_make(255, 0, 0), LV_PART_MAIN);
	lv_label_set_text(label, "Hello, world !scr ");

	while (1) {
		lv_task_handler();
		k_sleep(K_MSEC(CONFIG_LV_DISP_DEF_REFR_PERIOD));
	}
#endif
	while (1) {
		main_msg_proc(NULL, NULL, NULL);
	}
}

extern char z_main_stack[CONFIG_MAIN_STACK_SIZE];

APP_DEFINE(main, z_main_stack, CONFIG_MAIN_STACK_SIZE,\
			APP_PRIORITY, RESIDENT_APP,\
			NULL, NULL, NULL,\
			NULL, NULL);

