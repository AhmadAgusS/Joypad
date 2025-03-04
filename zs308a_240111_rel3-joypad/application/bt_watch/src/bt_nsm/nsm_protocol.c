
#include <app_defines.h>
#include <sys_manager.h>
#include <bt_manager.h>
#include <msg_manager.h>
#include <app_switch.h>
#include <app_config.h>
#include <os_common_api.h>
#include <ctype.h>

#include "nsm_test_backend.h"
#include "nsm_test_inner.h"

//define for bt test mode
//#define BT_TEST_BLE 0x0106A000    //0: BR/EDR TEST, 1: BLE TEST
//#define BT_TEST_BLE_1M 0x0106A004 //0: BLE 1M, 1: BLE 2M
//#define BT_TEST_CHANNEL 0x0106A008 //tx channel    （0-79）
//#define BT_TEST_TXPOWER 0x0106A00C //tx power        （0-26）
//#define BT_TEST_TXMODE 0x0106A010 //tx mode, DH1/DH3/DH5    （0-22）
//#define BT_TEST_PAYLOAD_MODE 0x0106A014 //payload mode   (0-6)
//#define BT_TEST_EXCUTE_MODE 0x0106A018 //single or continue send  （1 or 2）

#define BT_TEST_BLE_REG (*(volatile unsigned *)0x0106A000)
#define BT_TEST_BLE_1M_REG (*(volatile unsigned *)0x0106A004)
#define BT_TEST_CHANNEL_REG (*(volatile unsigned *)0x0106A008)
#define BT_TEST_TXPOWER_REG (*(volatile unsigned *)0x0106A00C)
#define BT_TEST_TXMODE_REG (*(volatile unsigned *)0x0106A010)
#define BT_TEST_PAYLOAD_MODE_REG (*(volatile unsigned *)0x0106A014)
#define BT_TEST_EXCUTE_MODE_REG (*(volatile unsigned *)0x0106A018)
#define BT_MODE_TEST_TIME_SECOND_BASE (*(volatile unsigned *)0x0106A01C)

/**
* @brief Run bt fcc test
*
* This function is used to run bt fcc test
*
* @param mode 0: uart test mode, for pcba
*           1: bt test mode, for demo, only test bt tx power...
*          other value: invalid
*
* @return 0 if succsess.
* @return 1 if failed.
*/
extern int fcc_test_main(uint8_t mode);
void sys_pm_reboot(int type);

enum NSM_COMMANDS
{
    NSM_BR_TX_START  = 0x00C9,
    NSM_BR_RX_START  = 0x00CA,
    NSM_BR_RX_READ  = 0x00CB,
    NSM_MODE_START  = 0x00FD,
};


typedef struct
{
    uint8_t  sync_byte1;   // sync byte1 0xAA
    uint8_t  sync_byte2;   // sync byte2 0xAA
    uint16_t f_len;
    uint16_t cmd;

} __attribute__((packed)) nsm_frame_head_s;

typedef u8_t (*command_callback)(u8_t *buf, u16_t len);
typedef struct{
    u16_t recv_cmd_type;
    command_callback cmd_cbk;
} nsm_recv_cmd_list_s;

#if 0
static void bt_mac_get(uint8_t *mac_str)
{
#ifdef CONFIG_PROPERTY
    int ret;

    ret = property_get(CFG_BT_MAC, mac_str, (MAC_STR_LEN-1));
    if (ret < (MAC_STR_LEN-1)) 
    {
        SYS_LOG_ERR("mac is null.");
    }
#endif
    SYS_LOG_INF("BT MAC: %s", mac_str);
}

void nsm_at_start(void)
{
    const char *resp_start_cmd = "OK";
    
    if ((strlen(resp_start_cmd)+1) != 
            spp_test_backend_write(resp_start_cmd, (strlen(resp_start_cmd)+1), 100))
    {
        SYS_LOG_INF("nsm_at_start RESP ERROR!");
    }
}

void nsm_at_address_get(void)
{
    const char *resp_address_cmd = "OK+Get:0x";

    char resp[32] = "\0";

    strcpy(resp, resp_address_cmd);
    bt_mac_get(resp+strlen(resp_address_cmd));
    resp[31] = "\0";

    if ((strlen(resp_address_cmd)+1) != spp_test_backend_write(resp_address_cmd, (strlen(resp_address_cmd)+1), 100))
    {
        SYS_LOG_INF("nsm_at_address_get RESP ERROR!");
    }
}

void nsm_at_version_get(void)
{
    const char *resp_version_cmd = "OK+Get:";
    CFG_Struct_User_Version   usr_ver;

    char resp[128] = "\0";

    strcpy(resp, resp_version_cmd);

    app_config_read(CFG_ID_USER_VERSION,
        &usr_ver, 0, sizeof(CFG_Struct_User_Version));

    if ((strlen(resp_version_cmd)+ strlen(usr_ver.Version) >= 128))
        sprintf(resp+strlen(resp_version_cmd), "ERROR\n");
    else
        sprintf(resp+strlen(resp_version_cmd), "%s\n", usr_ver.Version);

    resp[127] = "\0";
    if ((strlen(resp_version_cmd)+1) != 
        spp_test_backend_write(resp_version_cmd, (strlen(resp_version_cmd)+1), 100))
    {
        SYS_LOG_INF("nsm_at_version_get RESP ERROR!");
    }
}


