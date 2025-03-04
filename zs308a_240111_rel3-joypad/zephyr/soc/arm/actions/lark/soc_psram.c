/*
 * Copyright (c) 2021 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Actions PSRAM Implementation.
 */

#include <kernel.h>
#include <device.h>
#include <zephyr.h>
#include <soc.h>
#include <linker/linker-defs.h>
#include <dvfs.h>

#define SPI1_DDR_ADDR0		(SPI1_CTL + 0x2C)
#define SPI1_DDR_ADDR1      (SPI1_CTL + 0x30)
#define SPI1_DDR_TRXLEN		(SPI1_CTL + 0x34)
#define SPI1_DDR_CMD		(SPI1_CTL + 0x20)
#define SPI1_DDR_START		(SPI1_CTL + 0x24)
#define SPI1_TXDAT			(SPI1_CTL + 0x08)
#define SPI1_DDR_STATUS		(SPI1_CTL + 0x28)
#define SPI1_STA			(SPI1_CTL + 0x04)
#define SPI1_RXDAT			(SPI1_CTL + 0x0C)

#define PSRAM_CS_PIN        (40)
#ifndef CONFIG_SOC_NO_PSRAM
__sleepfunc void   __psram_reg_write(unsigned int reg_addr, unsigned int reg_data)
{
	uint32_t ctl, mctl;

	ctl = sys_read32(SPI1_CTL);
	mctl = sys_read32(SPI1_DDR_MODE_CTL);

	/* switch to normal */
	sys_write32(sys_read32(SPI1_CTL) | (1 << 15), SPI1_CTL);

	/* wait ready */
	while ((sys_read32(SPI1_STA) & 0x2) != 2) {
		;
	}
	/* use CPU clock, 8Bit FIFO mode */
	sys_write32(sys_read32(SPI1_CTL) & ~(3 << 30), SPI1_CTL);
	if(soc_boot_is_mini()){
		sys_write32((0<<20)|(6<<16)|(5<<12)|(4<<8)|(4<<4)|(4<<0), SPI1_DDR_MODE_CTL);			//DDR OPI OKmode
		sys_write32(0x600001, SPI1_DDR_ADDR1);
	}else{
		/* DDR OPI OKmode */
		sys_write32((0 << 20) | (5 << 16)| (5 << 12) \
					| (4 << 8) | (10 << 4) | (1 << 0), SPI1_DDR_MODE_CTL);
	}

	/* psram register address */
	sys_write32(reg_addr, SPI1_DDR_ADDR0);


	sys_write32(2, SPI1_DDR_TRXLEN);

	/* write command is 0xC0 */
	sys_write32(0xC0, SPI1_DDR_CMD);

	/* TX transmit start */
	sys_write32(0x02, SPI1_DDR_START);

	if(soc_boot_is_mini()){
		sys_write32(reg_data>>8, SPI1_TXDAT);
		sys_write32(reg_data&0xff, SPI1_TXDAT); /* must send two bytes */
	}else{
		sys_write32(reg_data, SPI1_TXDAT);
		sys_write32(0, SPI1_TXDAT); /* must send two bytes */
	}

	soc_udelay(1);/*Must delay to resolve sleep&wakeup panic*/
	/* wait transmit complete */
	while ((sys_read32(SPI1_DDR_STATUS) & 0x1) != 1);

	/* clear status bit */
	sys_write32(0x01, SPI1_DDR_STATUS);
	sys_write32(0x00, SPI1_DDR_START);

	sys_write32(ctl, SPI1_CTL);
	sys_write32(mctl, SPI1_DDR_MODE_CTL);
 
}
__sleepfunc void psram_self_refresh_control(bool low_refresh_en)
{
	if(soc_boot_is_mini())
		return;

	if (low_refresh_en) {
		__psram_reg_write(4, 0x48);
	} else {
		__psram_reg_write(4, 0x40);
		//soc_udelay(150);
	}

}

__sleepfunc void psram_power_control(bool low_power_en)
{
	volatile int loop;

	if (low_power_en) {
		if(soc_boot_is_mini())
			__psram_reg_write(1, 0x20); //Hybrid Sleep 
		else
			__psram_reg_write(6, 0xf0);
	} else {
		/* spi1 cs low  to exit deep power mode */
		sys_write32(1 << 8, GPIO_REG_BASE + GPIO_BRR0 + 4);

		/* gpio40 cs low */
		sys_write32(0x3840, GPIO_REG_BASE + 4 + PSRAM_CS_PIN * 4);

		/* 60ns  keep cs low */
		loop = 30;
		while(loop)loop--;

		/* spi1 cs high  exit deep power mode */
		sys_write32(1 << 8, GPIO_REG_BASE + GPIO_BSR0 + 4);

		/* mini is 70us  keep cs high else 150us */
	
		soc_udelay(150);

		/*  psram gpio40 cs recovery */
		sys_write32(0x2 | (0x4 << 12), GPIO_REG_BASE + 4 + PSRAM_CS_PIN * 4);
	}
}



