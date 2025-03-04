/*
 * Copyright (c) 2019 Actions Semiconductor Co, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <device.h>
#include <thread_timer.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <app_defines.h>
#include <app_manager.h>
#include <app_switch.h>
#include <srv_manager.h>
#include <sys_manager.h>
#include <sys_event.h>
#include <sys_wakelock.h>
#include <soc_pm.h>     // sys_pm_reboot()
#include <fw_version.h> // fw_version_get_current
#include <ota_upgrade.h>
#include <ota_backend.h>
#include <ota_backend_usbhid.h>
#include <os_common_api.h>
#include <config.h>
#include <drivers/nvram_config.h>
#include <drivers/flash.h>
#include <partition/partition.h>
#include <soc_pstore.h>
#ifdef CONFIG_DVFS
#include <dvfs.h>
#endif
#ifdef CONFIG_PROPERTY
#include <property_manager.h>
#endif
#include "app_config.h"
#include "usb_hid_handler.h"
#include "ota_app.h"

#define CONFIG_XSPI_NOR_ACTS_DEV_NAME "spi_flash"
#define OTA_STORAGE_DEVICE_NAME		CONFIG_XSPI_NOR_ACTS_DEV_NAME

static struct ota_upgrade_info *g_ota;
static struct ota_backend *backend_usbhid;

extern void ota_backend_notify_callback(struct ota_backend *backend, int cmd, int state)
{
	// SYS_LOG_INF("backend %p cmd %d state %d", backend, cmd, state);

	if (cmd == OTA_BACKEND_UPGRADE_PROGRESS) {
#ifdef CONFIG_UI_MANAGER
        // you could update progress view (if there's lcd display), reference: bt_watch app
		// ota_view_show_upgrade_progress(state);
#endif
        SYS_LOG_INF("%s, Upgrade Progress: %d%%", __func__, state);
	}
}

static void sys_clear_remain_msg(void)
{
	struct app_msg  msg;

	/*clear all remain message */
	while (receive_msg(&msg, 0)) {
		if (msg.callback) {
			msg.callback(&msg, 0, NULL);
		}
	}
}

#define	REBOOT_REASON_OTA_FINISHED  0x02
void ota_app_notify(int state, int old_state)
{
	SYS_LOG_INF("%s, ota state: %d->%d", __func__, old_state, state);
	CFG_Struct_OTA_Settings cfg;

    app_config_read (
        CFG_ID_OTA_SETTINGS,
        &cfg,0,sizeof(CFG_Struct_OTA_Settings)
    );

	if (state == OTA_DONE) {
        sys_clear_remain_msg();	
#ifdef CONFIG_PROPERTY
		property_set("REC_OTA_FLAG", "yes", 4);
		property_set("OTA_UPG_FLAG", "done", 5);
		property_flush(NULL);
#endif
		/*Set hardware flags to ensure enter ota upgrade durring bootloader*/
		soc_pstore_set(SOC_PSTORE_TAG_OTA_UPGRADE, 1);
        
        app_switch_unlock(1);
	}
}

#define  OTA_BP_STRUCT_SIZE 62
static int update_restore_ota_bp(void)
{
    int rlen = 0;
    const struct fw_version *cur_ver;

    u8_t bp[OTA_BP_STRUCT_SIZE] = {0};

    rlen = nvram_config_get("OTA_BP", bp, OTA_BP_STRUCT_SIZE);
    if (rlen != OTA_BP_STRUCT_SIZE) {
        SYS_LOG_INF("cannot found OTA_BP");
        return -1;
    }

    memset(bp, 0, OTA_BP_STRUCT_SIZE);
    bp[3] = 0x0;
    bp[1] = !partition_get_current_mirror_id();
    cur_ver = fw_version_get_current();
    *((u32_t *)bp+1) = cur_ver->version_code;
    rlen = nvram_config_set("OTA_BP", bp, OTA_BP_STRUCT_SIZE);
    if(rlen)
        return -1;

    return 0;
}

extern struct k_sem hid_write_sem, hid_read_sem;
struct ota_backend *ota_app_init_usbhid(void *mode)
{
    struct ota_backend_usbhid_init_param ota_back_param;
    ota_back_param.sem_read = &hid_read_sem;        // pass in usbhid read semaphore
    ota_back_param.sem_write = &hid_write_sem;        // pass in usbhid read semaphore
    
