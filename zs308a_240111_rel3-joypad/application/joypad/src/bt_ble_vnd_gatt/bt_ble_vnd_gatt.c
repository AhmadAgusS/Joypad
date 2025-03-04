/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <os_common_api.h>
#include <os_common_api.h> // #include <logging/sys_log.h> #2855
#include <input_manager.h>
#include <msg_manager.h>
#include <app_manager.h>
#include <srv_manager.h>
#include <bt_manager.h>
// #include <global_mem.h> 2855
#include <dvfs.h>
#include <app_defines.h>
#include "system_defs.h"


#ifdef CONFIG_BT_BLE_VENDER_GATT_APP

#define CFG_VND_UUID_IN_16				(0)

#define VND_BLE_BUF_MAX				(20)


#if (CFG_VND_UUID_IN_16)
	#define VND_BLE_SERVICE_UUID		BT_UUID_DECLARE_16(0xFFC0)
	#define VND_BLE_WRITE_UUID			BT_UUID_DECLARE_16(0xFFC1)
	#define VND_BLE_READ_UUID			BT_UUID_DECLARE_16(0xFFC2)
#else
	/*	"e49a25f8-f69a-11e8-8eb2-f2801f1b9fd1" reverse order  */
	#define VND_BLE_SERVICE_UUID BT_UUID_DECLARE_128( \
					0xd1, 0x9f, 0x1b, 0x1f, 0x80, 0xf2, 0xb2, 0x8e, \
					0xe8, 0x11, 0x9a, 0xf6, 0xf8, 0x25, 0x9a, 0xe4)

	/* "e49a25e0-f69a-11e8-8eb2-f2801f1b9fd2" reverse order */
	#define VND_BLE_WRITE_UUID BT_UUID_DECLARE_128( \
					0xd2, 0x9f, 0x1b, 0x1f, 0x80, 0xf2, 0xb2, 0x8e, \
					0xe8, 0x11, 0x9a, 0xf6, 0xe0, 0x25, 0x9a, 0xe4)

	/* "e49a28e1-f69a-11e8-8eb2-f2801f1b9fd3" reverse order */
	#define VND_BLE_READ_UUID BT_UUID_DECLARE_128( \
					0xd3, 0x9f, 0x1b, 0x1f, 0x80, 0xf2, 0xb2, 0x8e, \
					0xe8, 0x11, 0x9a, 0xf6, 0xe1, 0x28, 0x9a, 0xe4)
#endif //CFG_VND_UUID_IN_16

static ssize_t vnd_gatt_write_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, u16_t len, u16_t offset, u8_t flags);
static void vnd_gatt_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value);

static u8_t g_vnd_gatt_notify_enable = 0;
// static struct bt_gatt_ccc_cfg g_vnd_ccc_cfg[1];
static u8_t vnd_data_buf[VND_BLE_BUF_MAX];

static struct bt_gatt_attr ble_vnd_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(VND_BLE_SERVICE_UUID),

	BT_GATT_CHARACTERISTIC(VND_BLE_WRITE_UUID,
						   BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
						   BT_GATT_PERM_WRITE,
						   NULL,
						   vnd_gatt_write_cb,
						   NULL),
	BT_GATT_DESCRIPTOR(VND_BLE_WRITE_UUID, BT_GATT_PERM_WRITE,
			   NULL, vnd_gatt_write_cb, NULL),

	BT_GATT_CHARACTERISTIC(VND_BLE_READ_UUID, BT_GATT_CHRC_NOTIFY,		/* or BT_GATT_CHRC_INDICATE */
						   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
						   NULL,
						   NULL,
						   NULL),
	BT_GATT_DESCRIPTOR(VND_BLE_READ_UUID, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   NULL, NULL, NULL),
	BT_GATT_CCC(vnd_gatt_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
};

static ssize_t vnd_gatt_write_cb( struct bt_conn *conn, const struct bt_gatt_attr *attr,
								  const void *buf, u16_t len, u16_t offset,u8_t flags)
{
	SYS_LOG_INF("cccd: %d", g_vnd_gatt_notify_enable);
	if(g_vnd_gatt_notify_enable)
	{
		u16_t cpy_len = ((len + offset) > VND_BLE_BUF_MAX) ? (VND_BLE_BUF_MAX - offset) : len;
		memcpy(&vnd_data_buf[offset], buf, cpy_len);
		int err = bt_manager_ble_send_data(&ble_vnd_attrs[3], &ble_vnd_attrs[4], vnd_data_buf, (offset+cpy_len));
		SYS_LOG_INF("notify: %d", err);
		return cpy_len;
	}
	return (BT_GATT_ERR(BT_ATT_ERR_WRITE_NOT_PERMITTED));
}


static void vnd_gatt_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	SYS_LOG_INF("value: %d", value);
	g_vnd_gatt_notify_enable = (u8_t)value;
}

static void ble_vnd_connect_cb(u8_t *mac, u8_t connected)
{
	SYS_LOG_INF("BLE %s", connected ? "connected" : "disconnected");
	SYS_LOG_INF("MAC %2x:%2x:%2x:%2x:%2x:%2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static struct ble_reg_manager ble_vnd_mgr = {
	.link_cb = ble_vnd_connect_cb,
};

void bt_ble_vnd_gatt_init(void)
{
	ble_vnd_mgr.gatt_svc.attrs = ble_vnd_attrs;
	ble_vnd_mgr.gatt_svc.attr_count = ARRAY_SIZE(ble_vnd_attrs);
	bt_manager_ble_service_reg(&ble_vnd_mgr);
}

#endif //CONFIG_BT_BLE_VENDER_GATT_APP