/* att.c - Attribute protocol handling */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>

#include <acts_bluetooth/hci.h>
#include <acts_bluetooth/bluetooth.h>
#include <acts_bluetooth/uuid.h>
#include <acts_bluetooth/gatt.h>
#include <drivers/bluetooth/hci_driver.h>

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_DEBUG_ATT)
#define LOG_MODULE_NAME si_bt_att
#include "common/log.h"

#include "hci_core.h"
#include "conn_internal.h"
#include "l2cap_internal.h"
#include "smp.h"
#include "att_internal.h"
#include "gatt_internal.h"


#define SI_BT_ATT_DBG_ENABLED (0) //[Si-Xpandas]
#define SI_BT_SMP_DBG_ENABLED (0) //[Si-Xpandas]

#if (SI_BT_ATT_DBG_ENABLED || SI_BT_SMP_DBG_ENABLED)
static void bt_conn_addr_str(struct bt_conn *conn, char *addr, size_t len)
{
	struct bt_conn_info info;
	addr[0] = '\0';
	if (bt_conn_get_info(conn, &info) < 0) {
		return;
	}

	switch (info.type) {
#if defined(CONFIG_BT_BREDR)
	case BT_CONN_TYPE_BR:
	case BT_CONN_TYPE_SCO:
		bt_addr_to_str(info.br.dst, addr, len);
		break;
#endif
	case BT_CONN_TYPE_LE:
		bt_addr_le_to_str(info.le.dst, addr, len);
		break;
	}
}
#endif //(SI_BT_ATT_DBG_ENABLED || SI_BT_SMP_DBG_ENABLED)

#if SI_BT_ATT_DBG_ENABLED
//#include <bluetooth/host_interface.h>
static const struct att_dbg_handler {
	u8_t       op;
	u8_t       min_len_check;
	char       print_str[16];
} g_dbg_handlers[] = {
	{	BT_ATT_OP_ERROR_RSP,			5,	"ERROR_RSP",		},
	{	BT_ATT_OP_MTU_REQ,				3,	"MTU_REQ",			},
	{	BT_ATT_OP_MTU_RSP,				3,	"MTU_RSP",			},
	{	BT_ATT_OP_FIND_INFO_REQ,		5,	"FIND_INFO_REQ",	},
	{	BT_ATT_OP_FIND_INFO_RSP,		6,	"FIND_INFO_RSP",	},
	{	BT_ATT_OP_FIND_TYPE_REQ,		7,	"FIND_TYPE_REQ",	},
	{	BT_ATT_OP_FIND_TYPE_RSP,		5,	"FIND_TYPE_RSP",	},
	{	BT_ATT_OP_READ_TYPE_REQ,		7,	"READ_TYPE_REQ",	},
	{	BT_ATT_OP_READ_TYPE_RSP,		4,	"READ_TYPE_RSP",	},
	{	BT_ATT_OP_READ_REQ,				3,	"READ_REQ",			},
	{	BT_ATT_OP_READ_RSP,				1,	"READ_RSP",			},
	{	BT_ATT_OP_READ_BLOB_REQ,		5,	"READ_BLOB_REQ",	},
	{	BT_ATT_OP_READ_BLOB_RSP,		1,	"READ_BLOB_RSP",	},
	{	BT_ATT_OP_READ_MULT_REQ,		5,	"READ_MULT_REQ",	},
	{	BT_ATT_OP_READ_MULT_RSP,		1,	"READ_MULT_RSP",	},
	{	BT_ATT_OP_READ_GROUP_REQ,		7,	"READ_GROUP_REQ",	},
	{	BT_ATT_OP_READ_GROUP_RSP,		6,	"READ_GROUP_RSP",	},
	{	BT_ATT_OP_WRITE_REQ,			3,	"WRITE_REQ",		},
	{	BT_ATT_OP_WRITE_RSP,			1,	"WRITE_RSP",		},
	{	BT_ATT_OP_PREPARE_WRITE_REQ,	5,	"PRE_WRITE_REQ",	},
	{	BT_ATT_OP_PREPARE_WRITE_RSP,	5,	"PRE_WRITE_RSP",	},
	{	BT_ATT_OP_EXEC_WRITE_REQ,		2,	"EXE_WRITE_REQ",	},
	{	BT_ATT_OP_EXEC_WRITE_RSP,		1,	"EXE_WRITE_RSP",	},
	{	BT_ATT_OP_NOTIFY,				3,	"NOTIFY",			},
	{	BT_ATT_OP_INDICATE,				3,	"INDICATE",			},
	{	BT_ATT_OP_CONFIRM,				1,	"CONFIRM",			},
	{	BT_ATT_OP_WRITE_CMD,			3,	"WRITE_CMD",		},
	{	BT_ATT_OP_SIGNED_WRITE_CMD,		15,	"SIGN_WRITE_CMD",	},
};

