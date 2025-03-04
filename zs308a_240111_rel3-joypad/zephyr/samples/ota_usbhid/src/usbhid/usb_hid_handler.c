/*
 * Copyright (c) 2018 Actions Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <init.h>
#include <usb/usb_device.h>
#include <drivers/usb/usb_phy.h>
// #include <shell/shell.h>
// #include <shell/shell_uart.h>
#include <sys/byteorder.h>

#include "usb_hid_report_desc.h"
#include "usb_hid_descriptor.h"
#include "usb_hid_handler.h"

#include <os_common_api.h>  // OS_NO_WAIT

struct k_sem hid_write_sem;
struct k_sem hid_read_sem;

system_call_status_flag call_status_cb;
static u8_t usb_hid_buf[64];

#ifdef CONFIG_HID_INTERRUPT_OUT
extern void hid_out_rev_data(void *arg1, void *arg2, void *arg3);   // tr_usb_hid.c
extern char *hid_thread_stack;      // tr_usb_hid.c
extern int hid_thread_exit;         // tr_usb_hid.c
#endif
static u32_t g_recv_report;

void hid_read_complete(void)
{
	SYS_LOG_DBG("Read Self-defined data finish");
    k_sem_give(&hid_read_sem);
}

static void _hid_inter_out_ready_cb(void)
{
#ifdef CONFIG_HID_INTERRUPT_OUT
	hid_read_complete();

#else
	u32_t bytes_to_read;
	u8_t ret;
	u8_t usb_hid_out_buf[64];
	/* Get all bytes were received */
	ret = hid_int_ep_read(usb_hid_out_buf, sizeof(usb_hid_out_buf),
			&bytes_to_read);
	if (!ret && bytes_to_read != 0) {
		for (u8_t i = 0; i < bytes_to_read; i++) {
			SYS_LOG_DBG("usb_hid_out_buf[%d]: 0x%02x", i,
			usb_hid_out_buf[i]);
		}

	} else {
		SYS_LOG_INF("Read Self-defined data fail");
	}
#endif
}

static void _hid_inter_in_ready_cb(void)
{
	SYS_LOG_DBG("Self-defined data has been sent");
	k_sem_give(&hid_write_sem);
}

static int _usb_hid_debug_cb(struct usb_setup_packet *setup, s32_t *len,
	     u8_t **data)
{
	SYS_LOG_DBG("Debug callback");

	return -ENOTSUP;
}

/*
 * Self-defined protocol via set_report
 */
static int _usb_hid_set_report(struct usb_setup_packet *setup, s32_t *len,
				u8_t **data)
{
    
    if (sys_le16_to_cpu(setup->wValue) == 0x0300)
    {
        SYS_LOG_INF("Set Feature\n");
        return 0 ;
    }

	u8_t *temp_data = *data;

	SYS_LOG_DBG("temp_data[0]: 0x%04x\n", temp_data[0]);
	SYS_LOG_DBG("temp_data[1]: 0x%04x\n", temp_data[1]);
	SYS_LOG_DBG("temp_data[2]: 0x%04x\n", temp_data[2]);

	SYS_LOG_DBG("g_recv_report:0x%04x\n", g_recv_report);
      
    g_recv_report = (temp_data[2] << 16) | (temp_data[1] << 8) | temp_data[0];
	
    if (call_status_cb) {
			call_status_cb(g_recv_report);
	}

	return 0;
}

static int _usb_hid_get_report(struct usb_setup_packet *setup, s32_t *len, u8_t **data)
{
    switch (sys_le16_to_cpu(setup->wValue)) 
    {
        case 0x0300:
            SYS_LOG_INF("Get Feature\n");
            usb_hid_buf[0] = 0x41;
            usb_hid_buf[1] = 0x63;
            usb_hid_buf[2] = 0x74;
            usb_hid_buf[3] = 0x69;
            usb_hid_buf[4] = 0x6F;
            usb_hid_buf[5] = 0x6E;
            usb_hid_buf[6] = 0x00;
            usb_hid_buf[7] = 0x46;
            usb_hid_buf[8] = 0x72;
            *data = usb_hid_buf;
			*len = min(*len, 64);
            break;
        
        default:
            break;
    }
    return 0;
}

static const struct hid_ops ops = {
	.get_report = _usb_hid_get_report,
	.get_idle = _usb_hid_debug_cb,
	.get_protocol = _usb_hid_debug_cb,
	.set_report = _usb_hid_set_report,
	.set_idle = _usb_hid_debug_cb,
	.set_protocol = _usb_hid_debug_cb,
	.int_in_ready = _hid_inter_in_ready_cb,
	.int_out_ready = _hid_inter_out_ready_cb,
};

void hid_sem_init(void)
{
	k_sem_init(&hid_read_sem, 0, 1);
    k_sem_init(&hid_write_sem, 1, 1);
}

int usbhid_dev_init(void)
{
    usb_phy_init();
	usb_phy_enter_b_idle();

    hid_sem_init();

#if 0
    hid_thread_exit = 0;        
    hid_thread_stack = mem_malloc(1536);
    if (!hid_thread_stack) {
        SYS_LOG_ERR("hid thread malloc fail");
    } else {
        os_thread_create(hid_thread_stack, 1536, hid_out_rev_data, NULL, NULL, NULL, 5, 0, OS_NO_WAIT);
    }
	
#endif
    /* Register hid report desc to HID core */
	usb_hid_register_device(hid_report_desc, sizeof(hid_report_desc), &ops);

    /* register device descriptors */
	usb_device_register_descriptors(usb_hid_fs_desc, usb_hid_hs_desc);	

    /* USB HID initialize */    
    return usb_hid_init();
}