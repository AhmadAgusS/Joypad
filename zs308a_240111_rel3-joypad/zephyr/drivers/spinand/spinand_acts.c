/*
 * Copyright (c) 2018 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

//#define DT_DRV_COMPAT actions_acts_flash

#include <errno.h>
#include <disk/disk_access.h>
#include "spinand_acts.h"

#include <drivers/spinand.h>
#include <board.h>
#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
#include <dvfs.h>
#endif

#define ID_TBL_MAGIC			0x53648673
#define ID_TBL_ADDR				soc_boot_get_nandid_tbl_addr()

#ifndef CONFIG_SPINAND_LIB
//spinand code rom api address
#define SPINAND_API_ADDR	    0x00006800
#define p_spinand_api	((struct spinand_operation_api *)SPINAND_API_ADDR)
#endif

#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_DISK_OK		0x08	/* Medium OK in the drive */

#define LIB_LOG_LEVEL DEBUG_LEVEL
#define HEAP_SIZE (14*1024)
uint32_t api_bss[HEAP_SIZE/4];

struct k_mutex mutex;

#define SECTOR_SIZE 512

#include <logging/log.h>
LOG_MODULE_REGISTER(spinand_acts, CONFIG_FLASH_LOG_LEVEL);

#define LOG printk
//#define LOG LOG_INF

#ifndef CONFIG_SPINAND_DATA
#define CONFIG_SPINAND_DATA 1
#endif

static bool spinand_initial;
static bool spinand_resume;
static bool spinand_poweroff;
static u32_t start_cycle;
#ifndef CONFIG_SPINAND_POWER_CONTROL_SECONDS
#define CONFIG_SPINAND_POWER_CONTROL_SECONDS 5
#endif
#ifdef CONFIG_PM_DEVICE
K_THREAD_STACK_DEFINE(spinand_workq_stack, 1024);
struct k_work_q spinand_workq;
struct k_delayed_work power_control;
int spinand_resume_init(struct spinand_info *sni);
void spinand_workqueue_poweroff(struct k_work *work);
#endif
void set_spinand_spimfp();

static uint8_t spinand_dvfs_clk;
static uint8_t spinand_power_level;
static uint32_t spinand_max_sectors;

void *spinand_panic(const char *fmt, void *sni)
{
    spinand_dump_zoneinfo((struct spinand_info *)sni);
    LOG("%s\n", fmt);
    k_panic();
    return NULL;
}

static struct nand_info system_spinand = {
    .base = SPI0_REG_BASE+(0x4000*CONFIG_SPINAND_USE_SPICONTROLER),
    .bus_width = CONFIG_SPINAND_FLASH_BUS_WIDTH,
    .delay_chain = 0,
    .data = CONFIG_SPINAND_DATA,
    .dma_base = DMA_REG_BASE+0x100+(CONFIG_DMA_SPINAND_RESEVER_CHAN*0x100), //DMA9
    .printf = (void *)printk,
    .loglevel = LIB_LOG_LEVEL,
    .panic = (void *)spinand_panic,
};

static struct spinand_info spinand_acts_data = {
    .protect = 1,
};

#ifndef CONFIG_SPINAND_LIB
#define SPI_CTL                                     (0x00)
#define SPI_STAT                                    (0x04)
#define SPI_TXDAT                                   (0x08)
#define SPI_RXDAT                                   (0x0C)
#define SPI_TCNT                                    (0x10)

#define SPI_DELAY(s)						(s->spi->delay_chain)

enum
{
    DISABLE,
    SPIMODE_READ_ONLY,
    SPIMODE_WRITE_ONLY,
    SPIMODE_RW,
};

enum
{
    TWO_X_FOUR_X,
    DUAL_QUAD,
};

enum
{
    ONE_X_IO_0,
    ONE_X_IO_1,
    TWO_X_IO,
    FOUR_X_IO,
};

static inline uint32_t reg_read(struct spinand_info *si, uint32_t reg)
{
    return sys_read32(si->spi->base + reg);
}

static inline void reg_write(struct spinand_info *si, uint32_t value, uint32_t reg)
{
    sys_write32(value, si->spi->base + reg);
}

static inline int spi_controller_num(struct spinand_info *si)
{
    if (si->spi->base == (SPI0_REG_BASE)) {
        return 0;
    } else if (si->spi->base == (SPI1_REG_BASE)) {
        return 1;
    } else if (si->spi->base == (SPI2_REG_BASE)) {
        return 2;
    } else {
        return 3;
    }
}

void SPI_SET_TX_CPU(struct spinand_info *sni)
{
    if (spi_controller_num(sni) == 0) {
        /* SPI0 */
        reg_write(sni, (reg_read(sni, SPI_CTL) | (SPI_DELAY(sni) << 16) | (1 << 8)), SPI_CTL);
    } else if (spi_controller_num(sni) == 1) {
        /* SPI1 */
        reg_write(sni, (reg_read(sni, SPI_CTL) | (SPI_DELAY(sni) << 16) | (1 << 15)), SPI_CTL);
    } else if (spi_controller_num(sni) == 2) {
        /* SPI2 */
        reg_write(sni, ((reg_read(sni, SPI_CTL) & ~(1U << 31 | 1 << 30 | 3 << 0)) | (1 << 15) | (1 << 5) | (0x7 << 16)), SPI_CTL);
    } else {
        /* SPI3 */
        reg_write(sni, (reg_read(sni, SPI_CTL) | (SPI_DELAY(sni) << 16)), SPI_CTL);
    }
}