enum{
	ATT_DBG_RX,
	ATT_DBG_TX,
};

static const char att_pkt_rx[]  = "R";
static const char att_pkt_tx[]  = "T";
static const char att_unknow[]  = "UNKNOWN";

static const struct att_dbg_err_str_t {
	char       print_str[16];
} att_dbg_err_code[] ={
	{"InvalidHdl"},
	{"RdNotPermit"},
	{"WrNotPermit"},
	{"InvalidPDU"},
	{"InsufAuthen"},
	{"NotSupport"},
	{"InvalidOffset"},
	{"InsufAuthorz"},
	{"QueueFull"},
	{"AttrNotFound"},
	{"AttrNotLong"},
	{"EncKeyShort"},
	{"InvalidLen"},
	{"Err"},
	{"InsufEnc"},
	{"UnsupportGrp"},
	{"InsufResource"},
	{"DBOutSync"},
	{"NotAllow"},
};

static const char*att_dbg_get_err_code_str(u8_t err)
{
	if(err >= 0x80 && err <= 0x9F)
		return "AppErr";
	if(err >= 1 && err <= ARRAY_SIZE(att_dbg_err_code))
		return att_dbg_err_code[err-1].print_str;
	if(err >= 0xE0 && err <= 0xFF)
		return "ProfileServiceErr";
	return "RFU";
}

static const char *att_dbg_opcode(u8_t op)
{
	for (int i = 0; i < ARRAY_SIZE(g_dbg_handlers); i++) {
		if (op == g_dbg_handlers[i].op) {
			return g_dbg_handlers[i].print_str;
		}
	}
	return att_unknow;
}

static bool att_dbg_min_len_check(u8_t op, u8_t len)
{
	for (int i = 0; i < ARRAY_SIZE(g_dbg_handlers); i++) {
		if (op == g_dbg_handlers[i].op) {
			return (len >= g_dbg_handlers[i].min_len_check);
		}
	}
	return false;
}

static char *att_dbg_uuid_str(u8_t *val)
{
	static char str[37];
	u32_t tmp1, tmp5;
	u16_t tmp0, tmp2, tmp3, tmp4;
	memcpy(&tmp0, &val[0], sizeof(tmp0));
	memcpy(&tmp1, &val[2], sizeof(tmp1));
	memcpy(&tmp2, &val[6], sizeof(tmp2));
	memcpy(&tmp3, &val[8], sizeof(tmp3));
	memcpy(&tmp4, &val[10], sizeof(tmp4));
	memcpy(&tmp5, &val[12], sizeof(tmp5));

	snprintk(str, 37, "%08x-%04x-%04x-%04x-%08x%04x", tmp5, tmp4, tmp3, tmp2, tmp1, tmp0);
	return str;
}

