/*
 * Copyright (c) 2021 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Actions LARK family boot related infomation public interfaces.
 */

#include <kernel.h>
#include <device.h>
#include <string.h>
#include "soc.h"
#include <linker/linker-defs.h>
#include <partition/partition.h>
#include <drivers/flash.h>
#include <board_cfg.h>


 /*mbrec mem layout*/
 /*
 0x1000000-0x1000200 //sram 1KB save boot_info(0x20) +NANDID
 0x1000020-0x1000400  // NANDID
 */
#define BOOT_INFO_SRAM_ADDR	0x1000000


uint32_t soc_boot_get_part_tbl_addr(void)
{
	return soc_boot_get_info()->param_phy_addr;
}

uint32_t soc_boot_get_fw_ver_addr(void)
{
	return (soc_boot_get_part_tbl_addr() + SOC_BOOT_FIRMWARE_VERSION_OFFSET);
}

const boot_info_t *soc_boot_get_info(void)
{
	return (const boot_info_t *)BOOT_INFO_SRAM_ADDR;
}

uint32_t soc_boot_get_nandid_tbl_addr(void)
{
	const boot_info_t *p_boot_info = soc_boot_get_info();
	return p_boot_info->nand_id_offs;
}

u32_t soc_boot_get_reboot_reason(void)
{
	const boot_info_t *p_boot_info = soc_boot_get_info();
	return p_boot_info->reboot_reason;
}

bool soc_boot_get_watchdog_is_reboot(void)
{
	const boot_info_t *p_boot_info = soc_boot_get_info();
	return !!p_boot_info->watchdog_reboot;
}

void soc_boot_set_system_phy_addr(uint32_t system_phy_addr)
{
	boot_info_t *p_boot_info = (boot_info_t*)soc_boot_get_info();
	p_boot_info->system_phy_addr = system_phy_addr;
}

#ifdef CONFIG_SPI_FLASH_ACTS
void  sys_norflash_exit_4b(void);
#endif

#ifdef CONFIG_PM_DEVICE
extern int pm_power_off_devices(void);
#endif

 #ifdef CONFIG_SOC_SIGN_ENABLE

static uint8_t g_key_data[0x400];
static uint16_t g_key_len;
#define BOOT_RUN_ADDR 0x11000000

static void sign_load_key(void)
{
	uint16_t key_len;
    uint8_t *key;
	key = p_brom_api->p_image_tlv_find((void*)BOOT_RUN_ADDR, IMAGE_TLV_PUBKEY, &key_len, 1);
	if(key == NULL){
		printk("sign_load_key fail\n");
		g_key_len = 0;
	}else{
		g_key_len = key_len;
		memcpy(g_key_data, key, key_len);
		printk("sign_load_key ok, key_len=0x%x\n", key_len);
	}
}

 /*0 is ok else fail*/
__ramfunc int image_security_data_check(image_head_t *ih_app)
{
	uint8_t *sig_data;
	uint16_t  sig_len;
	uint32_t all_len;

	sig_data =p_brom_api->p_image_tlv_find(ih_app, IMAGE_TLV_RSA2048_PSS, &sig_len, 0);
	if(sig_data == NULL){
		return 0x1001;
	}
	if(!g_key_len)
		return 0x1002;

	all_len =  ih_app->ih_hdr_size + ih_app->ih_img_size + ih_app->ih_ptlv_size;
    return p_brom_api->p_verify_signature(g_key_data, sig_data, (const uint8_t *)(ih_app), all_len);
}

#else
__ramfunc int image_security_data_check(image_head_t *ih_app)
{
	return 0;
}
static void sign_load_key(void)
{

}
#endif



extern uint32_t libboot_version_dump(void);

int boot_to_part(int part_type, int mirror_id, int flashid)
{
	const struct partition_entry *part;
	int crc_is_enabled;

#ifdef CONFIG_ACTIONS_PRINTK_DMA
	printk_dma_switch(0);
#endif
	part = partition_get_part_by_type_mirrorid(part_type, mirror_id);
	if(part == NULL){
		printk("not find  app parition--\n");
		return -1;
	}
	crc_is_enabled = part->flag & PARTITION_FLAG_ENABLE_CRC ? 1 : 0;
	printk("app offset=0x%x ,crc=%d\n", part->file_offset, crc_is_enabled);
	soc_boot_set_system_phy_addr(part->file_offset);

	if(flashid == BOOT_FLASH_ID_NAND){
		boot_nand(part->file_offset, 0);
	}else if(flashid == BOOT_FLASH_ID_EXTNOR){	
#ifdef CONFIG_SPI_FLASH_ACTS
		#ifdef CONFIG_SPI_NOR_FLASH_4B_ADDRESS
		sys_norflash_exit_4b();
		#endif
#endif
		boot_ext_nor(part->file_offset, crc_is_enabled, 0);
	}else{
		boot_nor(part->file_offset, crc_is_enabled, 0);
	}
	printk("boot fail\n");

	__asm__ volatile("cpsie	i");
	__asm__ volatile("dsb");

	return 0;
}

int boot_to_app(int mirror_id, int flashid)
{
	libboot_version_dump();
	printk("boot main=%d \n", flashid);
	sign_load_key();
	irq_lock();
#ifdef CONFIG_PM_DEVICE
	printk("dev power off\n");
	pm_power_off_devices();
	printk("dev power end\n");
#endif

	return boot_to_part(PARTITION_TYPE_SYSTEM, mirror_id, flashid);
}