void SPI_RESET_FIFO(struct spinand_info *sni)
{
    if (spi_controller_num(sni) == 0) {
        /* SPI0 */
        //Only bit7, 1 can be written.
        reg_write(sni, 0x82, SPI_STAT);
    } else {
        /* SPI3 */
        //Only bit2,3,8,9,11 can be written.
        reg_write(sni, 0xb0c, SPI_STAT);
    }
}

void spinand_basic_set_to_send_mode(struct spinand_info *sni)
{
    SPI_SET_TX_CPU(sni);
    SPI_RESET_FIFO(sni);
}

void SPI_SET_DUAL_MODE(struct spinand_info *sni, uint8_t mode)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) & ~(1 << 27)) | ((mode) << 27), SPI_CTL);
}

void SPI_SET_IO_MODE(struct spinand_info *sni, uint8_t mode)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) & ~(3 << 10)) | ((mode) << 10), SPI_CTL);
}

void SPI_WRITE_START(struct spinand_info *sni)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) | (1 << 5)), SPI_CTL);
}

void SPI_SET_RW_MODE(struct spinand_info *sni, uint8_t mode)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) & ~(3 << 0)) | ((mode) << 0), SPI_CTL);
}

void WAIT_SPI_TX_NOT_FULL(struct spinand_info *sni)
{
    while(reg_read(sni, SPI_STAT) & (1 << 5));
}

void WRITE_TX_FIFO(struct spinand_info *sni, uint32_t val)
{
    reg_write(sni, val, SPI_TXDAT);
}

void WAIT_SPI_TX_EMPTY(struct spinand_info *sni)
{
    while(!(reg_read(sni, SPI_STAT) & (1 << 4)));
}

void WAIT_SPI_BUS_IDLE(struct spinand_info *sni)
{
    if (spi_controller_num(sni) == 0) {
        /* SPI0 */
        while(reg_read(sni, SPI_STAT) & (1 << 6));
    } else {
        /* SPI1/2/3 */
        while(reg_read(sni, SPI_STAT) & (1 << 0));
    }
}

void SPI_REVERSE(struct spinand_info *sni)
{
    if (spi_controller_num(sni) == 0) {
        /* SPI0 */
        reg_write(sni, reg_read(sni, SPI_CTL) & ~(3 << 0 | 1U << 31 | 3 << 4 | 3 << 6 | 15 << 16), SPI_CTL);
    } else {
        /* SPI1/2/3 */
        reg_write(sni, reg_read(sni, SPI_CTL) & ~(3 << 0 | 1U << 31 | 3 << 4 | 3 << 6 | 15 << 16), SPI_CTL);
    }
}

void spinand_basic_send_data_with_byte(struct spinand_info *sni, uint8_t *send_buf, uint32_t len)
{
    spinand_basic_set_to_send_mode(sni);

    SPI_SET_DUAL_MODE(sni, TWO_X_FOUR_X);
    SPI_SET_IO_MODE(sni, ONE_X_IO_0);

    SPI_WRITE_START(sni);

    SPI_SET_RW_MODE(sni, SPIMODE_WRITE_ONLY);
    while (len-- > 0)
    {
        WAIT_SPI_TX_NOT_FULL(sni);
        WRITE_TX_FIFO(sni, *send_buf++);
    }
    WAIT_SPI_TX_EMPTY(sni);

    WAIT_SPI_BUS_IDLE(sni);
    SPI_REVERSE(sni);
}

void SPI_SET_RX_CPU(struct spinand_info *sni)
{
    if (spi_controller_num(sni) == 0) {
        /* SPI0 */
        reg_write(sni, (reg_read(sni, SPI_CTL) | (SPI_DELAY(sni) << 16) | (1 << 8)), SPI_CTL);
    } else if (spi_controller_num(sni) == 1) {
        /* SPI1 */
        reg_write(sni, (reg_read(sni, SPI_CTL) | (SPI_DELAY(sni) << 16) | (1 << 15)), SPI_CTL);
    } else if (spi_controller_num(sni) == 2) {
        /* SPI2 */
        reg_write(sni, ((reg_read(sni, SPI_CTL) & ~(1U << 31 | 1 << 30 | 3 << 0)) | (0x7 << 16) | (1 << 15)), SPI_CTL);
    } else {
        /* SPI3 */
        reg_write(sni, (reg_read(sni, SPI_CTL) | (SPI_DELAY(sni) << 16)), SPI_CTL);
    }
}

void spinand_basic_set_to_receive_mode(struct spinand_info *sni)
{
    SPI_SET_RX_CPU(sni);
    SPI_RESET_FIFO(sni);
}

