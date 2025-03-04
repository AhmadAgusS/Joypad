/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fs_manager.h>
#include <file_stream.h>
#include <os_common_api.h>
#include <thread_timer.h>
#include <soc.h>
#include <media_player.h>
#include <srv_manager.h>
#include <app_manager.h>
#include <sys_manager.h>
#include <input_manager.h>

#define STORAGE_NAME "/"CONFIG_MASS_STORAGE_DISK_NAME":/"

extern int system_audio_init(void);
extern void system_audio_policy_init(void);
extern void system_init(void);
extern void *voice_player_start(const char *mic_in_url, const char *sco_in_url, const char *speaker_out_url, const char *sco_out_url); 
extern int voice_player_stop(media_player_t *player, bool force);


void sys_set_bt_host_wake_up_pending(uint8_t pending) {}
void sys_set_bt_ctrl_sleep_pending(uint8_t pending, u32_t sleep_time) {}

extern int card_reader_mode_check(void);
extern bool usb_hotplug_device_mode(void);
int main(void)
{
#ifdef CONFIG_SOC_DVFS_DYNAMIC_LEVEL
	soc_dvfs_set_level(SOC_DVFS_LEVEL_FULL_PERFORMANCE, "main");
#endif

	system_init();

	fs_manager_init();

	system_audio_policy_init();

#ifdef CONFIG_INPUT_MANAGER
	input_manager_init(NULL);
	/** input init is locked ,so we must unlock*/
	input_manager_unlock();
#endif

	if (usb_hotplug_device_mode()) {
		os_sleep(5000);
		card_reader_mode_check();
	}
	fs_unlink(STORAGE_NAME"spk_out.pcm");
	fs_unlink(STORAGE_NAME"sco_out.pcm");

	void *player = NULL;

	player = voice_player_start(STORAGE_NAME"mic_in.pcm", 
								STORAGE_NAME"sco_in.pcm",
								STORAGE_NAME"spk_out.pcm",
								STORAGE_NAME"sco_out.pcm");

	if (player)
		voice_player_stop(player, false);
	else
		return 0;

	return 0;
}

extern char z_main_stack[CONFIG_MAIN_STACK_SIZE];

APP_DEFINE(main, z_main_stack, CONFIG_MAIN_STACK_SIZE,\
			APP_PRIORITY, RESIDENT_APP,\
			NULL, NULL, NULL,\
			NULL, NULL);

