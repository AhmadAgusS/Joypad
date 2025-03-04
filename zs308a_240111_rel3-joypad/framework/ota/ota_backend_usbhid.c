
#include <kernel.h>
#include <fs/fs.h>
#include <stream.h>
#include <string.h>
#include <mem_manager.h>
#include <ota_api.h>
#include <ota_backend.h>
#include <ota_backend_usbhid.h>
#include <usb/class/usb_hid.h>

#define TLV_PACK_U8(buf, type, value)	tlv_pack_data(buf, type, sizeof(uint8_t), value)
#define TLV_PACK_U32(buf, type, value)	tlv_pack_data(buf, type, sizeof(uint32_t), value)

// NOTICE THAT, USBHID does not support complex coupling with
// BT, SPPBLE as 3031 did. Therefore, if the conjuction is more
// specific, this code shall be modified accordingly.

struct ota_backend_usbhid{
    struct ota_backend backend;
    io_stream_t stream;
    int stream_opened;
    char *read_buf, *write_buf;
};

static struct k_sem *sem_read = NULL, *sem_write = NULL;

static uint8_t *tlv_pack(uint8_t *buf, const struct tlv_data *tlv)
{
	uint16_t len;

	len = tlv->len;

	*buf++ = tlv->type;
	*buf++ = len & 0xff;
	*buf++ = len >> 8;

	if (tlv->value_ptr) {
		memcpy(buf, tlv->value_ptr, len);
		buf += len;
	}
	
	return buf;
}

static uint8_t *tlv_pack_data(uint8_t *buf, uint8_t type, uint16_t len, uint32_t number)
{
	struct tlv_data tlv;

	tlv.type = type;
	tlv.len = len;
	tlv.value_ptr = (uint8_t *)&number;

	return tlv_pack(buf, &tlv);
}