void SPI_SET_READ_COUNT(struct spinand_info *sni, uint32_t val)
{
    reg_write(sni, val, SPI_TCNT);
}

void SPI_READ_START(struct spinand_info *sni)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) | (1 << 4)), SPI_CTL);
}

void WAIT_SPI_RX_NOT_EMPTY(struct spinand_info *sni)
{
    if (spi_controller_num(sni) == 0) {
        /* SPI0 */
        while(reg_read(sni, SPI_STAT) & (1 << 2));
    } else {
        /* SPI1/2/3 */
        while(reg_read(sni, SPI_STAT) & (1 << 6));
    }
}

uint32_t READ_RX_FIFO(struct spinand_info *sni)
{
    return reg_read(sni, SPI_RXDAT);
}

void spinand_basic_receive_data_with_byte(struct spinand_info *sni, uint8_t *receive_buf, uint32_t len)
{
    spinand_basic_set_to_receive_mode(sni);
    //1xIO
    SPI_SET_DUAL_MODE(sni, TWO_X_FOUR_X);
    SPI_SET_IO_MODE(sni, ONE_X_IO_0);

    SPI_SET_READ_COUNT(sni, len);
    SPI_READ_START(sni);

    //Must be start read at last, otherwise nothing can be read!
    SPI_SET_RW_MODE(sni, SPIMODE_READ_ONLY);
    while (len-- > 0)
    {
        WAIT_SPI_RX_NOT_EMPTY(sni);
        *receive_buf++ = READ_RX_FIFO(sni);
    }

    SPI_REVERSE(sni);
}

void SPI_CS_LOW(struct spinand_info *sni)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) & (~(1 << 3))), SPI_CTL);
}

void SPI_CS_HIGH(struct spinand_info *sni)
{
    reg_write(sni, (reg_read(sni, SPI_CTL) | (1 << 3)), SPI_CTL);
}

void spinand_basic_select_spinand(struct spinand_info *sni, uint8_t select)
{
    if (select)
        SPI_CS_LOW(sni);
    else
        SPI_CS_HIGH(sni);
}

#define SPINAND_FEATURE_SECURE_ADDRESS				0xB0
#define SPINAND_GET_FEATURE_COMMAND					0x0F
#define SPINAND_SET_FEATURE_COMMAND					0x1F

/*feature value should be A0/B0/C0/D0/F0H*/
uint8_t spinand_get_feature(struct spinand_info *sni, uint8_t feature)
{
    uint8_t status;
    uint8_t cmd_addr[2] = {SPINAND_GET_FEATURE_COMMAND, feature};

    spinand_basic_select_spinand(sni, 1);
    spinand_basic_send_data_with_byte(sni, cmd_addr, sizeof(cmd_addr));
    spinand_basic_receive_data_with_byte(sni, &status, 1);
    spinand_basic_select_spinand(sni, 0);

    return status;
}

/*feature value should be A0/B0/C0/D0/F0H*/
void spinand_set_feature(struct spinand_info *sni, uint8_t feature, uint8_t setting)
{
    uint8_t cmd_addr[3] = {SPINAND_SET_FEATURE_COMMAND, feature, setting};

    spinand_basic_select_spinand(sni, 1);
    spinand_basic_send_data_with_byte(sni, cmd_addr, sizeof(cmd_addr));
    spinand_basic_select_spinand(sni, 0);
}
#endif

static int spinand_acts_read(const struct device *dev, off_t offset, void *data, size_t len)
{
    k_mutex_lock(&mutex, K_FOREVER);
    //offset, len shuold align with 512B.
    struct spinand_info *sni = &spinand_acts_data;
    int ret = 0;

#ifdef CONFIG_PM_DEVICE
    start_cycle = k_cycle_get_32();
    if (spinand_resume) {
        ret = spinand_resume_init(sni);
        if (ret != 0) {
            LOG("spinand resume err, spinand read err!\n");
            k_mutex_unlock(&mutex);
            return ret;
        }
    }
#endif

    if (!spinand_initial) {
        LOG("%s spinand init failed, please check...\n", __func__);
        k_mutex_unlock(&mutex);
        return -1;
    }

    //printk("read offset = %d; len = %d;\n", offset, len);
    if ((offset + len) > spinand_max_sectors) {
        LOG("%s error! read from 0x%x len 0x%x excceed storage capacity 0x%x.\n", __func__, (s32_t)offset, len, spinand_max_sectors);
        k_mutex_unlock(&mutex);
        return -1;
    }

#ifndef CONFIG_SPINAND_LIB
    ret =  p_spinand_api->read(sni, offset, data, len);
#else
    ret = spinand_read(sni, offset, data, len);
#endif

    k_mutex_unlock(&mutex);

    return ret;
}

