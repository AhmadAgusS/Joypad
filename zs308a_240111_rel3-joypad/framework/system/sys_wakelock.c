/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system wakelock
 */

#define LOG_MODULE_CUSTOMER

#include <os_common_api.h>
#include <string.h>
#include <sys_manager.h>
#include <sys_wakelock.h>
#include <assert.h>

LOG_MODULE_REGISTER(sys_wakelock, LOG_LEVEL_INF);

struct wakelock_user_info_t {
	uint16_t ref_cnt;
};

struct wakelock_t {
	uint8_t wake_lock_type;
	uint16_t ref_cnt;
	struct wakelock_user_info_t user_info[MAX_WAKE_LOCK_USER];
	uint32_t free_timestamp;
};

struct wakelocks_manager_context_t {
	struct wakelock_t wakelocks[MAX_WAKE_LOCK_TYPE];
};

struct wakelocks_manager_context_t wakelocks_manager;

static os_sem wakelocks_sem;

static struct wakelock_t *_sys_wakelock_lookup(int wake_lock_type)
{
	for (int i = 0; i < MAX_WAKE_LOCK_TYPE; i++) {
		if (wakelocks_manager.wakelocks[i].wake_lock_type == wake_lock_type) {
			return &wakelocks_manager.wakelocks[i];
		}
	}
	return NULL;
}

int sys_wake_lock(int wake_lock_type)
{
	return sys_wake_lock_ext(wake_lock_type,APP_WAKE_LOCK_USER);
}

int sys_wake_lock_ext(int wake_lock_type, int wake_lock_user)
{
	int res = 0;
	struct wakelock_t *wakelock = NULL;

#ifdef CONFIG_SYS_IRQ_LOCK
	SYS_IRQ_FLAGS flags;
	sys_irq_lock(&flags);
#else
	int key = os_irq_lock();
#endif

	wakelock = _sys_wakelock_lookup(wake_lock_type);
	if (!wakelock) {
		SYS_LOG_ERR("err, holder: %d", wake_lock_type);
		res = -EEXIST;
		goto exit;
	}

	assert(wakelock->ref_cnt < 0xFFFF);

	wakelock->ref_cnt++;
	wakelock->user_info[wake_lock_user].ref_cnt++;
	wakelock->free_timestamp = 0;
exit:

#ifdef CONFIG_SYS_IRQ_LOCK 
	sys_irq_unlock(&flags);
#else
	os_irq_unlock(key);
#endif

	return res;	
}

int sys_wake_unlock(int wake_lock_type)
{
	return sys_wake_unlock_ext(wake_lock_type, APP_WAKE_LOCK_USER);
}

int sys_wake_unlock_ext(int wake_lock_type, int wake_lock_user)
{
	int res = 0;
	struct wakelock_t *wakelock = NULL;

#ifdef CONFIG_SYS_IRQ_LOCK 
	SYS_IRQ_FLAGS flags;
	sys_irq_lock(&flags);
#else
	int key = os_irq_lock();
#endif

	wakelock = _sys_wakelock_lookup(wake_lock_type);
	if (!wakelock) {
		SYS_LOG_ERR("err, holder: %d", wake_lock_type);
		res = -ESRCH;
		goto exit;
	}

	assert(wakelock->ref_cnt > 0);

	wakelock->user_info[wake_lock_user].ref_cnt--;

	if (wakelock->ref_cnt--) {
		/** reset all wake lock timestamp */
		if (wake_lock_type == FULL_WAKE_LOCK) {
			for (int i = 0; i < MAX_WAKE_LOCK_TYPE; i++) {
				wakelocks_manager.wakelocks[i].free_timestamp = os_uptime_get_32();
			}
		} else {
			wakelock->free_timestamp = os_uptime_get_32();
		}
	}

	if (wake_lock_type == FULL_WAKE_LOCK) {
#ifdef CONFIG_SYS_STANDBY
		system_clear_fast_standby();
#endif
	}

exit:

	if (wakelock->ref_cnt == 0) {
		sys_wakelocks_wake(wake_lock_type);
	}
	
#ifdef CONFIG_SYS_IRQ_LOCK
	sys_irq_unlock(&flags);
#else
	os_irq_unlock(key);
#endif

	return res;
}