void nsm_at_nosignaling_start(void)
{
    /* 先回复后处理
     */
    const char *resp_cmd = "OK+GET";
    
    if (sizeof(resp_cmd) != spp_test_backend_write(resp_cmd, sizeof(resp_cmd), 100))
    {
        SYS_LOG_INF("nsm_at_nosignaling_start RESP ERROR!");
    }
    os_sleep(200);

    // please call function to input nosignaling test.
}

const nsm_recv_cmd_list_s nsm_recv_cmd_list[] = {
    {"AT", (command_callback)nsm_at_start},
    {"AT+ADDR?", (command_callback)nsm_at_address_get},
    {"AT+VERS?", (command_callback)nsm_at_version_get},
    {"AT+NSM", (command_callback)nsm_at_nosignaling_start},
};
#endif

void nsm_br_tx_start(u8_t* payload, int payload_len)
{
#if 0
    if (1 != payload_len)
         SYS_LOG_ERR("payload_len %d!",payload_len);

    u8_t tx_payload[12];
    nsm_frame_head_s fhead;
    u8_t ch = payload[0];
    u16_t tx_len = 0;

    SYS_LOG_INF("ch %d!",ch);
    fhead.sync_byte1 = 0xAA;
    fhead.sync_byte2 = 0xAA;
    fhead.f_len = sizeof(u16_t)+sizeof(u8_t);
    fhead.cmd = NSM_BR_TX_START;
    memcpy(tx_payload, &fhead, sizeof(nsm_frame_head_s));
    tx_len += (sizeof(nsm_frame_head_s)-sizeof(u16_t));
    tx_len += fhead.f_len;
    tx_payload[sizeof(nsm_frame_head_s)] = 0x00;

    if (tx_len != nsm_test_backend_write(tx_payload, tx_len, 100))
    {
        SYS_LOG_ERR("nsm_at_start RESP ERROR!");
    }
    //test func
#endif
}

void nsm_br_rx_start(u8_t* payload, int payload_len)
{

}

void nsm_br_rx_read(u8_t* payload, int payload_len)
{

}
//void acts_reset_peripheral(int reset_id);