static int spinand_acts_write(const struct device *dev, off_t offset, const void *data, size_t len)
{
    k_mutex_lock(&mutex, K_FOREVER);
    //offset, len shuold align with 512B.
    struct spinand_info *sni = &spinand_acts_data;
    int ret = 0;

#ifdef CONFIG_PM_DEVICE
    start_cycle = k_cycle_get_32();
    if (spinand_resume) {
        ret = spinand_resume_init(sni);
        if (ret != 0) {
            LOG("spinand resume err, spinand write err!\n");
            k_mutex_unlock(&mutex);
            return ret;
        }
    }
#endif

    if (!spinand_initial) {
        LOG("%s spinand init failed, please check...\n", __func__);
        k_mutex_unlock(&mutex);
        return -1;
    }

    //printk("write offset = %d; len = %d;\n", offset, len);
    if ((offset + len) > spinand_max_sectors) {
        LOG("%s error! write from 0x%x len 0x%x excceed storage capacity 0x%x.\n", __func__, (s32_t)offset, len, spinand_max_sectors);
        k_mutex_unlock(&mutex);
        return -1;
    }

#ifndef CONFIG_SPINAND_LIB
    ret = p_spinand_api->write(sni, offset, data, len);
#else
    ret = spinand_write(sni, offset, data, len);
#endif

    k_mutex_unlock(&mutex);

    return ret;
}

static int spinand_acts_erase(const struct device *dev, off_t offset, size_t size)
{
    k_mutex_lock(&mutex, K_FOREVER);
    //offset, len shuold align with 512B.
    struct spinand_info *sni = &spinand_acts_data;
    int ret = 0;

#ifdef CONFIG_PM_DEVICE
    start_cycle = k_cycle_get_32();
    if (spinand_resume) {
        ret = spinand_resume_init(sni);
        if (ret != 0) {
            LOG("spinand resume err, spinand erase err!\n");
            k_mutex_unlock(&mutex);
            return ret;
        }
    }
#endif

    if (!spinand_initial) {
        LOG("%s spinand init failed, please check...\n", __func__);
        k_mutex_unlock(&mutex);
        return -1;
    }

    printk("erase offset = %d; len = %d;\n", (s32_t)offset, size);

#ifndef CONFIG_SPINAND_LIB
    ret = p_spinand_api->erase(sni, offset, size);
#else
    ret = spinand_erase(sni, offset, size);
#endif

    k_mutex_unlock(&mutex);

    return ret;
}

#ifdef CONFIG_SPINAND_LIB
static int spinand_acts_flush(const struct device *dev, bool efficient)
{
    k_mutex_lock(&mutex, K_FOREVER);
    //offset, len shuold align with 512B.
    struct spinand_info *sni = &spinand_acts_data;
    int ret = 0;

#ifdef CONFIG_PM_DEVICE
    start_cycle = k_cycle_get_32();
    if (spinand_resume) {
        ret = spinand_resume_init(sni);
        if (ret != 0) {
            LOG("spinand resume err, spinand flush err!\n");
            k_mutex_unlock(&mutex);
            return ret;
        }
    }
#endif

    if (!spinand_initial) {
        LOG("%s spinand init failed, please check...\n", __func__);
        k_mutex_unlock(&mutex);
        return -1;
    }

    ret = spinand_flush(sni, efficient);

    k_mutex_unlock(&mutex);

    return ret;
}
#endif

int get_storage_params(struct spinand_info *sni, u8_t *id, struct FlashChipInfo **ChipInfo)
{
    int i, j;

    struct NandIdTblHeader *id_tbl_header = (struct NandIdTblHeader *)sni->id_tbl;
    struct FlashChipInfo *id_tbl = (struct FlashChipInfo *)((uint8_t *)sni->id_tbl + sizeof(struct NandIdTblHeader));

    if (id_tbl_header->magic != ID_TBL_MAGIC)
    {
        LOG("id tbl magic err! 0x%x \n", id_tbl_header->magic);
        return -1;
    }

    for (i = 0; i < id_tbl_header->num; i++)
    {
        for (j = 0; j < NAND_CHIPID_LENGTH / 2; j++)
        {
            /* skip compare the 0xff value */
            if ((id_tbl[i].ChipID[j] != 0xff) && (id_tbl[i].ChipID[j] != id[j]))
            {
                //LOG("id not match; id_tbl[%d].ChipID[%d] = 0x%x; id[%d] = 0x%x\n", i, j, id_tbl[i].ChipID[j], j, id[j]);
                break;
            }
            //LOG("id match; id_tbl[%d].ChipID[%d] = 0x%x; id[%d] = 0x%x\n", i, j, id_tbl[i].ChipID[j], j, id[j]);
        }

        if (j == NAND_CHIPID_LENGTH / 2)
        {
            *ChipInfo = &id_tbl[i];
            //LOG("get chipinfo.\n");
            return 0;
        }
    }

    LOG("spinand id not match, please check!\n");
    return -1;
}

