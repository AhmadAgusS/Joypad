/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system app main
 */
#include <soc.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <thread_timer.h>
#include <sys_event.h>
#include "system_app.h"
#include <property_manager.h>
#include <dvfs.h>
#ifdef CONFIG_BT_BREDR
#include "app_br.h"
#endif //CONFIG_BT_BREDR
#include "rmc_timer.h"


/*share stack for app thread */
char __aligned(ARCH_STACK_PTR_ALIGN) share_stack_area[CONFIG_APP_STACKSIZE];


#define MAC_STR_LEN		            (12+1)	/* 12(len) + 1(NULL) */
static char g_name_str[] = CONFIG_BT_DEVICE_NAME;
static char g_mac_str[MAC_STR_LEN] = {'F','F','E','E','7','7','8','8','9','9','6','6','\0'};
//static char g_mac_str[MAC_STR_LEN] = {'F','F','E','E','1','1','2','2','3','3','4','4','\0'};
int main(void)
{
#ifdef CONFIG_ACTLOG
	act_log_init();
#endif

	SYS_LOG_ERR("===============================\n");
	property_set_factory(CFG_BT_NAME, g_name_str, sizeof(g_name_str));
	property_set_factory(CFG_BT_MAC, g_mac_str, sizeof(g_mac_str));
#ifdef CONFIG_BT_BREDR
	if(!app_br_init()){
		SYS_LOG_ERR("app_br_init\n");
		return 0;
	}
#endif //CONFIG_BT_BREDR

	struct app_msg msg;
	memset(&msg, 0, sizeof(msg));

	os_tid_t tid = os_current_get();
	if (!msg_manager_add_listener(APP_ID_MAIN, tid)){
		SYS_LOG_ERR("%s add listener failed\n", APP_ID_MAIN);
	}
	msg_manager_init();

#ifdef CONFIG_BT_BREDR
	rmc_timer_init();
	app_br_start_discover();
#endif //CONFIG_BT_BREDR


	while (1)
	{
		if (receive_msg(&msg, OS_FOREVER))
		{
			switch (msg.type)
			{
				case MSG_APP_TIMER:
					#if defined CONFIG_BT_BLE || defined CONFIG_BT_BREDR
					rmc_timer_event_handle(msg.value);
					#endif //defined CONFIG_BT_BLE || defined CONFIG_BT_BREDR
					break;
				#ifdef CONFIG_BT_BREDR
				case MSG_BR_EVENT:
					app_br_event_handle(msg.cmd, (void *)msg.value);
					break;
				#endif //CONFIG_BT_BREDR
				default:
					printk("error message type msg.type %d \n",msg.type);
					continue;
			}
		}
	}

	return 0;

}

extern char z_main_stack[CONFIG_MAIN_STACK_SIZE];

APP_DEFINE(main, z_main_stack, CONFIG_MAIN_STACK_SIZE,\
			APP_PRIORITY, RESIDENT_APP,\
			NULL, NULL, NULL,\
			NULL, NULL);