void nsm_br_mode_start(u8_t* payload, int payload_len)
{
    /* Payload(6bytes): 
        byte0 : bt_mode 0 or 1 //0: BR/EDR TEST, 1: BLE TEST
        byte1 : BLE_PHY 0 or 1 //0: BLE 1M, 1: BLE 2M
        byte2 : channel //tx channel     (0-79)
        byte3 : tx_power_mode //tx power       (0-43)
        byte4 : tx_mode //tx mode, DH1/DH3/DH5    (9-19, !=12)
        byte5 : payload_mode //payload mode   (0-6)
        byte6 : test time // unit : s
    */
    u8_t tx_payload[12];
    nsm_frame_head_s fhead;
    u16_t tx_len = 0;
    u8_t ret_code = 0;
	int result;

    if (7 != payload_len)
    {
         SYS_LOG_ERR("payload_len %d!",payload_len);
         ret_code = 0x1;
    }
    else
    {
        if (((0 != payload[0]) && (1 != payload[0])) ||
            ((0 != payload[1]) && (1 != payload[1])) ||
            (payload[2] > 79) ||
            (payload[3] > 43) ||
            (payload[5] > 6))
        {
            SYS_LOG_ERR("payload0~3 %x %x %x %x.",payload[0],payload[1],payload[2],payload[3]);
            SYS_LOG_ERR("payload4~5 %x %x.",payload[4],payload[5]);
            ret_code = 0x1;
        }

        if ((payload[4] < 9) || (payload[4] > 19) || (payload[4] == 12))
        {
            SYS_LOG_ERR("payload 0~3 %x %x %x %x.",payload[0],payload[1],payload[2],payload[3]);
            SYS_LOG_ERR("payload 4~5 %x %x.",payload[4],payload[5]);
            ret_code = 0x1;
        }
    }

    fhead.sync_byte1 = 0xAA;
    fhead.sync_byte2 = 0xAA;
    fhead.f_len = sizeof(u16_t)+sizeof(u8_t);
    fhead.cmd = NSM_MODE_START;
    memcpy(tx_payload, &fhead, sizeof(nsm_frame_head_s));
    tx_len += (sizeof(nsm_frame_head_s)-sizeof(u16_t));
    tx_len += fhead.f_len;
    tx_payload[sizeof(nsm_frame_head_s)] = ret_code;

    if (tx_len != nsm_test_backend_write(tx_payload, tx_len, 100))
    {
        SYS_LOG_ERR("nsm_at_start RESP ERROR!");
        return;
    }

    if (ret_code)
    {
        SYS_LOG_ERR("nsm_at_start payload error!");
        return;
    }
	//0xAAAA0900FD0000010F0810040103

    os_sleep(100);
    SYS_LOG_INF("Input BT test mode.");
#if 0
    BT_TEST_BLE_REG = 0x0;
    BT_TEST_BLE_1M_REG = 0x1;
    BT_TEST_CHANNEL_REG = 0xf;
    BT_TEST_TXPOWER_REG = 0x8;
    BT_TEST_TXMODE_REG = 0x10;
    BT_TEST_PAYLOAD_MODE_REG = 0x4;
    BT_TEST_EXCUTE_MODE_REG = 0x1;
#endif
#if 1
    BT_TEST_BLE_REG = payload[0];
    BT_TEST_BLE_1M_REG = payload[1];
    BT_TEST_CHANNEL_REG = payload[2];
    BT_TEST_TXPOWER_REG = payload[3];
    BT_TEST_TXMODE_REG = payload[4];
    BT_TEST_PAYLOAD_MODE_REG = payload[5];
    BT_TEST_EXCUTE_MODE_REG = 0x1;
	BT_MODE_TEST_TIME_SECOND_BASE = payload[6];

	bt_manager_ble_disconnect();
	os_sleep(200);
	//system_power_reboot(0);
	//printf("11111.\n");
	//acts_reset_peripheral(56);

	//sys_write32(sys_read32(RMU_MRCR1) & ~(1 << 24), RMU_MRCR1); /* disable bluetooth hub */
	//sys_write32(0x88000000, 0x40004030); /* m4f on, bt off  else off */

    uint32_t flags;
//#define BT_RAM_CLK_SRC		BIT(16)

    flags = irq_lock();
	k_sched_lock();
	//sys_write32(0x01000011, 0x40000000);
	//sys_write32(0x110e4f31, 0x40000000);
	//sys_write32(0, 0x40000004);
	//sys_write32(0xc10c000c, 0x40000004);
	//sys_write32(0x00000000, 0x40000080);
	//unsigned int val;
	//val = sys_read32(0x400010c4) & ~BT_RAM_CLK_SRC;
	//sys_write32(val, 0x400010c4);

    result = fcc_test_main(1);
	//printf("result %d.\n",result);
	k_sched_unlock();
    irq_unlock(flags);
	sys_pm_reboot(0);
#endif
    //test func

}

const nsm_recv_cmd_list_s nsm_recv_cmd_list[] = {
    {NSM_BR_TX_START, (command_callback)nsm_br_tx_start},
    {NSM_BR_RX_START, (command_callback)nsm_br_rx_start},
    {NSM_BR_RX_READ, (command_callback)nsm_br_rx_read},
    {NSM_MODE_START, (command_callback)nsm_br_mode_start},
};

u16_t nsm_cmd_xml_parse(void)
{
    int i = 0,j;
    u16_t ret_len = 0;
    nsm_frame_head_s fhead;
    u8_t read_buf[128];
    u8_t *ptr = (u8_t *)&fhead;

    ret_len = nsm_test_backend_read((uint8_t *)&fhead, sizeof(nsm_frame_head_s), 5000);

    printf("head:");
    for (j = 0;j < sizeof(nsm_frame_head_s);j++)
    {
        printf(" %x", ptr[j]);
    }
    printf("\n");

    if (sizeof(nsm_frame_head_s) != ret_len)
    {
        SYS_LOG_ERR("ret_len %d!", ret_len);
        return ret_len;
    }

    if ((0xAA != fhead.sync_byte1) || 
        (0xAA != fhead.sync_byte2) ||
        (fhead.f_len > 128) ||
        (fhead.f_len < 2))
    {
        SYS_LOG_ERR("head error %x %x %x!", 
            fhead.sync_byte1,fhead.sync_byte2,fhead.f_len);
        return (ret_len + sizeof(nsm_frame_head_s));
    }

    ret_len = nsm_test_backend_read(read_buf, fhead.f_len-2, 5000);
    if (fhead.f_len != ret_len+2)
    {
        SYS_LOG_ERR("ret_len %x f_len %x!", ret_len,fhead.f_len);
        return (ret_len + sizeof(nsm_frame_head_s));
    }

    for (i = 0 ; i < sizeof(nsm_recv_cmd_list) / sizeof(nsm_recv_cmd_list_s); i++) {
        if (nsm_recv_cmd_list[i].recv_cmd_type  == fhead.cmd) {
            SYS_LOG_INF(" nsm cmd 0x%x start!!", fhead.cmd);
            //print_hex("rxpayload", payload, payload_len);
            nsm_recv_cmd_list[i].cmd_cbk(read_buf, fhead.f_len-2);
            break;
        }
    }

    return (fhead.f_len-2 + sizeof(nsm_frame_head_s));
}