int spinand_get_chipid(struct spinand_info *sni, u32_t *chipid)
{
    int i = 0;
    struct FlashChipInfo *NandFlashInfo;

retry:
#ifndef CONFIG_SPINAND_LIB
    *chipid = p_spinand_api->read_chipid(sni);
#else
    *chipid = spinand_read_chipid(sni);
#endif
    //LOG("nand id = 0x%x\n", *chipid);
    if (*chipid == 0x0 || *chipid == 0xffffffff) {
        if (i++ < 3) {
            LOG("Can't get spinand id, retry %d...\n", i);
            goto retry;
        } else {
            LOG("Can't get spinand id, Please check!\n");
            return -1;
        }
    }

    if (get_storage_params(sni, (u8_t *)chipid, &NandFlashInfo) != 0) {
        LOG("Get chipid = 0x%x; But Can't get this chipid in idtbl.\n", *chipid);
        return -1;
    }

    return 0;
}

int spinand_get_delaychain(struct spinand_info *sni)
{
    uint32_t chipid = 0;
    struct FlashChipInfo *NandFlashInfo;

    /* setup SPI clock rate, 16MHz */
    clk_set_rate(CLOCK_ID_SPI0+CONFIG_SPINAND_USE_SPICONTROLER, MHZ(16));

    if (spinand_get_chipid(sni, &chipid) != 0) {
        LOG("spinand get chipid err!\n");
        return -EINVAL;
    }
    if (get_storage_params(sni, (u8_t *)&chipid, &NandFlashInfo) != 0) {
        LOG("Can't get flashinfo.\n");
        return -1;
    }

    LOG("spinand chipid: 0x%x; chipname: %s \n", chipid, NandFlashInfo->FlashMark);

    return NandFlashInfo->delayChain;
}

uint32_t spinand_get_storage_capacity(struct spinand_info *sni)
{
    uint32_t chipid = 0;
    uint32_t max_sectors = 0;
    uint32_t zone_num = 0;
    struct FlashChipInfo *NandFlashInfo;

    if (spinand_get_chipid(sni, &chipid) != 0) {
        LOG("spinand get chipid err!\n");
        return 0;
    }
    if (get_storage_params(sni, (u8_t *)&chipid, &NandFlashInfo) != 0) {
        LOG("Can't get flashinfo.\n");
        return 0;
    }

    zone_num = NandFlashInfo->TotalBlkNumPerDie / NandFlashInfo->DefaultLBlkNumPer1024Blk;
    max_sectors = zone_num * NandFlashInfo->DefaultLBlkNumPer1024Blk * NandFlashInfo->SectorNumPerPage * NandFlashInfo->PageNumPerPhyBlk;

    return max_sectors;
}

int spinand_storage_ioctl(const struct device *dev, uint8_t cmd, void *buff)
{
    k_mutex_lock(&mutex, K_FOREVER);
    struct spinand_info *sni = &spinand_acts_data;
    uint32_t chipid = 0;
    int ret;

#ifdef CONFIG_PM_DEVICE
    start_cycle = k_cycle_get_32();
    if (spinand_resume) {
        ret = spinand_resume_init(sni);
        if (ret != 0) {
            LOG("spinand resume err, spinand ioctl err!\n");
            k_mutex_unlock(&mutex);
            return ret;
        }
    }
#endif

    if (!spinand_initial) {
        LOG("%s spinand init failed, please check...\n", __func__);
        k_mutex_unlock(&mutex);
        return -1;
    }

    switch (cmd) {
    case DISK_IOCTL_CTRL_SYNC:
#ifdef CONFIG_SPINAND_LIB
        spinand_acts_flush(dev, 1);
#endif
        break;
    case DISK_IOCTL_GET_SECTOR_COUNT:
        if (spinand_max_sectors == 0) {
            LOG("Can't get spinand storage capacity.\n");
            k_mutex_unlock(&mutex);
            return -1;
        }
        *(uint32_t *)buff = spinand_max_sectors;
        break;
    case DISK_IOCTL_GET_SECTOR_SIZE:
        *(uint32_t *)buff = SECTOR_SIZE;
        break;
    case DISK_IOCTL_GET_ERASE_BLOCK_SZ:
        *(uint32_t *)buff  = SECTOR_SIZE;
        break;
    case DISK_IOCTL_HW_DETECT:
        if (spinand_get_chipid(sni, &chipid) != 0) {
            *(uint8_t *)buff = STA_NODISK;
        } else {
            *(uint8_t *)buff = STA_DISK_OK;
        }
        break;
    default:
        k_mutex_unlock(&mutex);
        return -EINVAL;
    }

    k_mutex_unlock(&mutex);

    return 0;
}

static int spinand_power_manager(bool on)
{
    if (CONFIG_SPINAND_USE_GPIO_POWER) {
        int ret = 0;
        const struct device *power_gpio_dev;
        uint8_t power_gpio = CONFIG_SPINAND_POWER_GPIO % 32;
        power_gpio_dev = device_get_binding(CONFIG_GPIO_PIN2NAME(CONFIG_SPINAND_POWER_GPIO));

        if (!power_gpio_dev) {
            LOG("Failed to bind spinand power GPIO(%d:%s)",
            power_gpio, CONFIG_GPIO_PIN2NAME(CONFIG_SPINAND_POWER_GPIO));
            return -1;
        }

        ret = gpio_pin_configure(power_gpio_dev, power_gpio, GPIO_OUTPUT);
        if (ret) {
            LOG("Failed to config output GPIO:%d", power_gpio);
            return ret;
        }

        if (on) {
            /* power on spinand */
            gpio_pin_set(power_gpio_dev, power_gpio, spinand_power_level);
            //k_busy_wait(10*1000);
            k_msleep(10);
        } else {
            /* power off spinand */
            gpio_pin_set(power_gpio_dev, power_gpio, !spinand_power_level);
        }

        //printk("GPIO64 = 0x%x \n", sys_read32(0x40068104));
        //printk("GPIO64 Output = 0x%x \n", sys_read32(0x40068208));
    }

    return 0;
}

