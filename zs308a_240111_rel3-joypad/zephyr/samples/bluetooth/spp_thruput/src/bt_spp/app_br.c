/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_br.h"
#include "rmc_timer.h"
#include "app_utils.h"
#include "system_app.h"
#include <acts_bluetooth/host_interface.h>

#define TARGET_DEV_NAME		CONFIG_BT_DEVICE_NAME
#define USE_LIAC			(0)
#define STANDARD_SPP		(0)


enum
{
	SPP_STATE_IDLE,
	SPP_STATE_WAIT_CONNECT,
	SPP_STATE_ACTIVE_CONNECT,
};

struct br_dev_manager_t {
	struct bt_conn *conn;
	bt_addr_t addr;
	u8_t type;
	u8_t master;
	u8_t acl_connected;
	u8_t spp_channel;
};
#if (STANDARD_SPP)
static const u8_t app_spp_uuid_reg[16] = {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,	\
										0x00, 0x10, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00};

static const u8_t app_spp_uuid_connect[16] = {0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00,	\
										0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

static u8_t test_data[]=
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

#else
static const u8_t app_spp_uuid_reg[16] = {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,	\
										0x00, 0x10, 0x00, 0x00, 0x66, 0x66, 0x00, 0x00};

static const u8_t app_spp_uuid_connect[16] = {0x00, 0x00, 0x66, 0x66, 0x00, 0x00, 0x10, 0x00,	\
										0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

static u8_t test_data[]=
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
};
static u8_t g_spp_tx_cnt=0;
#endif


static u8_t app_spp_state = SPP_STATE_IDLE;

static struct br_dev_manager_t g_brdev_info[CONFIG_BT_MAX_BR_CONN];

static os_delayed_work g_spp_tx_work;
static os_delayed_work g_spp_tx_thruput;
static int32_t g_spp_tx_interval = 3000;
static unsigned int g_pkt_size = 10;
static u8_t g_spp_loopback = 1;
static int32_t g_rx_cnt = 0;
static int32_t g_tx_cnt = 0;

static struct br_dev_manager_t *_find_free_brdev(void)
{
	int i;
	for (i = 0; i < CONFIG_BT_MAX_BR_CONN; i++) {
		SYS_LOG_INF("[%d] conn %p", i, g_brdev_info[i].conn);
		if (g_brdev_info[i].conn == NULL) {
			return &g_brdev_info[i];
		}
	}
	return NULL;
}

static struct br_dev_manager_t *_find_brdev_by_conn(struct bt_conn *conn)
{
	int i;
	for (i = 0; i < CONFIG_BT_MAX_BR_CONN; i++) {
		if (g_brdev_info[i].conn == conn) {
			return &g_brdev_info[i];
		}
	}
	return NULL;
}

static struct br_dev_manager_t *_find_brdev_by_addr(bt_addr_t *addr)
{
	int i;
	for (i = 0; i < CONFIG_BT_MAX_BR_CONN; i++) {
		if (!memcmp(&g_brdev_info[i].addr, addr, sizeof(bt_addr_t))){
			return &g_brdev_info[i];
		}
	}
	return NULL;
}

static void _clear_brdev(struct br_dev_manager_t *dev)
{
	memset(dev, 0, sizeof(struct br_dev_manager_t));
}

static struct bt_conn *_find_connected_brdev(void)
{
	int i;
	for (i = 0; i < CONFIG_BT_MAX_BR_CONN; i++) {
		if (g_brdev_info[i].conn) {
			return g_brdev_info[i].conn;
		}
	}
	return NULL;
}

static void _clear_brdev_acl_connecting(void)
{
	int i;
	for (i = 0; i < CONFIG_BT_MAX_BR_CONN; i++) {
		if (g_brdev_info[i].conn && g_brdev_info[i].acl_connected == 0) {
			memset(&g_brdev_info[i], 0, sizeof(struct br_dev_manager_t));
			return;
		}
	}
}


static void app_br_send_msg_to_app(u8_t event_code, void *event_data)
{
	app_send_async_msg2(APP_ID_MAIN, MSG_BR_EVENT, event_code, event_data, 0);
	SYS_LOG_INF("event_code %d", event_code);
}

static void app_br_connected(struct bt_conn *conn, u8_t conn_err)
{
	char addr[BT_ADDR_STR_LEN];
	struct bt_conn_info info;
	if(!conn || hostif_bt_conn_get_info(conn, &info) || info.type != BT_CONN_TYPE_BR)
		return;

	bt_addr_to_str(info.br.dst, addr, sizeof(addr));
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	/* stop connect timer */
	if(dev_info)
	{
		rmc_timer_stop(ID_BR_CONNECT_TIMEOUT);
	}

	if (conn_err)
	{
		SYS_LOG_ERR("Failed to connect to %s (%x)", addr, conn_err);
		/* send a message to main thread */
		app_br_send_msg_to_app(BR_START_DISCOVER, 0);
		if(dev_info)
		{
			_clear_brdev(dev_info);
		}
		return;
	}
	if(!dev_info)
	{
		dev_info = _find_free_brdev();
		if(!dev_info)
		{
			hostif_bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			hostif_bt_conn_unref(conn);
			return;
		}
		_clear_brdev(dev_info);
		dev_info->conn = conn;
		memcpy(&dev_info->addr, &info.br.dst, sizeof(bt_addr_t));
	}
	SYS_LOG_INF("BR Connected: %s,conn=%p, dev=%p\n", addr, conn, dev_info);
	/* send a message to main thread */
	hostif_bt_conn_ref(conn);
	app_br_send_msg_to_app(BR_ACL_CONNECTED_IND, conn);
	dev_info->acl_connected = 1;
	if(info.role == BT_HCI_ROLE_MASTER)
	{
		dev_info->master = true;
		int err = hostif_bt_spp_connect(conn, (u8_t *)app_spp_uuid_connect);
		if(err < 0)
		{
			SYS_LOG_INF("BR SPP Err:%d", err);
		}
		else
		{
			SYS_LOG_INF("BR SPP channel connecting:%d", err);
		}
	}
}

static void app_br_disconnected(struct bt_conn *conn, u8_t reason)
{
	char addr[BT_ADDR_STR_LEN];
	struct bt_conn_info info;
	if(!conn || hostif_bt_conn_get_info(conn, &info) || info.type != BT_CONN_TYPE_BR)
		return;

	bt_addr_to_str(info.br.dst, addr, sizeof(addr));
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	SYS_LOG_INF("BR Disconnected: %s (reason %x), conn=%p, dev=%p\n", addr, reason, conn, dev_info);

	hostif_bt_conn_unref(conn);
	if (dev_info) {
		_clear_brdev(dev_info);
		k_delayed_work_cancel(&g_spp_tx_work);
		k_delayed_work_cancel(&g_spp_tx_thruput);
	}

	/* send a message to main thread */
	app_br_send_msg_to_app(BR_ACL_DISCONNECTED_IND, conn);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = app_br_connected,
	.disconnected = app_br_disconnected,
	//.identity_resolved = le_identity_resolved,
	//.security_changed = security_changed,
	//.le_param_updated = le_param_updated,
};

static void app_spp_connect_failed_cb(struct bt_conn *conn, u8_t spp_id)
{
	SYS_LOG_INF("channel:%d\n", spp_id);
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if (dev_info)
	{
		dev_info->spp_channel = 0;
		app_spp_state = SPP_STATE_IDLE;
		SYS_LOG_INF("Back to SPP_STATE_IDLE\n");
	}
}

static void app_spp_connected_cb(struct bt_conn *conn, u8_t spp_id)
{
	SYS_LOG_INF("channel:%d\n", spp_id);
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if (dev_info)
	{
		dev_info->spp_channel = spp_id;
		app_spp_state = SPP_STATE_ACTIVE_CONNECT;
		app_br_send_msg_to_app(BR_SPP_CONNECTED_IND, conn);
		k_delayed_work_submit(&g_spp_tx_thruput, K_SECONDS(1));
		g_rx_cnt = 0;
		g_tx_cnt = 0;
		#if (!STANDARD_SPP)
		if(dev_info->master)
		#endif //STANDARD_SPP
		{
			SYS_LOG_INF("\n >>>>>>>MASTER!!!! \n");
			k_delayed_work_submit(&g_spp_tx_work, K_MSEC(g_spp_tx_interval));

		}
	}

}

static void app_spp_disconnected_cb(struct bt_conn *conn, u8_t spp_id)
{
	SYS_LOG_INF("channel:%d\n", spp_id);
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if (dev_info && dev_info->spp_channel == spp_id)
	{
		dev_info->spp_channel = 0;
		if (app_spp_state == SPP_STATE_ACTIVE_CONNECT)
		{
			app_spp_state = SPP_STATE_IDLE;
			SYS_LOG_INF("Back to SPP_STATE_IDLE\n");
		}
		app_br_send_msg_to_app(BR_SPP_DISCONNECTED_IND, conn);
		k_delayed_work_cancel(&g_spp_tx_work);
		k_delayed_work_cancel(&g_spp_tx_thruput);
	}
}

static void app_spp_receive_data_cb(struct bt_conn *conn, u8_t spp_id, u8_t *data, u16_t len)
{
	SYS_LOG_INF("Rx: channel:%d ,len %d byte", spp_id, len);
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if (dev_info && dev_info->spp_channel == spp_id)
	{
		g_rx_cnt += len;
		if(!dev_info->master)
		{
			if(data[0] & 0x80)
				app_br_tx((void *)data, len);
		}
		if(data[0] == 0x00 && data[1] == 0xFF)
		{
			if(data[2] == 0x01 && data[3] == 0x04)
			{
				u32_t interval = (((u32_t)data[4])<<24) | (((u32_t)data[5])<<16) | (((u32_t)data[6])<<8) |(((u32_t)data[7]));
				g_spp_tx_interval = interval;
				k_delayed_work_cancel(&g_spp_tx_work);
				k_delayed_work_submit(&g_spp_tx_work, K_MSEC(g_spp_tx_interval));
				SYS_LOG_INF("====New interval=%d", g_spp_tx_interval);
			}
			else if(data[2] == 0x02 && data[3] == 0x04)
			{
				u32_t psize = (((u32_t)data[4])<<24) | (((u32_t)data[5])<<16) | (((u32_t)data[6])<<8) |(((u32_t)data[7]));
				g_pkt_size = psize;
				SYS_LOG_INF("====New packet size=%d", g_pkt_size);
			}
			else if(data[2] == 0x03 && data[3] == 0x01)
			{
				g_spp_loopback = data[4];
				SYS_LOG_INF("====New loopback=%d", g_spp_loopback);
			}
		}
	}
}

static struct bt_spp_app_cb spp_app_cb = 
{
	.connect_failed = app_spp_connect_failed_cb,
	.connected = app_spp_connected_cb,
	.disconnected = app_spp_disconnected_cb,
	.recv = app_spp_receive_data_cb,
};


static void app_spp_tx_cb(struct k_work *work)
{
#if (!STANDARD_SPP)
	test_data[0] = g_spp_tx_cnt++;
	g_spp_tx_cnt &= 0x7F;

	if(g_spp_loopback)
		test_data[0] |=0x80;
#endif //STANDARD_SPP
	app_br_tx((void *)test_data, g_pkt_size);
	k_delayed_work_submit(&g_spp_tx_work, K_MSEC(g_spp_tx_interval));
}

static void app_spp_tx_thruput_cb(struct k_work *work)
{
	SYS_LOG_INF("Thruput clock=%d: Rx=%d bytes, Tx=%d bytes,", sys_clock_tick_get_32(), g_rx_cnt, g_tx_cnt);
	g_tx_cnt = 0;
	g_rx_cnt = 0;
	k_delayed_work_submit(&g_spp_tx_thruput, K_SECONDS(1));
}

static void app_br_discover_result(struct bt_br_discovery_result *cb_result)
{
	do
	{
		if (!cb_result) 
		{
			SYS_LOG_INF("Discover finish");
			break;
		}

		if (cb_result->len) 
		{
			uint8_t name_temp[50];
			uint8_t name_len_max = (cb_result->len > 50) ? 50 : cb_result->len;
			memset(name_temp, 0, sizeof(name_temp));
			memcpy(name_temp, cb_result->name, name_len_max);

			SYS_LOG_INF("MAC %02x:%02x:%02x:%02x:%02x:%02x, rssi %i, Name:%s",
			cb_result->addr.val[5], cb_result->addr.val[4], cb_result->addr.val[3],
			cb_result->addr.val[2], cb_result->addr.val[1], cb_result->addr.val[0],
			cb_result->rssi, name_temp);
		}
		else
		{
			SYS_LOG_INF("MAC %02x:%02x:%02x:%02x:%02x:%02x, rssi %i",
			cb_result->addr.val[5], cb_result->addr.val[4], cb_result->addr.val[3],
			cb_result->addr.val[2], cb_result->addr.val[1], cb_result->addr.val[0],
			cb_result->rssi);
		}

		if(cb_result->len < (strlen(TARGET_DEV_NAME)))
			break;

		if(memcmp(TARGET_DEV_NAME, cb_result->name, strlen(TARGET_DEV_NAME)))
			break;

		if(_find_brdev_by_addr(&cb_result->addr))
			return;

		struct br_dev_manager_t *dev_info = _find_free_brdev();
		if(!dev_info)
		{
			SYS_LOG_WRN("Link Full!!!");
			break;
		}

		app_br_stop_discover();

		struct bt_conn *conn = hostif_bt_conn_create_br(&cb_result->addr, BT_BR_CONN_PARAM_DEFAULT);
		if(conn)
		{
			hostif_bt_conn_unref(conn);
			SYS_LOG_INF("BR Connecting: conn=%p, dev=%p\n", conn, dev_info);
			_clear_brdev(dev_info);
			dev_info->conn = conn;
			memcpy(&dev_info->addr, &cb_result->addr, sizeof(bt_addr_t));
			rmc_timer_start(ID_BR_CONNECT_TIMEOUT);
		}
		else
		{
			app_br_send_msg_to_app(BR_START_DISCOVER, 0);
		}
	}while(0);
}

void app_br_start_discover(void)
{
	hostif_bt_br_write_scan_enable(3);
	struct bt_br_discovery_param param;
	param.length = 5;
	param.num_responses = 0;
	#if USE_LIAC
	param.limited = true;
	#else
	param.limited = false;
	#endif //USE_LIAC
	int err = hostif_bt_br_discovery_start(&param, app_br_discover_result);
	if (err) {
		SYS_LOG_ERR("Failed to start discovery:%u", err);
	}

}

void app_br_stop_discover(void)
{
	hostif_bt_br_write_scan_enable(0);
	int err = hostif_bt_br_discovery_stop();
	if (err) {
		SYS_LOG_ERR("Failed to stop discovery:%u ", err);
	}
}

bool is_bt_enabled = false;
bool app_br_init(void)
{
	if(!is_bt_enabled){
		//hostif_bt_init_class(0x00100000); //Object Transfer (v-InBox, v-Folder)
		int err = hostif_bt_enable(NULL);
		if (err) {
			SYS_LOG_INF("Bluetooth init failed (err %d)\n", err);
			return false;
		}
		SYS_LOG_INF("Bluetooth initialize success\n");
		is_bt_enabled = true;
	}
	memset(&g_brdev_info, 0 , sizeof(g_brdev_info));

	hostif_bt_conn_cb_register(&conn_callbacks);
	hostif_bt_spp_register_service((u8_t *)app_spp_uuid_reg);
	hostif_bt_spp_register_cb(&spp_app_cb);

	os_delayed_work_init(&g_spp_tx_work, app_spp_tx_cb);
	os_delayed_work_init(&g_spp_tx_thruput, app_spp_tx_thruput_cb);

	hostif_bt_br_write_inquiry_scan_activity(0x1000, 0x0012);
	hostif_bt_br_write_inquiry_scan_type(0);
	hostif_bt_br_write_page_scan_activity(0x0800, 0x0012);
	hostif_bt_br_write_page_scan_type(0);
	#if USE_LIAC
	hostif_bt_br_write_iac(true);
	#endif//USE_LIAC

	return true;
}

void app_br_event_handle(u8_t event_code, void *event_data)
{
	SYS_LOG_INF("%d", event_code);
	switch (event_code) {
		case BR_ACL_DISCONNECTED_IND:
		case BR_START_DISCOVER:
			app_br_start_discover();
			break;
		case BR_ACL_CONNECTED_IND:
			break;
		case BR_SPP_CONNECTED_IND:
			//app_usb_init_xinput();
			break;
		case BR_SPP_DISCONNECTED_IND:
			//app_usb_deinit_xinput();
			break;
	}
}

void app_br_create_conn_timeout_handler(void)
{
	//Todo Cancel connection
	_clear_brdev_acl_connecting();
	app_br_start_discover();
}

void app_br_tx(u8_t *buf, u16_t buf_len)
{
	struct bt_conn *conn = _find_connected_brdev();
	if(!conn)
		return;
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if(!dev_info || dev_info->spp_channel == 0)
		return;

	hostif_bt_spp_send_data(dev_info->spp_channel, buf, buf_len);
	g_tx_cnt += buf_len;
}


#ifdef CONFIG_SHELL
#include <shell/shell.h>
static int shell_cmd_appspp_pktsize(const struct shell *shell, size_t argc, char **argv)
{
	unsigned int pkt_size;

	if (argc < 2)
	{
		SYS_LOG_INF("appspp: argc %d failed", argc);
		return 0;
	}

	pkt_size = strtoul(argv[1], NULL, 10);
	if( pkt_size > sizeof(test_data))
	{
		SYS_LOG_INF("appspp: pkt_size %d failed %d", pkt_size, sizeof(test_data));
		return 0;
	}
	g_pkt_size = pkt_size;

	SYS_LOG_INF("appspp: pkt_size %d", g_pkt_size);

	struct bt_conn *conn = _find_connected_brdev();
	if(!conn)
		return 0;
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if(!dev_info || dev_info->spp_channel == 0 || dev_info->master)
		return 0;

	u8_t cmd[8] = {0x00, 0xFF, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
	cmd[4] = (g_pkt_size>>24);
	cmd[5] = (g_pkt_size>>16);
	cmd[6] = (g_pkt_size>>8);
	cmd[7] = (g_pkt_size);
	app_br_tx(cmd, sizeof(cmd));

	return 0;
}

static int shell_cmd_appspp_interval(const struct shell *shell, size_t argc, char **argv)
{
	unsigned int interval;

	if (argc < 2)
	{
		SYS_LOG_INF("appspp: argc %d failed", argc);
		return 0;
	}

	interval = strtoul(argv[1], NULL, 10);
	g_spp_tx_interval = interval;
	SYS_LOG_INF("appspp: interval %d ms", g_spp_tx_interval);

	struct bt_conn *conn = _find_connected_brdev();
	if(!conn)
		return 0;
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if(!dev_info || dev_info->spp_channel == 0 || dev_info->master)
		return 0;

	u8_t cmd[8] = {0x00, 0xFF, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00};
	u32_t ticks = g_spp_tx_interval;
	cmd[4] = (ticks>>24);
	cmd[5] = (ticks>>16);
	cmd[6] = (ticks>>8);
	cmd[7] = (ticks);
	app_br_tx(cmd, sizeof(cmd));

	return 0;
}

static int shell_cmd_appspp_loopback(const struct shell *shell, size_t argc, char **argv)
{
	if (argc < 2)
	{
		SYS_LOG_INF("appspp: argc %d failed", argc);
		return 0;
	}

	g_spp_loopback = atoi(argv[1]);

	SYS_LOG_INF("appspp: loopback %d", g_spp_loopback);

	struct bt_conn *conn = _find_connected_brdev();
	if(!conn)
		return 0;
	struct br_dev_manager_t *dev_info = _find_brdev_by_conn(conn);
	if(!dev_info || dev_info->spp_channel == 0 || dev_info->master)
		return 0;

	u8_t cmd[5] = {0x00, 0xFF, 0x03, 0x01, 0x00};
	cmd[4] = g_spp_loopback;
	app_br_tx(cmd, sizeof(cmd));

	return 0;
}

static int cmd_app_spp(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		shell_help(shell);
		return SHELL_CMD_HELP_PRINTED;
	}

	shell_error(shell, "%s unknown parameter: %s", argv[0], argv[1]);

	return -EINVAL;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_appspp_cmds,
	SHELL_CMD_ARG(psize, NULL, "set spp packet size", shell_cmd_appspp_pktsize, 2, 0),
	SHELL_CMD_ARG(int, NULL, "set spp packet interval", shell_cmd_appspp_interval, 2, 0),
	SHELL_CMD_ARG(loop, NULL, "set spp tx loopback", shell_cmd_appspp_loopback, 2, 0),
	SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(appspp, &sub_appspp_cmds, "AppSpp commands", cmd_app_spp);
#endif //CONFIG_SHELL


