/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file peripheral clock interface for Actions SoC
 */

#include <kernel.h>
#include <device.h>
#include <soc.h>
extern void cmu_dev_clk_enable(uint32_t id);
extern void cmu_dev_clk_disable(uint32_t id);

static void acts_clock_peripheral_control(int clock_id, int enable)
{
	unsigned int key;

	if (clock_id > CLOCK_ID_MAX_ID)
		return;

	key = irq_lock();

	if (enable) {
		if (clock_id < 32) {
			sys_set_bit(CMU_DEVCLKEN0, clock_id);
		} else {
			sys_set_bit(CMU_DEVCLKEN1, clock_id - 32);
		}
	} else {
		if (clock_id < 32) {
			sys_clear_bit(CMU_DEVCLKEN0, clock_id);
		} else {
			sys_clear_bit(CMU_DEVCLKEN1, clock_id - 32);
		}
	}
	irq_unlock(key);
}

void acts_clock_peripheral_enable(int clock_id)
{
	acts_clock_peripheral_control(clock_id, 1);
}

void acts_clock_peripheral_disable(int clock_id)
{
	acts_clock_peripheral_control(clock_id, 0);
}



uint32_t clk_rate_get_corepll(void)
{
	return MHZ(((sys_read32(COREPLL_CTL)&0X3F)*8));

}




#ifdef CONFIG_MMC_ACTS
static void  acts_clk_set_rate_sd(int sd, unsigned int rate_hz)
{
	unsigned int core_pll, hosc_hz, val, real_rate, div;
	core_pll = clk_rate_get_corepll();
	hosc_hz = CONFIG_HOSC_CLK_MHZ*1000000;

	/*
	* Set the RDELAY and WDELAY based on the sd clk.
	*/
	if (rate_hz < hosc_hz/16) {
		/* clock source: HOSC, 1/128, real freq: 188KHz */
		div = (hosc_hz+(128*rate_hz)-1)/(128*rate_hz);
		val = 0x40 + div-1 ;
		real_rate = hosc_hz/(128*div);

	} else if (rate_hz <= hosc_hz) {
		/* clock source: HOSC, real freq: 1.5M~24M */
		div = (hosc_hz+rate_hz-1)/rate_hz;
		val = div - 1;
		real_rate = hosc_hz/div;
	} else {
		/* clock source: core_pll, real freq: 200MHz */
		div = (core_pll+rate_hz-1)/rate_hz;
		real_rate = core_pll/div;
		val = (div-1)|(1<<8);
	}
	sys_write32(val, CMU_SD0CLK+sd*4);

	printk("mmc%d: set rate %d Hz, real rate %d Hz, core pll=%d HZ\n",
		sd, rate_hz, real_rate, core_pll);

}
#else
static void  acts_clk_set_rate_sd(int sd, unsigned int rate_hz)
{

}
#endif


#ifdef CONFIG_SOC_SPI0_USE_CK64M

static unsigned int calc_spi_clk_div(unsigned int max_freq, unsigned int spi_freq)
{
	unsigned int div;

	if (max_freq > spi_freq && max_freq <= (spi_freq * 3 / 2)) {
		/* /1.5 */
		div = 14;
	} else if ((max_freq > 2 * spi_freq) && max_freq <= (spi_freq * 5 / 2)) {
		/* /2.5 */
		div = 15;
	} else {
		/* /n */
		div = (max_freq + spi_freq - 1) / spi_freq - 1;
		if (div > 13) {
			div = 13;
		}
	}

	return div;
}

static void __acts_clk_set_rate_spi_ck64m(int clock_id, unsigned int rate_hz)
{
	unsigned int div, reg_val, real_rate;

	div = calc_spi_clk_div(MHZ(64), rate_hz) & 0xf;

	/* check CK64M has been enabled or not */
	if (!(sys_read32(CMU_S1CLKCTL) & (1 << 2))) {
		/* enable S1 CK64M */
		sys_write32(sys_read32(CMU_S1CLKCTL) | (1 << 2), CMU_S1CLKCTL);
		/* enable S1BT CK64M */
		sys_write32(sys_read32(CMU_S1BTCLKCTL) | (1 << 2), CMU_S1BTCLKCTL);
		k_busy_wait(10);
		/* calibrate CK64M clock */
		sys_write32(0xe, CK64M_CTL);
		/* wait calibration done */
		while(!(sys_read32(CK64M_CTL) & (1 << 8))) {
			;
		}
	}

	/* set SPIx clock source and divison */
	reg_val = sys_read32(CMU_SPI0CLK + ((clock_id - CLOCK_ID_SPI0) * 4));
	reg_val &= ~0x30f;
	reg_val |= (0x3 << 8) | (div << 0);

	if (div == 14)
		real_rate = MHZ(64) * 2 / 3;
	else if (div == 15)
		real_rate = MHZ(64) * 2 / 5;
	else
		real_rate = MHZ(64) / (div + 1);

	sys_write32(reg_val, CMU_SPI0CLK + ((clock_id - CLOCK_ID_SPI0) * 4));

	printk("SPI%d: set rate %d Hz real rate %d Hz\n",
			clock_id - CLOCK_ID_SPI0, rate_hz, real_rate);
}