static uint8_t spinand_try_power_level(struct spinand_info *sni)
{
    if (CONFIG_SPINAND_USE_GPIO_POWER) {
        int ret = 0;
        const struct device *power_gpio_dev;
        uint8_t power_gpio = CONFIG_SPINAND_POWER_GPIO % 32;
        power_gpio_dev = device_get_binding(CONFIG_GPIO_PIN2NAME(CONFIG_SPINAND_POWER_GPIO));

        if (!power_gpio_dev) {
            LOG("Failed to bind spinand power GPIO(%d:%s)",
            power_gpio, CONFIG_GPIO_PIN2NAME(CONFIG_SPINAND_POWER_GPIO));
            return -1;
        }

        ret = gpio_pin_configure(power_gpio_dev, power_gpio, GPIO_OUTPUT);
        if (ret) {
            LOG("Failed to config output GPIO:%d", power_gpio);
            return ret;
        }

        uint32_t chipid = 0;
        
        /* setup SPI clock rate, 16MHz */
        clk_set_rate(CLOCK_ID_SPI0+CONFIG_SPINAND_USE_SPICONTROLER, MHZ(16));
        
        /* power on spinand */
        gpio_pin_set(power_gpio_dev, power_gpio, CONFIG_SPINAND_GPIO_POWER_LEVEL);
        //k_busy_wait(10*1000);
        k_msleep(10);
        set_spinand_spimfp();
        if (spinand_get_chipid(sni, &chipid) != 0) {
            LOG("Can't get spinand chipid by using GPIO%d powel_level:%s.\n", CONFIG_SPINAND_POWER_GPIO, (CONFIG_SPINAND_GPIO_POWER_LEVEL > 0 ? "high" : "low"));
		    LOG("Reverse power level and retry ...\n");
            gpio_pin_set(power_gpio_dev, power_gpio, 0);
            return (!CONFIG_SPINAND_GPIO_POWER_LEVEL);
        } else {
            return CONFIG_SPINAND_GPIO_POWER_LEVEL;
        }
    }

    return 0;
}

static int spinand_env_init(struct spinand_info *sni)
{
    uint8_t feture;
    //uint32_t chipid = 0;
    int delay_chain = 0;

    /* enable spi controller clock */
    acts_clock_peripheral_enable(CLOCK_ID_SPI0+CONFIG_SPINAND_USE_SPICONTROLER);

    /* reset spi controller */
    acts_reset_peripheral(RESET_ID_SPI0+CONFIG_SPINAND_USE_SPICONTROLER);

    if (!spinand_resume)
        spinand_power_level = spinand_try_power_level(sni);

    if (spinand_power_manager(1) != 0) {
        LOG("spinand power on failed!\n");
        return -1;
    }

    if (sni->spi->delay_chain == 0) {
        delay_chain = spinand_get_delaychain(sni);
        if (delay_chain < 0 || delay_chain > 15) {
            LOG("spinand get delaychain failed!\n");
            return -1;
        }
        sni->spi->delay_chain = delay_chain;
        LOG("spinand set delaychain %d.\n", sni->spi->delay_chain);
    }

    spinand_max_sectors = spinand_get_storage_capacity(sni);
    if (spinand_max_sectors == 0) {
        LOG("spinand get storage capacity failed!\n");
        return -1;
    }
    LOG("spinand contain sectors 0x%x.\n", spinand_max_sectors);

    /* setup SPI clock rate */
    clk_set_rate(CLOCK_ID_SPI0+CONFIG_SPINAND_USE_SPICONTROLER, MHZ(spinand_dvfs_clk));

    //if use 4xIO, SIO2 & SIO3 do not need pull up, otherwise, SIO2 & SIO3 need setting pull up.
    if (sni->spi->bus_width == 4) {
        feture = spinand_get_feature(sni, 0xB0);
        spinand_set_feature(sni, 0xB0, feture | 0x1);
    }

    //For debug only.
    if (0) {
        LOG("MRCR0 = 0x%x \n", sys_read32(RMU_MRCR0));
        //bit7 for spi3
        LOG("CMU_DEVCKEN0 = 0x%x \n", sys_read32(CMU_DEVCLKEN0));
        LOG("CORE_PLL = 0x%x \n", sys_read32(COREPLL_CTL));
        LOG("SPI3CLK  = 0x%x \n", sys_read32(CMU_SPI3CLK));
        LOG("GOIO64   = 0x%x \n", sys_read32(0x40068104));
        LOG("GPIO64 Output = 0x%x \n", sys_read32(0x40068208));
        LOG("GPIO8  = 0x%x \n", sys_read32(0x40068024));
        LOG("GPIO9  = 0x%x \n", sys_read32(0x40068028));
        LOG("GPIO10 = 0x%x \n", sys_read32(0x4006802c));
        LOG("GPIO11 = 0x%x \n", sys_read32(0x40068030));
        LOG("GPIO12 = 0x%x \n", sys_read32(0x40068034));
        LOG("GPIO13 = 0x%x \n", sys_read32(0x40068038));
    }

    return 0;
}

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
__dvfs_notifier_func static void spinand_dvfs_notify(void *user_data, struct dvfs_freqs *dvfs_freq)
{
	struct dvfs_level *old_dvfs_level, *new_dvfs_level;

	if (!dvfs_freq) {
		LOG("dvfs notify invalid param");
		return ;
	}

	if (dvfs_freq->old_level == dvfs_freq->new_level)
		return;

	old_dvfs_level = dvfs_get_info_by_level_id(dvfs_freq->old_level);
	new_dvfs_level = dvfs_get_info_by_level_id(dvfs_freq->new_level);

    if (dvfs_freq->state == DVFS_EVENT_PRE_CHANGE) {
        k_mutex_lock(&mutex, K_FOREVER);
    	if (new_dvfs_level->vdd_volt >= 1100)
            spinand_dvfs_clk = CONFIG_SPINAND_FLASH_FREQ_MHZ;
        else
            spinand_dvfs_clk = 64;

        clk_set_rate(CLOCK_ID_SPI0+CONFIG_SPINAND_USE_SPICONTROLER, MHZ(spinand_dvfs_clk));
    
    	LOG("spi%dclk update by vdd:%d => %d\n",
    			CONFIG_SPINAND_USE_SPICONTROLER, old_dvfs_level->vdd_volt, new_dvfs_level->vdd_volt);
    } else if (dvfs_freq->state == DVFS_EVENT_POST_CHANGE) {
        k_mutex_unlock(&mutex);
    }
}