void ATTDbgLog(u8_t type, struct bt_conn *conn, struct net_buf *buf)
{
	const char *p_type;
	char addr[30];

	switch(type)
	{
		case ATT_DBG_RX:
			p_type = att_pkt_rx;
			break;
		case ATT_DBG_TX:
			p_type = att_pkt_tx;
			break;
		default:
			p_type = att_unknow;
			break;
	}

	memset(addr, 0, sizeof(addr));
	bt_conn_addr_str(conn, addr, sizeof(addr));

	bool len_check = att_dbg_min_len_check(buf->data[0], buf->len);
	BT_INFO("%s[%s]ATT_%s(0x%02x),len=%u(len check %s)", addr, p_type, att_dbg_opcode(buf->data[0]), buf->data[0], buf->len-1, (len_check)? "Pass" : "Fail!!");
	if(!len_check || buf->len == 1)
	{
		if(!len_check)
		{
			bt_hex(buf, buf->len);
		}
		return;
	}
	switch(buf->data[0])
	{
		case BT_ATT_OP_ERROR_RSP:
			BT_INFO("ErrReqOpcode:%s(0x%02x),AttrHdl:0x%04x,ErrCode:0x%02x(%s)", 
						att_dbg_opcode(buf->data[1]), 
						buf->data[1], 
						((u16_t)buf->data[2] | ((u16_t)buf->data[3]<<8)), 
						buf->data[4], att_dbg_get_err_code_str(buf->data[4]));
			break;
		case BT_ATT_OP_MTU_REQ:
		case BT_ATT_OP_MTU_RSP:
		{
			u16_t mtu = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			BT_INFO("MTU=0x%02x=%u", mtu, mtu);
			break;
		}
		case BT_ATT_OP_FIND_INFO_REQ:
		{
			u16_t startHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			u16_t endHdl = (u16_t)buf->data[3] | ((u16_t)buf->data[4]<<8);
			BT_INFO("StartHdl:0x%04x,EndHdl:0x%04x", startHdl, endHdl);
			break;
		}
		case BT_ATT_OP_FIND_INFO_RSP:
		{
			if(buf->data[1] != 1 && buf->data[1] != 2)
				break;

			u8_t len_t = buf->len-2;
			u8_t pair_len = (buf->data[1] == 1)? 4 : 18;
			if(len_t % pair_len)
			{
				BT_INFO("Information Data Length Error:%d %d, Val:%s", len_t, pair_len, bt_hex(&buf->data[1], buf->len-1));
				break;
			}
			u8_t offset = 2;
			u8_t cnt = 1;
			u8_t total_cnt = len_t/pair_len;
			while(len_t)
			{
				u16_t Hdl = (u16_t)buf->data[offset] | ((u16_t)buf->data[offset+1]<<8);
				if(buf->data[1] == 1)
				{
					u16_t uuid16 = (u16_t)buf->data[offset+2] | ((u16_t)buf->data[offset+3]<<8);
					BT_INFO("[%u-%u]Format:0x01(UUID16),Hdl=0x%04x,UUID:0x%04x", cnt, total_cnt, Hdl, uuid16);
				}
				else if(buf->data[1] == 2)
				{
					BT_INFO("[%u-%u]Format:0x02(UUID128),Hdl:0x%04x,Val:%s", cnt, total_cnt, Hdl, att_dbg_uuid_str(&buf->data[offset+2]));
				}
				len_t -= pair_len;
				offset += pair_len;
				cnt++;
			}
			break;
		}
		case BT_ATT_OP_FIND_TYPE_REQ:
		{
			u16_t startHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			u16_t endHdl = (u16_t)buf->data[3] | ((u16_t)buf->data[4]<<8);
			u16_t attrType = (u16_t)buf->data[5] | ((u16_t)buf->data[6]<<8);
			BT_INFO("StartHdl:0x%04x,EndHdl:0x%04x,AttrType:0x%04x,AttrVal:%s", startHdl, endHdl, attrType, bt_hex(&buf->data[7], buf->len-7));
			break;
		}
		case BT_ATT_OP_FIND_TYPE_RSP:
		{
			u8_t len_t = buf->len-1;
			u8_t offset = 0;
			u8_t cnt = 1;
			u8_t total_cnt = len_t/4;

			if(len_t%4)
			{
				BT_INFO("Handle Information Len Error:%d , Val:%s", len_t, bt_hex(&buf->data[1], len_t));
				break;
			}
			while(len_t)
			{
				u16_t startHdl = (u16_t)buf->data[1+offset] | ((u16_t)buf->data[2+offset]<<8);
				u16_t endHdl = (u16_t)buf->data[3+offset] | ((u16_t)buf->data[4+offset]<<8);
				BT_INFO("[%u-%u]FoundAttrHdl:0x%04x,EndHdl:0x%04x", cnt, total_cnt, startHdl, endHdl);
				len_t -= 4;
				offset += 4;
				cnt++;
			}
			break;
		}
		case BT_ATT_OP_READ_TYPE_REQ:
		case BT_ATT_OP_READ_GROUP_REQ:
		{
			u16_t startHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			u16_t endHdl = (u16_t)buf->data[3] | ((u16_t)buf->data[4]<<8);
			if(buf->len == 7)
			{
				u16_t uuid16 = (u16_t)buf->data[5] | ((u16_t)buf->data[6]<<8);
				BT_INFO("StartHdl:0x%04x,EndHdl:0x%04x,UUID16:0x%04x", startHdl, endHdl, uuid16);
			}
			else if(buf->len == 21)
			{
				BT_INFO("StartHdl:0x%04x,EndHdl:0x%04x,UUID128:%s", startHdl, endHdl, att_dbg_uuid_str(&buf->data[5]));
			}
			else
			{
				BT_INFO("Len Error:%d , Val:%s", buf->len-1, bt_hex(&buf->data[1], buf->len-1));
			}
			break;
		}
		case BT_ATT_OP_READ_TYPE_RSP:
		{
			u8_t len_t = buf->len-2;
			u8_t pair_len = buf->data[1];
			if(len_t % pair_len)
			{
				BT_INFO("Attr Data Len Error:%d %d, Val:%s", len_t, pair_len, bt_hex(&buf->data[1], buf->len-1));
				break;
			}
			u8_t offset = 2;
			u8_t cnt = 1;
			u8_t total_cnt = len_t/pair_len;
			while(len_t)
			{
				u16_t Hdl = (u16_t)buf->data[offset] | ((u16_t)buf->data[offset+1]<<8);
				BT_INFO("[%u-%u]Hdl:0x%04x,Val:%s", cnt, total_cnt, Hdl, bt_hex(&buf->data[offset+2], pair_len-2));
				len_t -= pair_len;
				offset += pair_len;
				cnt++;
			}
			break;
		}
		case BT_ATT_OP_READ_REQ:
		case BT_ATT_OP_READ_MULT_REQ:
		{
			u8_t offset = 1;
			u8_t cnt = 1;
			u8_t len_t = buf->len-1;
			u8_t total_cnt = len_t/2;
			if(len_t % 2)
			{
				BT_INFO("Len Error:%d, Val:%s", len_t, bt_hex(&buf->data[1], buf->len-1));
				break;
			}
			while(len_t)
			{
				u16_t attrHdl = (u16_t)buf->data[offset] | ((u16_t)buf->data[offset+1]<<8);
				BT_INFO("[%u-%u]AttrHdl:0x%04x", cnt, total_cnt, attrHdl);
				cnt++;
				len_t -= 2;
				offset += 2;
			}
			break;
		}
		case BT_ATT_OP_READ_RSP:
		case BT_ATT_OP_READ_BLOB_RSP:
		case BT_ATT_OP_READ_MULT_RSP:
		{
			u8_t len_t = buf->len-1;
			if(len_t)
			{
				BT_INFO("Val:%s", bt_hex(&buf->data[1], len_t));
			}
			break;
		}
		case BT_ATT_OP_READ_BLOB_REQ:
		{
			u16_t attrHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			BT_INFO("AttrHdl:0x%04x,ValOffset:%u", attrHdl, (u16_t)buf->data[3] | ((u16_t)buf->data[2]<<4));
			break;
		}
		case BT_ATT_OP_READ_GROUP_RSP:
		{
			u8_t len_t = buf->len-2;
			u8_t pair_len = buf->data[1];
			if(len_t % pair_len)
			{
				BT_INFO("Len Error:%d %d, Val:%s", len_t, pair_len, bt_hex(&buf->data[1], buf->len-1));
				break;
			}
			u8_t offset = 2;
			u8_t cnt = 1;
			u8_t total_cnt = len_t/pair_len;
			while(len_t)
			{
				u16_t Hdl = (u16_t)buf->data[offset] | ((u16_t)buf->data[offset+1]<<8);
				u16_t endHdl = (u16_t)buf->data[offset+2] | ((u16_t)buf->data[offset+3]<<8);
				BT_INFO("[%u-%u]AttrHdl:0x%04x,EndGrpHdl:0x%04x,Val:%s", cnt, total_cnt, Hdl, endHdl, bt_hex(&buf->data[offset+4], pair_len-4));
				len_t -= pair_len;
				offset += pair_len;
				cnt++;
			}
			break;
		}
		case BT_ATT_OP_WRITE_REQ:
		case BT_ATT_OP_WRITE_CMD:
		case BT_ATT_OP_NOTIFY:
		case BT_ATT_OP_INDICATE:
		{
			u16_t attrHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			BT_INFO("AttrHdl:0x%04x,Val:%s", attrHdl, bt_hex(&buf->data[3], buf->len-3));
			break;
		}
		case BT_ATT_OP_SIGNED_WRITE_CMD:
		{
			u16_t attrHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			BT_INFO("AttrHdl:0x%04x,Val:%s,SignVal:%s", attrHdl, bt_hex(&buf->data[3], buf->len-15), bt_hex(&buf->data[buf->len-12], 12));
			break;
		}
		case BT_ATT_OP_PREPARE_WRITE_REQ:
		case BT_ATT_OP_PREPARE_WRITE_RSP:
		{
			u16_t attrHdl = (u16_t)buf->data[1] | ((u16_t)buf->data[2]<<8);
			BT_INFO("AttrHdl:0x%04x,ValOffset:%u,Val:%s", attrHdl, (u16_t)buf->data[3] | ((u16_t)buf->data[2]<<4), bt_hex(&buf->data[5], buf->len-5));
			break;
		}
		case BT_ATT_OP_EXEC_WRITE_REQ:
		{
			BT_INFO("Flags:0x%02x(0=Cancel, 1=Write)", buf->data[1]);
			break;
		}
	}
}
#else
void ATTDbgLog(u8_t type, struct bt_conn *conn, struct net_buf *buf)
{
	
}
#endif //SI_BT_ATT_DBG_ENABLED


