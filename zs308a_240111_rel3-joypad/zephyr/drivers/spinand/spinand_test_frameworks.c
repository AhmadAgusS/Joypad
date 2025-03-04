
#include <errno.h>
#include <disk/disk_access.h>
#include "spinand_acts.h"

#include <drivers/spinand.h>
#include <board.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(spinand_test_frameworks, CONFIG_FLASH_LOG_LEVEL);

static uint8_t in_buf[512], out_buf[512];
static uint8_t result[16];

#include <drivers/flash.h>
#include <partition/partition.h>
//#define UDISK_OFFSET 0x4a00000
static uint32_t get_udisk_offset(void)
{
	const struct partition_entry *parti;

#ifdef CONFIG_BOARD_NANDBOOT
	parti = partition_get_stf_part(STORAGE_ID_BOOTNAND, PARTITION_FILE_ID_UDISK);
#else
	parti = partition_get_stf_part(STORAGE_ID_NAND, PARTITION_FILE_ID_UDISK);
#endif
	if (parti != NULL) {
	    LOG_INF("udisk offset=0x%x secotr\n", parti->offset);
		return (parti->offset);
	}
    LOG_ERR("get udisk offset err!\n");
    return 0;
}

int spinand_scan_delaychain(const struct device *dev)
{
    uint8_t i;
    uint8_t j = 0;
    uint8_t sum = 0;
    uint8_t ret = 0;
    uint8_t tmp_buf[512];

    struct spinand_info *sni = DEV_DATA(dev);
    uint8_t old_dc = sni->spi->delay_chain;

    uint32_t chipid = 0;
    struct FlashChipInfo *NandFlashInfo;
    u32_t offset = get_udisk_offset();

    memset(result, 0x0, 16);
    memset(tmp_buf, 0x0, 512);
    spinand_read(sni, offset >> 9, tmp_buf, 1);

    for (i = 0; i < 128; i++)
        in_buf[i] = i + 1;

    for (i = 0; i < 16; i++)
    {
        sni->spi->delay_chain = i;

        if (spinand_get_chipid(sni, &chipid) != 0) {
            LOG_ERR("delay_chain:%d; spinand get chipid err!\n", i);
            continue;
        }
        if (get_storage_params(sni, (u8_t *)&chipid, &NandFlashInfo) != 0) {
            LOG_ERR("delay_chain:%d; Can't get flashinfo.\n", i);
            continue;
        }

        LOG_INF("spinand chipid: 0x%x; chipname: %s; delay_chain: %d \n", chipid, NandFlashInfo->FlashMark, i);

	    ret = spinand_write(sni, offset >> 9, in_buf, 1);
        if (ret != 0) {
            LOG_ERR("delaychain=%d fail!\n", i);
            result[i] = 0;
            continue;
        }
        memset(out_buf, 0x0, 512);
        ret = spinand_read(sni, offset >> 9, out_buf, 1);
        ret = memcmp(out_buf, in_buf, 512);
        if (ret != 0) {
            LOG_ERR("delaychain=%d fail!\n", i);
            result[i] = 0;
            continue;
        }
        LOG_INF("delaychain=%d pass!\n", i);
        result[i] = 1;
    }
    LOG_INF("spinand scan delaychain result:\n");
    for (i = 0; i < 16; i++) {
        LOG_INF("[%d]=%d\r\n", i, result[i]);
        if (result[i] == 1) {
            j += 1;
            sum += i;
        }
    }

    if (j != 0) {
        LOG_INF("spinand best delaychain is: %d; old delaychain before test is: %d \n", sum/j, old_dc);
        spinand_write(sni, offset >> 9, tmp_buf, 1);
        sni->spi->delay_chain = old_dc;
    } else {
        LOG_ERR("spinand scan delaychain failed! get all delaychain is 0!\n");
    }

    return 0;
}