void sys_wakelocks_dump(void)
{
	struct wakelock_t *wakelock = NULL;

	os_printk("wlock info: \n");
	wakelock = _sys_wakelock_lookup(PARTIAL_WAKE_LOCK);
	os_printk("type : PARTIAL_WAKE_LOCK	(%d)\n",wakelock->ref_cnt);
	os_printk("	BT_WAKE_LOCK_USER    :%d\n",wakelock->user_info[BT_WAKE_LOCK_USER].ref_cnt);
	os_printk("	MEDIA_WAKE_LOCK_USER :%d\n",wakelock->user_info[MEDIA_WAKE_LOCK_USER].ref_cnt);
	os_printk("	USB_WAKE_LOCK_USER   :%d\n",wakelock->user_info[USB_WAKE_LOCK_USER].ref_cnt);
	os_printk("	POWER_WAKE_LOCK_USER :%d\n",wakelock->user_info[POWER_WAKE_LOCK_USER].ref_cnt);
	os_printk("	SYS_WAKE_LOCK_USER   :%d\n",wakelock->user_info[SYS_WAKE_LOCK_USER].ref_cnt);
	os_printk("	APP_WAKE_LOCK_USER   :%d\n",wakelock->user_info[APP_WAKE_LOCK_USER].ref_cnt);

	wakelock = _sys_wakelock_lookup(FULL_WAKE_LOCK);

	os_printk("type : FULL_WAKE_LOCK total	(%d)\n",wakelock->ref_cnt);
	os_printk("	BT_WAKE_LOCK_USER   : %d\n",wakelock->user_info[BT_WAKE_LOCK_USER].ref_cnt);
	os_printk("	MEDIA_WAKE_LOCK_USER: %d\n",wakelock->user_info[MEDIA_WAKE_LOCK_USER].ref_cnt);
	os_printk("	USB_WAKE_LOCK_USER  : %d\n",wakelock->user_info[USB_WAKE_LOCK_USER].ref_cnt);
	os_printk("	POWER_WAKE_LOCK_USER: %d\n",wakelock->user_info[POWER_WAKE_LOCK_USER].ref_cnt);
	os_printk("	SYS_WAKE_LOCK_USER  : %d\n",wakelock->user_info[SYS_WAKE_LOCK_USER].ref_cnt);
	os_printk("	APP_WAKE_LOCK_USER  : %d\n",wakelock->user_info[APP_WAKE_LOCK_USER].ref_cnt);
}

int sys_wakelocks_init(void)
{
	int current_timestamp = os_uptime_get_32();

	memset(&wakelocks_manager, 0 , sizeof(struct wakelocks_manager_context_t));
	os_sem_init(&wakelocks_sem, 0, 1);

	for (int i = 0; i < MAX_WAKE_LOCK_TYPE; i++) {
		wakelocks_manager.wakelocks[i].wake_lock_type = i;
		wakelocks_manager.wakelocks[i].free_timestamp = current_timestamp;
		wakelocks_manager.wakelocks[i].ref_cnt = 0;
	}

	/** only for debug*/
	sys_wake_lock(FULL_WAKE_LOCK);

	return 0;
}


int sys_wakelocks_check(int wake_lock_type)
{
	int ref_cnt = 0;
	struct wakelock_t *wakelock = NULL;
#ifdef CONFIG_SYS_IRQ_LOCK 
	SYS_IRQ_FLAGS flags;
	sys_irq_lock(&flags);
#else
	int key = os_irq_lock();
#endif

	wakelock = _sys_wakelock_lookup(wake_lock_type);
	if (!wakelock) {
		SYS_LOG_ERR("err, holder: %d", wake_lock_type);
		goto exit;
	}

	ref_cnt =  wakelock->ref_cnt;

exit:
#ifdef CONFIG_SYS_IRQ_LOCK 
	sys_irq_unlock(&flags);
#else
	os_irq_unlock(key);
#endif

	return ref_cnt;
}

uint32_t sys_wakelocks_get_free_time(int wake_lock_type)
{
	int free_time = 0;
	struct wakelock_t *wakelock = NULL;
#ifdef CONFIG_SYS_IRQ_LOCK 
	SYS_IRQ_FLAGS flags;
	sys_irq_lock(&flags);
#else
	int key = os_irq_lock();
#endif

	wakelock = _sys_wakelock_lookup(wake_lock_type);
	if (!wakelock) {
		SYS_LOG_ERR("err, holder: %d", wake_lock_type);
		goto exit;
	}

	if (!wakelock->ref_cnt) {
		free_time = os_uptime_get_32() - wakelock->free_timestamp;
	}

exit:

#ifdef CONFIG_SYS_IRQ_LOCK 
	sys_irq_unlock(&flags);
#else
	os_irq_unlock(key);
#endif

	return free_time;
}


int sys_wakelocks_wait(int wake_lock_type)
{
#ifndef CONFIG_SIMULATOR
	ARG_UNUSED(wake_lock_type);
#endif
	os_sem_take(&wakelocks_sem, OS_FOREVER);
	return 0;
}

int sys_wakelocks_wake(int wake_lock_type)
{
#ifndef CONFIG_SIMULATOR
	ARG_UNUSED(wake_lock_type);
#endif
	os_sem_give(&wakelocks_sem);
	return 0;
}