static struct dvfs_notifier __dvfs_notifier_data spinand_dvfs_notifier = {
	.dvfs_notify_func_t = spinand_dvfs_notify,
	.user_data = NULL,
};
#endif

void set_spinand_gpiohighz()
{
    struct board_pinmux_info pinmux_info;
    board_get_spinand_gpiohighz_info(&pinmux_info);
    acts_pinmux_setup_pins(pinmux_info.pins_config, pinmux_info.pins_num);
}

void set_spinand_spimfp()
{
    struct board_pinmux_info pinmux_info;
    board_get_spinand_pinmux_info(&pinmux_info);
    acts_pinmux_setup_pins(pinmux_info.pins_config, pinmux_info.pins_num);
}


static int spinand_acts_init(const struct device *dev)
{
    int ret = 0;
    //const struct spinand_acts_config *config = DEV_CFG(dev);
    struct spinand_info *sni = &spinand_acts_data;

    sni->bss = api_bss;
    memset(sni->bss, 0x0, HEAP_SIZE);
    sni->id_tbl = (void *)soc_boot_get_nandid_tbl_addr();
    sni->spi = &system_spinand;
    spinand_initial = false;
    spinand_resume = false;
    spinand_poweroff = false;
    //default clk is 96MHz.
    spinand_dvfs_clk = CONFIG_SPINAND_FLASH_FREQ_MHZ;

    set_spinand_gpiohighz();
    //set_spinand_spimfp();
    if (spinand_env_init(sni) != 0) {
        LOG("spinand env init err.\n");
        return -1;
    }

#ifndef CONFIG_SPINAND_LIB
    ret = p_spinand_api->init(sni);
#else
    LOG("spinand version %s \n", spinand_get_version());
    ret = spinand_init(sni);
#endif
    if (ret != 0) {
        LOG("SPINand rom driver init err.\n");
        return -1;
    }

    k_mutex_init(&mutex);

#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL
	dvfs_register_notifier(&spinand_dvfs_notifier);
#endif

#ifdef CONFIG_PM_DEVICE
	k_work_queue_start(&spinand_workq, spinand_workq_stack, K_THREAD_STACK_SIZEOF(spinand_workq_stack), 7, NULL);
    start_cycle = k_cycle_get_32();
	k_delayed_work_init(&power_control, spinand_workqueue_poweroff);
	k_delayed_work_submit_to_queue(&spinand_workq, &power_control, K_MSEC(1000));
#endif

    spinand_initial = true;

    return 0;
}

#ifdef CONFIG_PM_DEVICE
static u32_t bss_checksum;
#define CHECKSUM_XOR_VALUE      0xaa55
u32_t spinand_checksum(u32_t *buf, u32_t len)
{
    u32_t i;
    u32_t sum = 0;

    for (i = 0; i < len; i++)
    {
        sum += buf[i];
    }

    sum ^= (uint16_t)CHECKSUM_XOR_VALUE;

    return sum;
}