__sleepfunc void psram_delay_chain_set(uint8_t dqs, uint8_t clkout)
{
	sys_write32((sys_read32(SPI1_DDR_MODE_CTL) & ~(0xff << 4)) \
				| ((dqs << 4) \
				| (clkout << 8)), SPI1_DDR_MODE_CTL);
	soc_udelay(1);
}


#ifdef CONFIG_ACTS_DVFS_DYNAMIC_LEVEL

struct psram_delaychain_tbl {
	uint16_t vdd_volt;
	uint8_t clkout;
	uint8_t dqs;
};

static const struct psram_delaychain_tbl psram_delaychain_tbl0[] = {
	{950, 3, 0},
	{1000, 4, 2},
	{1100, 6, 3},
	{1150, 6, 5},
	{1200, 8, 6},
};

static const struct psram_delaychain_tbl psram_delaychain_tbl1[] = {
	{950, 4, 5},
	{1000, 5, 7},
	{1100, 7, 9},
	{1150, 9, 10},
	{1200, 11, 11},
};

static const struct psram_delaychain_tbl psram_delaychain_tbl_mini[] = {
	{950, 3, 5},
	{1000, 5, 7},
	{1100, 6, 8},
	{1150, 7, 8},
	{1200, 7, 8},
};

static inline void psram_set_delaychain_by_vdd(uint16_t vdd)
{
	uint8_t i, len;
	const struct psram_delaychain_tbl *tbl;

	if(soc_boot_is_mini()){
		tbl = psram_delaychain_tbl_mini;
		len = ARRAY_SIZE(psram_delaychain_tbl_mini);		
	}else {
		if (soc_dvfs_opt()) {
			tbl = psram_delaychain_tbl1;
			len = ARRAY_SIZE(psram_delaychain_tbl1);
		} else {
			tbl = psram_delaychain_tbl0;
			len = ARRAY_SIZE(psram_delaychain_tbl0);
		}
	}

	for (i = 0; i < len; i++) {
		if (tbl[i].vdd_volt == vdd) {
			psram_delay_chain_set(tbl[i].dqs, tbl[i].clkout);
			break;
		}
	}
}

__dvfs_notifier_func static void psram_dvfs_notify(void *user_data, struct dvfs_freqs *dvfs_freq)
{
	struct dvfs_level *old_dvfs_level, *new_dvfs_level;
	uint32_t key;

	ARG_UNUSED(user_data);

	if (!dvfs_freq) {
		printk("dvfs notify invalid param");
		return ;
	}

	if (dvfs_freq->old_level == dvfs_freq->new_level)
		return ;

	key = irq_lock();

	old_dvfs_level = dvfs_get_info_by_level_id(dvfs_freq->old_level);
	new_dvfs_level = dvfs_get_info_by_level_id(dvfs_freq->new_level);

	if (old_dvfs_level->vdd_volt > new_dvfs_level->vdd_volt) {
		/* vdd voltage decrease */
		if (dvfs_freq->state == DVFS_EVENT_PRE_CHANGE) {
			psram_set_delaychain_by_vdd(new_dvfs_level->vdd_volt);
			printk("psram delaychain update by vdd:%d => %d\n",
					old_dvfs_level->vdd_volt, new_dvfs_level->vdd_volt);
		}
	} else {
		/* vdd voltage increase */
		if (dvfs_freq->state == DVFS_EVENT_POST_CHANGE) {
			psram_set_delaychain_by_vdd(new_dvfs_level->vdd_volt);
			printk("psram delaychain update by vdd:%d => %d\n",
					old_dvfs_level->vdd_volt, new_dvfs_level->vdd_volt);
		}
	}

	irq_unlock(key);
}

static struct dvfs_notifier __dvfs_notifier_data psram_dvsf_notifier = {
	.dvfs_notify_func_t = psram_dvfs_notify,
};

static int soc_psram_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	dvfs_register_notifier(&psram_dvsf_notifier);

	return 0;
}


SYS_INIT(soc_psram_init, PRE_KERNEL_1, 21);

#endif /* CONFIG_ACTS_DVFS_DYNAMIC_LEVEL */

#else // no psram
__sleepfunc void   __psram_reg_write(unsigned int reg_addr, unsigned int reg_data)
{

}
__sleepfunc void psram_self_refresh_control(bool low_refresh_en)
{

}
__sleepfunc void psram_power_control(bool low_power_en)
{
	if(!low_power_en)
		soc_udelay(150);
}
__sleepfunc void psram_delay_chain_set(uint8_t dqs, uint8_t clkout)
{

}

#endif // #ifndef CONFIG_SOC_NO_PSRAM