    struct ota_backend *backend_usbhid_remote = ota_backend_usbhid_init(ota_backend_notify_callback, &ota_back_param);

    if(!backend_usbhid_remote)
        return NULL;

    return backend_usbhid_remote;
}

int ota_app_init(void)
{
	struct ota_upgrade_param param;
	CFG_Struct_OTA_Settings cfg;
	
    app_config_read
    (
        CFG_ID_OTA_SETTINGS,
        &cfg,0,sizeof(CFG_Struct_OTA_Settings)
    );
	
	memset(&param, 0x0, sizeof(struct ota_upgrade_param));

	param.storage_name = OTA_STORAGE_DEVICE_NAME;
	param.notify = ota_app_notify;
	param.flag_use_recovery = 1;
	param.flag_erase_part_for_upg = 1;
    param.no_version_control = 1;   // skip check version number

    update_restore_ota_bp();

	if (cfg.Enable_Ver_Low)
		param.no_version_control = 1;

	const struct device *flash_device = device_get_binding(CONFIG_XSPI_NOR_ACTS_DEV_NAME);
	if (!flash_device) {
        SYS_LOG_ERR("%s, Fail to find flash device[%s]", __func__, CONFIG_XSPI_NOR_ACTS_DEV_NAME);
        return -1;
    }
    
    flash_write_protection_set((const struct device *)flash_device, false);

	g_ota = ota_upgrade_init(&param);
	if (!g_ota) {
		SYS_LOG_INF("%s, Init failed", __func__);
		flash_write_protection_set(flash_device, true);
		return -2;
	}

    

	flash_write_protection_set(flash_device, true);
	return 0;
}

void start_usbhid_upgrade(void)
{
    int ret = 0;
    u32_t update_time = 0;

    SYS_LOG_INF("%s, Enter update", __func__);    
    
#ifdef CONFIG_DVFS_DYNAMIC_LEVEL
	dvfs_set_level(DVFS_LEVEL_HIGH_PERFORMANCE, "ota");
#endif
    // attach usbhid as ota backend
    backend_usbhid = ota_app_init_usbhid(0);
    ota_upgrade_attach_backend(g_ota, backend_usbhid);

    update_time = os_uptime_get_32();
    ret = ota_upgrade_check(g_ota);
    if (ret == 0) {
        SYS_LOG_INF("%s, ota_upgrade_check successful. Time spent: %d", __func__, os_uptime_get_32() - update_time);
        sys_pm_reboot(REBOOT_REASON_OTA_FINISHED);
        return;

    }
    SYS_LOG_ERR("%s, ota_upgrade_check fail, ret=%p", __func__, __LINE__, ret);
#ifdef CONFIG_DVFS_DYNAMIC_LEVEL
	dvfs_unset_level(DVFS_LEVEL_HIGH_PERFORMANCE, "ota");
#endif
    sys_pm_reboot(0); //reboot
}

static void ota_app_main(void *p1, void *p2, void *p3)
{
	struct app_msg msg = {0};
	bool terminaltion = false;

	SYS_LOG_INF("%s, Enter loop", __func__);

	while (!terminaltion) {
        SYS_LOG_INF("%s, %d", __func__, __LINE__);
		if (receive_msg(&msg, thread_timer_next_timeout())) {
			int result = 0;
            
			switch (msg.type) {
			case MSG_START_APP:
				break;

			case MSG_EXIT_APP:
				SYS_LOG_INF("ota application exit!");
				terminaltion = true;
				app_manager_thread_exit(APP_ID_OTA);
				break;
                
            case MSG_INPUT_EVENT:
                SYS_LOG_INF("ota input event");
                if (msg.cmd == MSG_USBHID_STARTOTA) {
                    start_usbhid_upgrade();
                }
                break;
			default:
				SYS_LOG_ERR("unknown: 0x%x!", msg.type);
				break;
			}
			if (msg.callback != NULL)
				msg.callback(&msg, result, NULL);
		}

		if (!terminaltion) {
		    thread_timer_handle_expired();
		}
	}
}

APP_DEFINE(ota, share_stack_area, sizeof(share_stack_area), CONFIG_APP_PRIORITY,
	   FOREGROUND_APP, NULL, NULL, NULL,
	   ota_app_main, NULL);