#if (SI_BT_SMP_DBG_ENABLED)
static const struct smp_dbg_handler {
    u8_t        op;
    char        print_str[16];
    u8_t        min_len_check;
} g_smp_dbg_handlers[] = {
    {BT_SMP_CMD_PAIRING_REQ,        "PAIRING_REQ",      sizeof(struct bt_smp_pairing)},
    {BT_SMP_CMD_PAIRING_RSP,        "PAIRING_RSP",      sizeof(struct bt_smp_pairing)},
    {BT_SMP_CMD_PAIRING_CONFIRM,    "PAIRING_CONFIRM",  sizeof(struct bt_smp_pairing_confirm)},
    {BT_SMP_CMD_PAIRING_RANDOM,     "PAIRING_RANDOM",   sizeof(struct bt_smp_pairing_random)},
    {BT_SMP_CMD_PAIRING_FAIL,       "PAIRING_FAIL",     sizeof(struct bt_smp_pairing_fail)},
    {BT_SMP_CMD_ENCRYPT_INFO,       "ENCRYPT_INFO",     sizeof(struct bt_smp_encrypt_info)},
    {BT_SMP_CMD_MASTER_IDENT,       "MASTER_IDENT",     sizeof(struct bt_smp_master_ident)},
    {BT_SMP_CMD_IDENT_INFO,         "IDENT_INFO",       sizeof(struct bt_smp_ident_info)},
    {BT_SMP_CMD_IDENT_ADDR_INFO,    "IDENT_ADDR_INFO",  sizeof(struct bt_smp_ident_addr_info)},
    {BT_SMP_CMD_SIGNING_INFO,       "SIGNING_INFO",     sizeof(struct bt_smp_signing_info)},
    {BT_SMP_CMD_SECURITY_REQUEST,   "SECURITY_REQUEST", sizeof(struct bt_smp_security_request)},
    {BT_SMP_CMD_PUBLIC_KEY,         "PUBLIC_KEY",       sizeof(struct bt_smp_public_key)},
    {BT_SMP_DHKEY_CHECK,            "DHKEY_CHECK",      sizeof(struct bt_smp_dhkey_check)},
};