static int usbhid_ep_write(const u8_t *data, u32_t data_len, u32_t timeout)
{
	u32_t bytes_ret, len;
 	int ret;

	while (data_len > 0) {
		len = data_len > CONFIG_HID_INTERRUPT_IN_EP_MPS ? CONFIG_HID_INTERRUPT_IN_EP_MPS : data_len;

        u32_t start_time;
        start_time = k_cycle_get_32();
        do {
            if (k_cycle_get_32() - start_time > 50 * 24000)
            {
                ret = -ETIME;
                break;
            }

		    ret = hid_int_ep_write(data, len, &bytes_ret);
        } while (ret == -EAGAIN);
		
		if (ret) {
			usb_hid_in_ep_flush();
			SYS_LOG_ERR("%s, err ret: %d", ret);
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

int ota_usbhid_send_cmd(uint8_t cmd, uint8_t *buf, int size)
{
	struct svc_prot_head *head;
	SYS_LOG_INF("%s, cmd=%d, buf=%p, size=%d", __func__, cmd, buf, size);

	head = (struct svc_prot_head *)buf;
	head->svc_id = SERVICE_ID_OTA;
	head->cmd = cmd;
	head->param_type = TLV_TYPE_MAIN;
	head->param_len = size - sizeof(struct svc_prot_head);
	
	return usbhid_ep_write(buf, size, 5000);
}


int ota_cmd_d2h_report_image_valid(uint8_t *write_buf, int is_valid)
{
	uint8_t *send_buf;
	uint8_t valid_flag;
	int err, send_len;

	valid_flag = is_valid ? 1 : 0;

	send_buf = write_buf + sizeof(struct svc_prot_head);
	send_buf = TLV_PACK_U8(send_buf, 0x1, valid_flag);
	send_len = send_buf - write_buf;

	err = ota_usbhid_send_cmd(OTA_CMD_D2H_VALIDATE_IMAGE, write_buf, send_len);
	if (err) {
		SYS_LOG_ERR("failed to send cmd %d, error %d", OTA_CMD_D2H_REQUIRE_IMAGE_DATA, err);
	}

	return err;
}

int ota_backend_usbhid_ioctl(struct ota_backend *backend, int cmd, unsigned int param)
{
	struct ota_backend_usbhid *backend_usbhid = CONTAINER_OF(backend, 
            struct ota_backend_usbhid, backend);
	
	int err;

	SYS_LOG_INF("cmd 0x%x: param %d\n", cmd, param);

	switch (cmd) {
	case OTA_BACKEND_IOCTL_REPORT_IMAGE_VALID:
		err = ota_cmd_d2h_report_image_valid(backend_usbhid->write_buf, param);
		if (err) {
			SYS_LOG_INF("send cmd 0x%x error", cmd);
			return -EIO;
		}
		break;
	case OTA_BACKEND_IOCTL_REPORT_PROCESS:
		backend->cb(backend, OTA_BACKEND_UPGRADE_PROGRESS, param);
		break;
	case OTA_BACKEND_IOCTL_GET_UNIT_SIZE:
		*(unsigned int*)param = CONFIG_HID_INTERRUPT_OUT_EP_MPS;
		break;
	default:
		SYS_LOG_ERR("unknow cmd 0x%x", cmd);
		return -EINVAL;
	}

	return 0;
}

static int usbhid_ep_read(u8_t *data, u32_t data_len, u32_t timeout_ms)
{
	u32_t bytes_ret = 0;
    u32_t len = 0;
	u8_t *buf;
	int ret;
    
	buf = data;

    if (sem_read != NULL && k_sem_take(sem_read, K_MSEC(timeout_ms))) {
		usb_hid_out_ep_flush();
		SYS_LOG_ERR("%s, timeout", __func__);
		return -ETIME;
	}

    len = data_len > CONFIG_HID_INTERRUPT_OUT_EP_MPS ? CONFIG_HID_INTERRUPT_OUT_EP_MPS : data_len;

	ret = hid_int_ep_read(buf, len, &bytes_ret);
	if (ret || (len != bytes_ret)) {
		SYS_LOG_ERR("%s, Wrong data length(expect len: %d / read: %d) or ret=%d", __func__, len, bytes_ret, ret);
		return -EIO;
	}

    return bytes_ret; 
}

static int ota_cmd_require_image_data(uint8_t *write_buf, uint32_t offset, int len)
{
	uint8_t *send_buf;
	int err, send_len;

	SYS_LOG_INF("%s, offset 0x%x, len %d, buf %p", __func__, offset, len);

	send_buf = write_buf + sizeof(struct svc_prot_head);
	send_buf = TLV_PACK_U32(send_buf, 0x1, offset);
	send_buf = TLV_PACK_U32(send_buf, 0x2, len);
	send_len = send_buf - write_buf;

	err = ota_usbhid_send_cmd(OTA_CMD_D2H_REQUIRE_IMAGE_DATA, write_buf, send_len);
	if (err) {
		SYS_LOG_ERR("failed to send cmd %d, error %d", OTA_CMD_D2H_REQUIRE_IMAGE_DATA, err);
		return err;
	}

	return 0;
}

int ota_backend_usbhid_read(struct ota_backend *backend, int offset, unsigned char *buf, int size)
{
    SYS_LOG_INF("%s, offset=%x, buf=%p, size=%d", __func__, offset, buf, size);
    int res = 0;
    u8_t fail_cnt = 0;
    int read_from_usbhid_cnt = 0;
	int err = 0;

	struct ota_backend_usbhid *backend_usbhid = CONTAINER_OF(backend, 
            struct ota_backend_usbhid, backend);

	err = ota_cmd_require_image_data(backend_usbhid->write_buf, offset, size);
	if (err) {
		SYS_LOG_ERR("read data err %d", err);
		return -EIO;
	}

    while (size > 0) {
        res = usbhid_ep_read(backend_usbhid->read_buf, CONFIG_HID_INTERRUPT_OUT_EP_MPS, 5000);
		if (res < 0) {
            fail_cnt++;
            SYS_LOG_ERR("%s, usbhid_ep_read update fail, res:%d", res);
            if (fail_cnt == 3) {
                return -1;
            }

        } else {
            SYS_LOG_DBG("%s, #%d read usb-hid: buf=[%p], size=%d, res=%d", __func__, read_from_usbhid_cnt++, buf, size, res);
            int write_size = (size > (res - 1)) ? (res - 1) : size;
            memcpy(buf, backend_usbhid->read_buf + 1, write_size);
            memset(backend_usbhid->read_buf, 0, sizeof(CONFIG_HID_INTERRUPT_OUT_EP_MPS));
            size -= res - 1;
            buf += res - 1;
            fail_cnt = 0;
        }
    }

    return 0;
}

int ota_backend_usbhid_open(struct ota_backend *backend)
{
    return 0;
}

int ota_backend_usbhid_close(struct ota_backend *backend)
{
    if (sem_read) {
		k_sem_give(sem_read);
	}
	
	if (sem_write) {
		k_sem_give(sem_write);
	}
    
    return 0;
}

void ota_backend_usbhid_exit(struct ota_backend *backend)
{
    struct ota_backend_usbhid *backend_usbhid = CONTAINER_OF(backend, 
            struct ota_backend_usbhid, backend);

    if (backend_usbhid->write_buf) {
		mem_free(backend_usbhid->write_buf);
	}
	if (backend_usbhid->read_buf) {
        mem_free(backend_usbhid->read_buf);
	}
    if (backend_usbhid) {
		mem_free(backend_usbhid);
	}   
    return;
}

struct ota_backend_api ota_backend_api_usbhid = {
    .init = (void *)ota_backend_usbhid_init,
    .exit = ota_backend_usbhid_exit,
    .open = ota_backend_usbhid_open,
    .close = ota_backend_usbhid_close,
    .read = ota_backend_usbhid_read,
	.ioctl = ota_backend_usbhid_ioctl,
};

struct ota_backend *ota_backend_usbhid_init(ota_backend_notify_cb_t cb,
        struct ota_backend_usbhid_init_param *param)
{
    struct ota_backend_usbhid *backend_usbhid;

    backend_usbhid = mem_malloc(sizeof(struct ota_backend_usbhid));
    if (!backend_usbhid)
    {
        SYS_LOG_ERR("%s, malloc failed", __func__);
        return NULL;
    }
    memset(backend_usbhid, 0 , sizeof(struct ota_backend_usbhid));

    backend_usbhid->write_buf = mem_malloc(CONFIG_HID_INTERRUPT_IN_EP_MPS);
    if (!backend_usbhid->write_buf) {
        SYS_LOG_ERR("%s, write_buf malloc fail", __func__);
        return NULL;
    }

	backend_usbhid->read_buf = mem_malloc(CONFIG_HID_INTERRUPT_OUT_EP_MPS);
    if (!backend_usbhid->read_buf) {
        SYS_LOG_ERR("%s, read_buf malloc fail", __func__);
        return NULL;
    }

    if (param->sem_read) {
        sem_read = param->sem_read;
    }

    if (param->sem_write) {
        sem_write = param->sem_write;
    }
	
	ota_backend_init(&backend_usbhid->backend, OTA_BACKEND_TYPE_USBHID, &ota_backend_api_usbhid, cb);

    return &backend_usbhid->backend;
}
