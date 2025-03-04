/***************************************************************************
 * Copyright (c) 2018 Actions Semi Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 * Description:    usb hid hal
 *
 * Change log:
 ****************************************************************************
 */
#include <usb/class/usb_hid.h>
#include <stdio.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <os_common_api.h>
#include <thread_timer.h>
#include <srv_manager.h> // BACKGOURND_APP

#include "usb_hid_handler.h"
#include "app_defines.h"    // APP_ID_OTA

#define CONFIG_USBHID_STACKSIZE 1536
char __aligned(ARCH_STACK_PTR_ALIGN) usbhid_stack_area[CONFIG_USBHID_STACKSIZE];
#define HID_TIME_OUT                50   //hid 写超时时间

extern struct k_sem hid_read_sem;

int usb_hid_ep_write(const u8_t *data, u32_t data_len, u32_t timeout)
{
	u32_t bytes_ret, len;
 	int ret;

	while (data_len > 0) {

		len = data_len > CONFIG_HID_INTERRUPT_IN_EP_MPS ? CONFIG_HID_INTERRUPT_IN_EP_MPS : data_len;

        u32_t start_time;
        start_time = k_cycle_get_32();
        do {
            if (k_cycle_get_32() - start_time > HID_TIME_OUT * 24000)
            {
                ret = -ETIME;
                break;
            }

		    ret = hid_int_ep_write(data, len, &bytes_ret);
        } while (ret == -EAGAIN);
		
		if (ret) {
			usb_hid_in_ep_flush();
			SYS_LOG_ERR("err ret: %d", ret);
			return ret;
		}

		if (len != bytes_ret) {
			SYS_LOG_ERR("err len: %d, wrote: %d", len, bytes_ret);
			return -EIO;
		}

		data_len -= len;
		data += len;
        if(data_len > 0)
            k_sleep(K_MSEC(100));

	}

	return 0;
}

#ifdef CONFIG_HID_INTERRUPT_OUT
int usb_hid_ep_read(u8_t *data, u32_t data_len, u32_t timeout)
{
	u32_t bytes_ret = 0;
    u32_t len = 0;
	u8_t *buf;
	int ret;
    
	buf = data;

    if (k_sem_take(&hid_read_sem, K_MSEC(timeout))) {
		usb_hid_out_ep_flush();
		SYS_LOG_ERR("%s, timeout", __func__);
		return -ETIME;
	}

    len = data_len > CONFIG_HID_INTERRUPT_OUT_EP_MPS ? CONFIG_HID_INTERRUPT_OUT_EP_MPS : data_len;

	ret = hid_int_ep_read(buf, len, &bytes_ret);
	if (ret) {
		SYS_LOG_ERR("err ret: %d\n", ret);
		return -EIO;
	}

	if (len != bytes_ret) {
		SYS_LOG_ERR("err len: %d, read: %d\n", len, bytes_ret);
		return -EIO;
	}
	
    return bytes_ret;
}
#endif

void trigger_ota_process(void)
{
    struct app_msg msg = {
		.type = MSG_INPUT_EVENT,
		.cmd = MSG_USBHID_STARTOTA,
	};

	send_async_msg(APP_ID_OTA, &msg);
}

void usbhid_service_stop(void)
{
    struct app_msg msg = {
		.type = MSG_EXIT_APP
	};

	send_async_msg(APP_ID_USBHID_SERVICE, &msg);
}

void hid_command_process_func(u8_t * buf, int flag, u8_t len)
{
    //SYS_LOG_INF("buf[0]:%x,buf[1]%x,buf[2]:%x, %x, %x, %x, %x\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6]);
    switch(buf[0]) {
        case 0x55: {
            switch (buf[1]) {
                case 0x55:
                    if (buf[2] == 0 || buf[2] == 0xff) { // Trigger OTA(USB-HID)               
#ifdef CONFIG_OTA_BACKEND_USBHID
                        trigger_ota_process();
						usbhid_service_stop();
#else
                        SYS_LOG_ERR("%s, L%d, enter update fail(not yet enable CONFIG_OTA_BACKEND_USBHID)", __func__, __LINE__);
#endif
                    }
                    break;
                default:
                    break;
            }
            break;
        }

        default:
            SYS_LOG_WRN("%s, cmd: 0x%02x 0x%02x is not yet defined", __func__, buf[0], buf[1]);
            break;
	}
}

int usbhid_service_start(void)
{
    /* active usbhid service */
	if (!srv_manager_check_service_is_actived(APP_ID_USBHID_SERVICE)) {
		return (srv_manager_active_service(APP_ID_USBHID_SERVICE) == true) ? 0 : -1;
	}
	return 0;
}

static void usbhid_app_main(void *p1, void *p2, void *p3)
{
	struct app_msg msg = { 0 };
	unsigned char buf[64] = { 0 };
	
	int ret = 0;
	bool terminaltion = false;

	SYS_LOG_INF("---usbhid service enter---");

	while (!terminaltion) {
		// read usbhid out
		ret = usb_hid_ep_read(buf, 64, -1);
        if (ret > 0) {
            hid_command_process_func(buf, 0, 64);
    	}     

		// receive msgbox from other app
		if (receive_msg(&msg, thread_timer_next_timeout())) {
			int result = 0;

			switch (msg.type) {
			case MSG_START_APP:
				usbhid_service_start();
                SYS_LOG_INF("START_APP");
				break;
			case MSG_EXIT_APP:
                SYS_LOG_INF("%s exit", __func__);
				terminaltion = true;
				break;
			case MSG_SUSPEND_APP:
				SYS_LOG_INF("SUSPEND_APP");
				break;
			case MSG_RESUME_APP:
				SYS_LOG_INF("RESUME_APP");
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

		k_sleep(K_MSEC(50));
	}
	SYS_LOG_INF("---usbhid service exit---");
}

SERVICE_DEFINE(usbhid, usbhid_stack_area, CONFIG_USBHID_STACKSIZE,
			   13, BACKGROUND_APP,
			   NULL, NULL, NULL, usbhid_app_main);