enum{
    SMP_DBG_RX,
    SMP_DBG_TX,
};

static const char smp_pkt_rx[]  = "R";
static const char smp_pkt_tx[]  = "T";
static const char smp_unknow[]  = "UNKNOWN";

static const char io_str_display_only[] = "DISPLAY_ONLY";
static const char io_str_display_yesno[] = "DISPLAY_YESNO";
static const char io_str_keyboard_only[] = "KEYBOARD_YESNO";
static const char io_str_no_in_no_out[] = "NO_INPUT_OUTPUT";
static const char io_str_keyboard_display[] = "KEYBOARD_DISPLAY";


static const char err_passkey_fail[] = "PasskeyEntryFailed";
static const char err_oob_fail[] = "OOBNotAvailable";
static const char err_auth_fail[] = "AuthRequirement";
static const char err_cfm_fail[] = "ConfirmFailed";
static const char err_pairing_fail[] = "PairingNotSupport";
static const char err_enc_fail[] = "EncKeySize";
static const char err_not_fail[] = "CmdNotSupport";
static const char err_unspecified_fail[] = "UnspecifiedReason";
static const char err_repeat_fail[] = "RepeatedAttempt";
static const char err_invalid_fail[] = "InvalidParameters";
static const char err_dhkey_fail[] = "DHKeyCheckFailed";
static const char err_numeric_fail[] = "NumericComparisonFailed";
static const char err_br_fail[] = "BREDRPairing";
static const char err_transport_fail[] = "CrossTransportKey";