void spinand_workqueue_poweroff(struct k_work *work)
{
    k_mutex_lock(&mutex, K_FOREVER);
    u32_t time_interval = k_cyc_to_ms_floor32(k_cycle_get_32() - start_cycle);
    //LOG("spinand check time_interval %u\n", time_interval);
    if (time_interval >= CONFIG_SPINAND_POWER_CONTROL_SECONDS * 1000 && !spinand_poweroff) {
        LOG("spinand power off ...\n");
        if (spinand_power_manager(0) != 0) {
            LOG("spinand power off failed!\n");
			k_delayed_work_submit_to_queue(&spinand_workq, &power_control, K_MSEC(1000));
            k_mutex_unlock(&mutex);
            return ;
        }
        set_spinand_gpiohighz();
        spinand_poweroff = true;
        //set resume flag for later poweron.
        spinand_resume = true;
        spinand_initial = false;
        bss_checksum = spinand_checksum(api_bss, sizeof(api_bss)/4);
        k_mutex_unlock(&mutex);
        return ;
    }
	k_delayed_work_submit_to_queue(&spinand_workq, &power_control, K_MSEC(1000));
    k_mutex_unlock(&mutex);
}

int spinand_resume_init(struct spinand_info *sni)
{
    int ret = 0;
    u32_t start = k_cycle_get_32();

    if (spinand_poweroff) {
        LOG("spinand power on ...\n");
        u32_t tmp_cksum = spinand_checksum(api_bss, sizeof(api_bss)/4);
        if (bss_checksum != tmp_cksum) {
            LOG("SPINand resume err! api_bss is changed! suspend_checksum = 0x%x; resume_checksum = 0x%x \n", bss_checksum, tmp_cksum);
            return -1;
        }
        spinand_poweroff = false;
        k_delayed_work_submit_to_queue(&spinand_workq, &power_control, K_MSEC(1000));
    }

    set_spinand_spimfp();
    if (spinand_env_init(sni) != 0) {
        LOG("spinand env init err.\n");
        return -1;
    }

#ifndef CONFIG_SPINAND_LIB
    ret = p_spinand_api->init(sni);
#else
    ret = spinand_pdl_init(sni);
#endif
    if (ret != 0) {
        LOG("SPINand rom driver init err.\n");
        return -1;
    }
    
    if (spinand_resume)
        spinand_resume = false;

    spinand_initial = true;
    LOG("spinand power on time cost (us)=%u\n", k_cyc_to_us_floor32(k_cycle_get_32() - start));

    return ret;
}

int spinand_pm_control(const struct device *device, enum pm_device_action action)
{
    int ret;
    u32_t tmp_cksum;
    //struct spinand_info *sni = &spinand_acts_data;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        LOG("spinand resume ...\n");
        tmp_cksum = spinand_checksum(api_bss, sizeof(api_bss)/4);
        if (bss_checksum != tmp_cksum) {
            LOG("SPINand resume err! api_bss is changed! suspend_checksum = 0x%x; resume_checksum = 0x%x \n", bss_checksum, tmp_cksum);
            return -1;
        }
        //LOG("resume_checksum = 0x%x; sizeof(api_bss) = 0x%x \n", tmp_cksum, sizeof(api_bss));
        spinand_resume = true;
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        if (mutex.lock_count != 0U) {
            LOG("spinand is using, can't suspend ...\n");
            return -1;
        }
        LOG("spinand suspend ...\n");
        if (spinand_power_manager(0) != 0) {
            LOG("spinand power off failed!\n");
            return -1;
        }
        set_spinand_gpiohighz();
        bss_checksum = spinand_checksum(api_bss, sizeof(api_bss)/4);
        //LOG("suspend_checksum = 0x%x; sizeof(api_bss) = 0x%x \n", bss_checksum, sizeof(api_bss));
        break;
    case PM_DEVICE_ACTION_EARLY_SUSPEND:
        break;
    case PM_DEVICE_ACTION_LATE_RESUME:
        break;
    case PM_DEVICE_ACTION_TURN_OFF:
#ifdef CONFIG_SPINAND_LIB
        //flush all data to spinand.
        spinand_acts_flush(device, 0);
#endif
        break;
    default:
        ret = -EINVAL;
    }
    return 0;
}
#else
#define adc_pm_control 	NULL
#endif

static struct flash_driver_api spinand_api = {
    .read = spinand_acts_read,
    .write = spinand_acts_write,
    .erase = spinand_acts_erase,
    .write_protection = NULL,
#ifdef CONFIG_SPINAND_LIB
    .flush = spinand_acts_flush,
#endif
};

#define SPINAND_ACTS_DEVICE_INIT(n)						\
    DEVICE_DEFINE(spinand, CONFIG_SPINAND_FLASH_NAME, \
            &spinand_acts_init, spinand_pm_control, \
            &spinand_acts_data, NULL, POST_KERNEL, \
            CONFIG_KERNEL_INIT_PRIORITY_OBJECTS, &spinand_api);

#if IS_ENABLED(CONFIG_SPINAND_3) || IS_ENABLED(CONFIG_SPINAND_0)
    SPINAND_ACTS_DEVICE_INIT(3)
#endif

