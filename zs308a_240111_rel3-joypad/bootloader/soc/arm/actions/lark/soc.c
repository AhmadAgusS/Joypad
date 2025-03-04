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


#define GPION(x)  		GPIO_REG_CTL(GPIO_REG_BASE, x)
#define GPIO_BSR(x)		GPIO_REG_BSR(GPIO_REG_BASE, x)
#define GPIO_BRR(x)		GPIO_REG_BRR(GPIO_REG_BASE, x)
#define GPIO_IDATA(x)	GPIO_REG_IDAT(GPIO_REG_BASE, x)

int check_adfu_connect(unsigned int gpio_in, unsigned int gpio_out)
{
	unsigned int checksum, org_checksum = 0x55aa55aa;
	unsigned int rdat, sumdat = 0;
	unsigned int gpio_rx_bak, gpio_tx_bak, i;

	printk("check txrx adfu\n");
	gpio_rx_bak= sys_read32(GPION(gpio_in));
	gpio_tx_bak= sys_read32(GPION(gpio_out));
	/* RX - INPUT */
	sys_write32(0x3880, GPION(gpio_in));
	/* TX - OUTPUT */
	sys_write32(0x3840, GPION(gpio_out));
	checksum = org_checksum;
	for (i = 0; i < 32; i++) {
		/* write data */
		if(checksum & 0x1){
			sys_write32(GPIO_BIT(gpio_out), GPIO_BSR(gpio_out));
		}else{
			sys_write32(GPIO_BIT(gpio_out), GPIO_BRR(gpio_out));
		}
		k_busy_wait(500);
		/* read data */
		if(sys_read32(GPIO_IDATA(gpio_in)) & GPIO_BIT(gpio_in))
			rdat = 1;
		else
			rdat = 0;
		sumdat |= (rdat << i);
		checksum >>= 1;
	}
	sys_write32(gpio_rx_bak, GPION(gpio_in));
	sys_write32(gpio_tx_bak, GPION(gpio_out));
	if (org_checksum == sumdat){
		printk("txrx enter adfu\n");
		return 1;/*enter adfu*/
	}
	return 0;
}

int check_adfu_gpiokey(unsigned int gpio)
{
	int ret;
	unsigned int gpio_bak;
	if(gpio >= GPIO_MAX_PIN_NUM)
		return 0;
	printk("check gpio adfu\n");
	gpio_bak= sys_read32(GPION(gpio));
	/* INPUT */
	sys_write32(0x3880, GPION(gpio));
	k_busy_wait(10);
	if(sys_read32(GPIO_IDATA(gpio)) & GPIO_BIT(gpio)){
		ret = 0;
	}else{
		printk("gpio key enter adfu\n");
		ret = 1;
	}
	sys_write32(gpio_bak, GPION(gpio));
	return ret;
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

	/* Update CMSIS SystemCoreClock variable (HCLK) */
	/* At reset, system core clock is set to 16 MHz from HSI */
	//SystemCoreClock = 16000000;
	//while(!arg);

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
	
	return 0;
}

SYS_INIT(lark_init, PRE_KERNEL_1, 0);