static const char *smp_dbg_opcode(u8_t op)
{
    for (int i = 0; i < ARRAY_SIZE(g_smp_dbg_handlers); i++) {
        if (op == g_smp_dbg_handlers[i].op) {
            return g_smp_dbg_handlers[i].print_str;
        }
    }
    return smp_unknow;
}

static bool smp_dbg_min_len_check(u8_t op, u8_t len)
{
    for (int i = 0; i < ARRAY_SIZE(g_smp_dbg_handlers); i++) {
        if (op == g_smp_dbg_handlers[i].op) {
            return (len >= g_smp_dbg_handlers[i].min_len_check);
        }
    }
    return false;
}

static const char *smp_get_io_str(u8_t io)
{
    switch(io)
    {
        case BT_SMP_IO_DISPLAY_ONLY:
            return io_str_display_only;
        case BT_SMP_IO_DISPLAY_YESNO:
            return io_str_display_yesno;
        case BT_SMP_IO_KEYBOARD_ONLY:
            return io_str_keyboard_only;
        case BT_SMP_IO_NO_INPUT_OUTPUT:
            return io_str_no_in_no_out;
        case BT_SMP_IO_KEYBOARD_DISPLAY:
            return io_str_keyboard_display;
    }
    return smp_unknow;
}


void SMPDbgLog(u8_t type, struct bt_conn *conn, struct net_buf *buf)
{
    const char *p_type;
    char addr[30];

    switch(type)
    {
        case SMP_DBG_RX:
            p_type = smp_pkt_rx;
            break;
        case SMP_DBG_TX:
            p_type = smp_pkt_tx;
            break;
        default:
            p_type = smp_unknow;
            break;
    }

    memset(addr, 0, sizeof(addr));
    bt_conn_addr_str(conn, addr, sizeof(addr));

    bool len_check = smp_dbg_min_len_check(buf->data[0], buf->len);
    BT_INFO("%s[%s]SMP_%s(0x%02x),len=%u(len check %s)", addr, p_type, smp_dbg_opcode(buf->data[0]), buf->data[0], buf->len-1, (len_check)? "Pass" : "Fail!!");
    if(!len_check || buf->len == 1)
    {
        if(!len_check)
        {
            bt_hex(buf, buf->len);
        }
        return;
    }

    switch(buf->data[0])
    {
        case BT_SMP_CMD_PAIRING_REQ:
        case BT_SMP_CMD_PAIRING_RSP:
        {
            struct bt_smp_pairing *req = (void *)&buf->data[1];
            BT_INFO("IOCap:%s(0x%02x),OOB:%d,MaxKeySize:%d=0x%02x(7-16)", 
                        smp_get_io_str(req->io_capability), 
                        req->io_capability, req->oob_flag, 
                        req->max_key_size, req->max_key_size);
            BT_INFO("AuthReq:0x%02x(Bonding=%d,MITM=%d,SC=%d,KeyPress=%d,CT2=%d)", 
                        req->auth_req, 
                        (req->auth_req & BT_SMP_AUTH_BONDING)? 1: 0, 
                        (req->auth_req & BT_SMP_AUTH_MITM)? 1: 0, 
                        (req->auth_req & BT_SMP_AUTH_SC)? 1: 0,
                        (req->auth_req & BT_SMP_AUTH_KEYPRESS)? 1: 0,
                        (req->auth_req & BT_SMP_AUTH_CT2)? 1: 0);
            BT_INFO("InitDiscKey:0x%02x(EncKey=%d,IdKey=%d,SignKey=%d,LinkKey=%d)", 
                        req->init_key_dist, 
                        (req->init_key_dist & BT_SMP_DIST_ENC_KEY)? 1: 0, 
                        (req->init_key_dist & BT_SMP_DIST_ID_KEY)? 1: 0,
                        (req->init_key_dist & BT_SMP_DIST_SIGN)? 1: 0, 
                        (req->init_key_dist & BT_SMP_DIST_LINK_KEY)? 1: 0);
            BT_INFO("RespDiscKey:0x%02x(EncKey=%d,IdKey=%d,SignKey=%d,LinkKey=%d)", 
                        req->resp_key_dist, 
                        (req->resp_key_dist & BT_SMP_DIST_ENC_KEY)? 1: 0, 
                        (req->resp_key_dist & BT_SMP_DIST_ID_KEY)? 1: 0,
                        (req->resp_key_dist & BT_SMP_DIST_SIGN)? 1: 0, 
                        (req->resp_key_dist & BT_SMP_DIST_LINK_KEY)? 1: 0);
            break;
        }
        case BT_SMP_CMD_PAIRING_CONFIRM:
        case BT_SMP_CMD_PAIRING_RANDOM:
        {
            BT_INFO("Val[%d]:%s", 16, bt_hex(&buf->data[1], 16));
            break;
        }
        case BT_SMP_CMD_ENCRYPT_INFO:
        {
            BT_INFO("Ltk[%d]:%s", 16, bt_hex(&buf->data[1], 16));
            break;
        }
        case BT_SMP_CMD_MASTER_IDENT:
        {
            struct bt_smp_master_ident *req = (void *)&buf->data[1];
            BT_INFO("EDIV:0x%04x, Random:0x%08llx", req->ediv, req->rand);
            break;
        }
        case BT_SMP_CMD_PAIRING_FAIL:
        {
            struct bt_smp_pairing_fail *fail = (void *)&buf->data[1];
            const char *p_str = smp_unknow;
   
            switch(fail->reason)
            {
                case BT_SMP_ERR_PASSKEY_ENTRY_FAILED:
                    p_str = err_passkey_fail;
                    break;
                case BT_SMP_ERR_OOB_NOT_AVAIL:
                    p_str = err_oob_fail;
                    break;
                case BT_SMP_ERR_AUTH_REQUIREMENTS:
                    p_str = err_auth_fail;
                    break;
                case BT_SMP_ERR_CONFIRM_FAILED:
                    p_str = err_cfm_fail;
                    break;
                case BT_SMP_ERR_PAIRING_NOTSUPP:
                    p_str = err_pairing_fail;
                    break;
                case BT_SMP_ERR_ENC_KEY_SIZE:
                    p_str = err_enc_fail;
                    break;
                case BT_SMP_ERR_CMD_NOTSUPP:
                    p_str = err_not_fail;
                    break;
                case BT_SMP_ERR_UNSPECIFIED:
                    p_str = err_unspecified_fail;
                    break;
                case BT_SMP_ERR_REPEATED_ATTEMPTS:
                    p_str = err_repeat_fail;
                    break;
                case BT_SMP_ERR_INVALID_PARAMS:
                    p_str = err_invalid_fail;
                    break;
                case BT_SMP_ERR_DHKEY_CHECK_FAILED:
                    p_str = err_dhkey_fail;
                    break;
                case BT_SMP_ERR_NUMERIC_COMP_FAILED:
                    p_str = err_numeric_fail;
                    break;
                case BT_SMP_ERR_BREDR_PAIRING_IN_PROGRESS:
                    p_str = err_br_fail;
                    break;
                case BT_SMP_ERR_CROSS_TRANSP_NOT_ALLOWED:
                    p_str = err_transport_fail;
                    break;
            }
            BT_INFO("Reason:0x%02x(%s)", fail->reason, p_str);
        }
		case BT_SMP_CMD_IDENT_INFO:
        {
            BT_INFO("IRK[%d]:%s", 16, bt_hex(&buf->data[1], 16));
            break;
        }
        case BT_SMP_CMD_IDENT_ADDR_INFO:
        {
            struct bt_smp_ident_addr_info *id_addr = (void *)&buf->data[1];
            BT_INFO("AddrType:%d, Addr=0x%02x%02x%02x%02x%02x%02x", id_addr->addr.type, id_addr->addr.a.val[0], id_addr->addr.a.val[1],
            id_addr->addr.a.val[2], id_addr->addr.a.val[3], id_addr->addr.a.val[4], id_addr->addr.a.val[5]);
            break;
        }
        case BT_SMP_CMD_SIGNING_INFO:
        {
            BT_INFO("SignKey(csrk)[%d]:%s", 16, bt_hex(&buf->data[1], 16));
            break;
        }
        case BT_SMP_CMD_SECURITY_REQUEST:
        {
            uint8_t auth = buf->data[1];
            BT_INFO("Auth:0x%02x(Bonding=%d,MITM=%d,SC=%d,KeyPress=%d,CT2=%d)", 
                        auth, 
                        (auth & BT_SMP_AUTH_BONDING)?1:0, 
                        (auth & BT_SMP_AUTH_MITM)?1:0, 
                        (auth & BT_SMP_AUTH_SC)?1:0,
                        (auth & BT_SMP_AUTH_KEYPRESS)?1:0,
                        (auth & BT_SMP_AUTH_CT2)?1:0);
            break;
        }
        case BT_SMP_CMD_PUBLIC_KEY:
        {
            struct bt_smp_public_key *pub_key = (void *)&buf->data[1];
            BT_INFO("x[%d]:%s", 32, bt_hex(&pub_key->x[0], 32));
            BT_INFO("y[%d]:%s", 32, bt_hex(&pub_key->y[0], 32));
            break;
        }
        case BT_SMP_DHKEY_CHECK:
        {
            struct bt_smp_dhkey_check *pub_key = (void *)&buf->data[1];
            BT_INFO("e[%d]:%s", 16, bt_hex(&pub_key->e[0], 16));
            break;
        }
    }
}
#else
void SMPDbgLog(u8_t type, struct bt_conn *conn, struct net_buf *buf)
{
    
}
#endif //SI_BT_SMP_DBG_ENABLED