#endif
static void  acts_clk_set_rate_spi(int clock_id, unsigned int rate_hz)
{
	unsigned int core_pll, val, real_rate, div;

#ifdef CONFIG_SOC_SPI0_USE_CK64M
	if (CLOCK_ID_SPI0 == clock_id)
		return __acts_clk_set_rate_spi_ck64m(CLOCK_ID_SPI0, rate_hz);
#endif

#ifndef CONFIG_SOC_EP_MODE

	core_pll = clk_rate_get_corepll();
	div = (core_pll+rate_hz-1)/rate_hz;
	real_rate = core_pll/div;
	val = (div-1)|(1<<8);

#else
	val = 0;
	real_rate = 32;
#endif
	sys_write32(val, CMU_SPI0CLK + (clock_id - CLOCK_ID_SPI0)*4);

	printk("SPI%d: set rate %d Hz, real rate %d Hz\n",
		clock_id - CLOCK_ID_SPI0, rate_hz, real_rate);
}


#ifdef CONFIG_SPIMT_ACTS

static void  acts_clk_set_rate_spimt(int clock_id, unsigned int rate_hz)
{
	unsigned int val, div, real_rate;

	if (rate_hz > 4000000) {
		rate_hz = 4000000;
	}
	div = 4000000 / rate_hz;
	real_rate = 4000000 / div;
	val = (0 << 8) | (div - 1);
	sys_write32(val, CMU_SPIMT0CLK + (clock_id - CLOCK_ID_SPIMT0)*4);

	printk("SPIMT%d: set rate %d Hz, real rate %d Hz\n",
		clock_id - CLOCK_ID_SPIMT0, rate_hz, real_rate);
}
#else
static void  acts_clk_set_rate_spimt(int clock_id, unsigned int rate_hz)
{

}
#endif

#ifdef CONFIG_DISPLAY_LCDC
static void acts_clk_set_rate_lcd(unsigned int rate_hz)
{
	uint32_t core_pll, real_rate, div;

	core_pll = clk_rate_get_corepll();
	div = (core_pll + rate_hz - 1) / rate_hz;
	if (div < 1) div = 1;
	else if (div > 12) div = 12;

	real_rate = core_pll / div;
	sys_write32((1 << 8) | (div - 1), CMU_LCDCLK);

	printk("LCD: set rate %d Hz, real rate %d Hz\n", rate_hz, real_rate);
}
#else
static void acts_clk_set_rate_lcd(unsigned int rate_hz)
{
}
#endif

#ifdef CONFIG_DISPLAY_ENGINE
static void acts_clk_set_rate_de(unsigned int rate_hz)
{
	uint32_t core_pll, real_rate, div2, div;

	core_pll = clk_rate_get_corepll();
	div2 = (core_pll * 2 + rate_hz - 1) / rate_hz;

	switch (div2) {
	case 3:
		div = 15; /* +1 */
		break;
	case 5:
		div = 16; /* +1 */
		break;
	default:
		div = div2 / 2;
		if (div < 1) div = 1;
		else if (div > 14) div = 14;

		div2 = div * 2;
		break;
	}

	real_rate = core_pll * 2 / div2;

	sys_write32((1 << 8) | (div - 1), CMU_DECLK);

	printk("DE: set rate %d Hz, real rate %d Hz\n", rate_hz, real_rate);
}
#else
static void acts_clk_set_rate_de(unsigned int rate_hz)
{
}
#endif

