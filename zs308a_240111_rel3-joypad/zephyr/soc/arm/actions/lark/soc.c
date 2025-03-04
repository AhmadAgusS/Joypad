/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2016 Linaro Limited.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System/hardware module for ATJ215X processor
 */

#include <device.h>
#include <init.h>
#include <arch/cpu.h>
#include "soc.h"

//#include <arch/arm/aarch32/cortex_m/cmsis.h>


static void jtag_config(unsigned int group_id)
{
	printk("jtag switch to group=%d\n", group_id);
	if (group_id < 3)
		sys_write32((sys_read32(JTAG_CTL) & ~(3 << 0)) | (group_id << 0) | (1 << 4), JTAG_CTL);

}

void jtag_set(void)
{
	jtag_config(1);
}
/**
 * \brief  clear watchdog
 */
void soc_watchdog_clear(void)
{
	sys_set_bit(WD_CTL, 0);
}
static void wd_check_wdreset_cnt(void)
{
	uint32_t reset_cnt;
	soc_watchdog_clear();
	soc_pstore_get(SOC_PSTORE_TAG_WD_RESET_CNT, &reset_cnt);
	printk("wd cnt=%d, WD_CTL=0x%x\n", reset_cnt, sys_read32(WD_CTL));
	if(reset_cnt > 10){
		printk("reboot ota\n");
		soc_pstore_set(SOC_PSTORE_TAG_WD_RESET_CNT, 0);
		sys_pm_reboot(REBOOT_TYPE_GOTO_OTA);
	}else{
		reset_cnt++;
		soc_pstore_set(SOC_PSTORE_TAG_WD_RESET_CNT, reset_cnt);
	}
}
 /**
  * \brief	if boot to main clear wd reset cnt
  */
 void wd_clear_wdreset_cnt(void)
{
	soc_pstore_set(SOC_PSTORE_TAG_WD_RESET_CNT, 0);
}


int soc_dvfs_opt(void)
{
	return sys_read32(0x400000a0)&0x0f;
}

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 * So the init priority has to be 0 (zero).
 *
 * @return 0
 */
static int lark_init(const struct device *arg)
{
	uint32_t key;

	ARG_UNUSED(arg);

	key = irq_lock();

	/* Install default handler that simply resets the CPU
	 * if configured in the kernel, NOP otherwise
	 */
	NMI_INIT();

	irq_unlock(key);
	wd_check_wdreset_cnt();
	/* Update CMSIS SystemCoreClock variable (HCLK) */
	/* At reset, system core clock is set to 16 MHz from HSI */
	//SystemCoreClock = 16000000;
	//while(!arg);

	sys_write32(0x0, WIO0_CTL); //default set wio0 to gpio func

	//RAM4 shareRAM select HOSC(32MHZ)
	sys_write32((sys_read32(CMU_MEMCLKSRC0)&(~0x3ff)) | (0x1<<5),
				CMU_MEMCLKSRC0);
	//ANCDSP RAM select HOSC(32MHZ)
	sys_write32((sys_read32(CMU_MEMCLKSRC1)&(~0xf)) | (0x1<<1),
				CMU_MEMCLKSRC1);

	/*for lowpower*/
	//sys_write32(0x30F, SPI1_CLKGATING);

	/* init ppi */
	ppi_init();

	if(soc_dvfs_opt())
		sys_write32(sys_read32(SPI1_CACHE_CTL) | (1u << 2), SPI1_CACHE_CTL);
	return 0;
}

SYS_INIT(lark_init, PRE_KERNEL_1, 0);


/*if CONFIG_WDOG_ACTS enable , wd driver call wd_clear_wdreset_cnt */
#ifndef CONFIG_WDOG_ACTS
/**
 * @brief Perform basic hardware initialization at boot.
 *
 * before boot to main clear wd reset cnt
 * @return 0
 */
static int wd_reset_cnt_init(const struct device *arg)
{
	wd_clear_wdreset_cnt();
	sys_write32(0, WD_CTL); /*disable watch dog*/
	return 0;
}

SYS_INIT(wd_reset_cnt_init, APPLICATION, 91);
#endif