int clk_set_rate(int clock_id,  uint32_t rate_hz)
{
	int ret = 0;
	switch(clock_id) {

	case CLOCK_ID_SD0:
	case CLOCK_ID_SD1:
		acts_clk_set_rate_sd(clock_id-CLOCK_ID_SD0, rate_hz);
		break;
	case CLOCK_ID_DMA:
		break;
	case CLOCK_ID_SPI0:
	case CLOCK_ID_SPI1:
	case CLOCK_ID_SPI2:
	case CLOCK_ID_SPI3:
		acts_clk_set_rate_spi(clock_id, rate_hz);
		break;
	case CLOCK_ID_SPIMT0:
	case CLOCK_ID_SPIMT1:
		acts_clk_set_rate_spimt(clock_id, rate_hz);
		break;
	case CLOCK_ID_LCD:
		acts_clk_set_rate_lcd(rate_hz);
		break;
	case CLOCK_ID_DE:
		acts_clk_set_rate_de(rate_hz);
		break;
	case CLOCK_ID_I2CMT0:
	case CLOCK_ID_I2CMT1:
	case CLOCK_ID_SPI0CACHE:
	case CLOCK_ID_SPI1CACHE:
	case CLOCK_ID_USB:
	case CLOCK_ID_USB2:
	case CLOCK_ID_DSI:
	case CLOCK_ID_SE:
	case CLOCK_ID_PWM:
	case CLOCK_ID_TIMER:
	case CLOCK_ID_LRADC:
	case CLOCK_ID_UART0:
	case CLOCK_ID_UART1:
	case CLOCK_ID_UART2:
	case CLOCK_ID_I2C0:
	case CLOCK_ID_I2C1:
	case CLOCK_ID_FFT:
	case CLOCK_ID_DSP:
	case CLOCK_ID_ASRC:
	case CLOCK_ID_DAC:
	case CLOCK_ID_ADC:
	case CLOCK_ID_I2STX:
	case CLOCK_ID_I2SRX:
		printk("clkid=%d not support clk set\n",clock_id);
		ret = -1;
		break;

	}

	return 0;
}

uint32_t clk_get_rate(int clock_id)
{

	uint32_t rate = 0;
	switch(clock_id) {
	case CLOCK_ID_SD0:
	case CLOCK_ID_SD1:
	case CLOCK_ID_DMA:
	case CLOCK_ID_SPI0:
	case CLOCK_ID_SPI1:
	case CLOCK_ID_SPI2:
	case CLOCK_ID_SPI3:
	case CLOCK_ID_SPI0CACHE:
	case CLOCK_ID_SPI1CACHE:
	case CLOCK_ID_USB:
	case CLOCK_ID_USB2:
	case CLOCK_ID_DE:
	case CLOCK_ID_DSI:
	case CLOCK_ID_LCD:
	case CLOCK_ID_SE:
	case CLOCK_ID_PWM:
	case CLOCK_ID_TIMER:
	case CLOCK_ID_LRADC:
	case CLOCK_ID_UART0:
	case CLOCK_ID_UART1:
	case CLOCK_ID_UART2:
	case CLOCK_ID_I2C0:
	case CLOCK_ID_I2C1:
	case CLOCK_ID_FFT:
	case CLOCK_ID_DSP:
	case CLOCK_ID_ASRC:
	case CLOCK_ID_DAC:
	case CLOCK_ID_ADC:
	case CLOCK_ID_I2STX:
	case CLOCK_ID_I2SRX:
		printk("clkid=%d not support clk get\n",clock_id);
		break;

	}

	return rate;

}


/*set cpu clk, return old clk*/
#if 0
uint32_t clk_cpu_set(uint32_t mhz)
{

	unsigned int core_pll, div, tmp, real_rate;
	core_pll = (sys_read32(COREPLL_CTL)&0x3F)*80;
	tmp = core_pll/mhz;
	if(tmp > 13){// div 1.5
		real_rate = core_pll/15;
		div = 14;
	} else if ( (tmp < 28) && (tmp > 22) ) { // div 2.5
		real_rate = core_pll/25;
		div = 15;

	}else if( tmp > 140){
		real_rate = core_pll/14;
		div = 13;
	}else{
		div = (tmp/10)-1;
		real_rate = core_pll/10;
	}
	tmp = sys_read32(CMU_SYSCLK);
	sys_write32((old & (~(0x3<<8))) | val , CMU_SYSCLK);
	printk("cpu: set rate %d MHz, real rate %d MHz, core pll=%d MHZ\n", mhz, real_rate, core_pll);

}
#endif
/*set ahb div, return old div*/
uint32_t clk_ahb_set(uint32_t div)
{
	uint32_t val, old;
	old = sys_read32(CMU_SYSCLK);
	if(div == 1)
		val = 1<<8;
	else if (div == 4)
		val = 3<<8;
	else
		val = 0;
	div = (old >>8) & 0x03;
	sys_write32((old & (~(0x3<<8))) | val , CMU_SYSCLK);
	return div;
}




