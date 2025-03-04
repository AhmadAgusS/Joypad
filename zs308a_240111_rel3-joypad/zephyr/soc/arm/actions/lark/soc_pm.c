/*
 * Copyright (c) 2018 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system reboot interface for Actions SoC
 */

#include <device.h>
#include <init.h>
#include <soc.h>
#include <pm/pm.h>
#include <drivers/rtc.h>
#include <board_cfg.h>
#include <linker/linker-defs.h>

#if (CONFIG_PM_BACKUP_TIME_FUNCTION_EN == 1)
#include <drivers/nvram_config.h>
#endif

#define REBOOT_REASON_MAGIC 		0x4252	/* 'RB' */

static bool g_b_boot_abnormal;  /*ture: boot abnormal, false: boot ok*/

int sys_pm_get_wakeup_source(union sys_pm_wakeup_src *src)
{
	uint32_t wk_pd;

	if (!src)
		return -EINVAL;

	src->data = 0;

	wk_pd = soc_pmu_get_wakeup_source();

	if (wk_pd & BIT(0))
		src->t.long_onoff = 1;

	if (wk_pd & BIT(1))
		src->t.short_onoff = 1;

	if (wk_pd & BIT(13))
		src->t.bat = 1;

	if (wk_pd & BIT(5))
		src->t.wio = 1;

	if (wk_pd & BIT(12))
		src->t.remote = 1;

	if (wk_pd & BIT(4))
		src->t.alarm = 1;

	if (wk_pd & BIT(11))
		src->t.batlv = 1;

	if (wk_pd & BIT(10))
		src->t.dc5vlv = 1;

	if (wk_pd & BIT(2))
		src->t.dc5vin = 1;

	if(g_b_boot_abnormal){
		if (soc_boot_get_watchdog_is_reboot() == 1)
			src->t.watchdog = 1;
		else
			src->t.onoff_reset = 1;
	}

	return 0;
}

void sys_pm_set_wakeup_src(void)
{
	uint32_t key, val;

	key = irq_lock();

	val = sys_read32(WKEN_CTL_SVCC) & (~0x1fff);
	val |= WAKE_CTL_LONG_WKEN | WAKE_CTL_ALARM8HZ_WKEN | WAKE_CTL_WIO0LV_DETEN;

	if(!soc_pmu_get_dc5v_status()) {
		val |= WAKE_CTL_DC5VIN_WKEN;
	}

	sys_write32(val, WKEN_CTL_SVCC);
	k_busy_wait(500);

	irq_unlock(key);
}

#if (CONFIG_PM_BACKUP_TIME_FUNCTION_EN == 1)
#include <drivers/alarm.h>

void sys_pm_poweroff_backup_time(void)
{
	int ret;
	uint32_t rc32k_freq;
	const struct device *rtc_dev = device_get_binding(CONFIG_RTC_0_NAME);
	struct sys_pm_backup_time pm_bak_time = {0};
	struct rtc_time rtc_time = {0};
	rc32k_freq = acts_clock_rc32k_set_cal_cyc(300);

#if (CONFIG_RTC_CLK_SOURCE == 2) // rc32k , need calibration  each hour
	#if IS_ENABLED(CONFIG_RTC_ENABLE_CALIBRATION)
	uint32_t b_cal = 0;
	soc_pstore_get(SOC_PSTORE_TAG_RTC_RC32K_CAL, &b_cal);
	//printk("rtc cal=%d\n", b_cal);
	if(b_cal) {
		const struct device *alarm_dev = device_get_binding(CONFIG_ALARM8HZ_0_NAME);
		struct alarm_status alarm_sta;
		uint32_t cycle, cycle_8hz, ms;
		uint32_t alram_ms = 1000*60*60; // 1h wakeup
		if(alarm_dev){
			acts_alarm_get_alarm(alarm_dev, &alarm_sta);
			printk("user alarm=%d\n", alarm_sta.is_on);
			if(alarm_sta.is_on){
				pm_bak_time.user_alarm_cycles = soc_pmu_get_alarm8hz();
				cycle_8hz = soc_pmu_get_counter8hz_cycles(false);
				if(pm_bak_time.user_alarm_cycles > cycle_8hz){
					cycle = pm_bak_time.user_alarm_cycles - cycle_8hz;
				}else{
					cycle = PMU_COUTNER8HZ_MAX - cycle_8hz +  pm_bak_time.user_alarm_cycles;
				}
				printk("alarm_cnt=%d,8hz_cnt=%d, diff_cycle=%d\n", pm_bak_time.user_alarm_cycles, cycle_8hz, cycle);
				ms =  (uint64_t)cycle*32768*125 /rc32k_freq; // cal real ms
				printk("user set alarm after %d ms\n", ms);
				pm_bak_time.is_user_alarm_on = 1;
				if(ms < alram_ms *2) {  // It is multiplied by 2 because the user's alarm may be lost due to calculation error
					alram_ms = ms;	// use  user alarm
					pm_bak_time.is_user_cur_use = 1;
				}
			}
		}
		pm_bak_time.is_use_alarm_cal = 1;
		soc_pmu_alarm8hz_enable(alram_ms);
		printk("pwoer set alarm=%d ms\n", alram_ms);
	}
	#endif
#endif

	ret = soc_pmu_get_counter8hz_cycles(true);

	if ((ret > 0) && rtc_dev) {
		pm_bak_time.counter8hz_cycles = ret;
		ret = rtc_get_time(rtc_dev, &rtc_time);
		if (!ret) {
			rtc_tm_to_time(&rtc_time, &pm_bak_time.rtc_time_sec);
			pm_bak_time.rtc_time_msec = rtc_time.tm_ms+5;
			pm_bak_time.is_backup_time_valid = 1;
			pm_bak_time.rc32k_freq = rc32k_freq;
			ret = nvram_config_set(CONFIG_PM_BACKUP_TIME_NVRAM_ITEM_NAME,
					&pm_bak_time, sizeof(struct sys_pm_backup_time));
			if (ret) {
				printk("failed to save pm backup time to nvram\n");
			} else {
				printk("power off current 8hz: %d, rc32k_freq=%d\n", pm_bak_time.counter8hz_cycles, rc32k_freq);
				print_rtc_time(&rtc_time);
			}
		}
	}

}
#endif

/*
**  system power off
*/
void sys_pm_poweroff(void)
{
	unsigned int key;
	/* wait 10ms, avoid trigger onoff wakeup pending */
	k_busy_wait(10000);
#ifdef CONFIG_ACTIONS_PRINTK_DMA
		printk_dma_switch(0);
#endif
	sys_pm_set_wakeup_src();

	printk("system power down!WKEN_CTL=0x%x\n", sys_read32(WKEN_CTL_SVCC));

	key = irq_lock();
#ifdef CONFIG_PM_DEVICE
	printk("dev power off\n");
	pm_power_off_devices();
	printk("dev power end\n");
#endif

#if (CONFIG_PM_BACKUP_TIME_FUNCTION_EN == 1)
	sys_pm_poweroff_backup_time();
#endif
	soc_pstore_set(SOC_PSTORE_TAG_HR_RESET, 0);/*poweroff set 0*/
	k_busy_wait(10);
	while(1) {
		sys_write32(0, POWER_CTL_SVCC);
		/* wait 10ms */
		k_busy_wait(10000);
		printk("poweroff fail, need reboot!\n");
		sys_pm_reboot(0);
	}

	/* never return... */
}

/*
CONFIG_WAKEUP_LONG_PRESS_TIME
*/
/*
 * The time threshold in millisecond to estimate the on-off key press is a long time pressed.
 *   - 0: 0.125s is a long pressed key.
 *   - 1: 0.25s is a long pressed key.
 *   - 2: 0.5s is a long pressed key.
 *   - 3: 1s is a long pressed key.
 *   - 4: 1.5s is a long pressed key.
 *   - 5: 2s is a long pressed key.
 *   - 6: 3s is a long pressed key.
 *   - 7: 4s is a long pressed key.
 */
#define CONFIG_WAKEUP_LONG_PRESS_TIME                (6)

void sys_pm_factory_poweroff(void)
{
	unsigned int key, val;
	/* wait 10ms, avoid trigger onoff wakeup pending */
	k_busy_wait(10000);
#ifdef CONFIG_ACTIONS_PRINTK_DMA
		printk_dma_switch(0);
#endif
	key = irq_lock();
	val = sys_read32(WKEN_CTL_SVCC)& (~0x1fff);
	val |= WAKE_CTL_LONG_WKEN | WAKE_CTL_WIO0LV_DETEN;
	if(!soc_pmu_get_dc5v_status()) {
		val |= WAKE_CTL_DC5VIN_WKEN;
	}
	sys_write32(val, WKEN_CTL_SVCC);
	k_busy_wait(500);

	soc_pmu_config_onoffkey_time(CONFIG_WAKEUP_LONG_PRESS_TIME);
	printk("factory power down WKEN_CTL=0x%x\n", sys_read32(WKEN_CTL_SVCC));

#ifdef CONFIG_PM_DEVICE
	printk("dev power off\n");
	pm_power_off_devices();
	printk("dev power end\n");
#endif

#if (CONFIG_PM_BACKUP_TIME_FUNCTION_EN == 1)
	sys_pm_poweroff_backup_time();
#endif
	soc_pstore_set(SOC_PSTORE_TAG_HR_RESET, 0);/*poweroff set 0*/
	k_busy_wait(10);
	while(1) {
		sys_write32(0, POWER_CTL_SVCC);
		/* wait 10ms */
		k_busy_wait(10000);
		printk("poweroff fail, need reboot!\n");
		sys_pm_reboot(0);
	}

	/* never return... */
}


void sys_pm_reboot(int type)
{
	unsigned int key;

#ifdef CONFIG_ACTIONS_PRINTK_DMA
	printk_dma_switch(0);
#endif
	if(type == REBOOT_TYPE_GOTO_SWJTAG){
		printk("set jtag flag\n");
		type = REBOOT_TYPE_NORMAL;
		sys_set_bit(RTC_REMAIN2, 0); //bit 0 adfu flag
	}
	printk("system reboot, type 0x%x!\n", type);

	key = irq_lock();
#ifdef CONFIG_PM_DEVICE
	printk("dev power off\n");
	pm_power_off_devices();
	printk("dev power end\n");
#endif
	soc_pstore_set(SOC_PSTORE_TAG_HR_RESET, 0);/*poweroff set 0*/
	/* store reboot reason in RTC_REMAIN0 for bootloader */
	sys_write32((REBOOT_REASON_MAGIC << 16) | (type & 0xffff), RTC_REMAIN3);
	k_busy_wait(500);
	sys_write32(0x5f, WD_CTL);
	while (1) {
		;
	}

	/* never return... */
}

int sys_pm_get_reboot_reason(u16_t *reboot_type, u8_t *reason)
{
	uint32_t reg_val;

	reg_val = soc_boot_get_reboot_reason();
	if ((reg_val >> 16) != REBOOT_REASON_MAGIC) {
		return -1;
	}

	*reboot_type = reg_val &  0xff00;
	*reason      =  reg_val & 0xff;

	return 0;
}

static void  ctk_close(void)
{
	int i;
	for(i = 0; i < 20; i++) // tpkey poweroff
	{
		sys_write32(0x40000009, CTK_CTL);
		sys_write32(0x4000000d, CTK_CTL);
	}
}

__sleepfunc void soc_udelay(uint32_t us)
{
	uint32_t cycles_per_1us, wait_cycles;
	volatile uint32_t i;
	uint8_t cpuclk_src = sys_read32(CMU_SYSCLK) & 0x7;

	if (cpuclk_src == 0)
		cycles_per_1us = 4;
	else if (cpuclk_src == 1)
		cycles_per_1us = 25;
	else if (cpuclk_src == 3)
		cycles_per_1us = 56; /* %15 deviation */
	else
		cycles_per_1us = 74;

	wait_cycles = cycles_per_1us * us / 10;

	for (i = 0; i < wait_cycles; i++) { /* totally 13 instruction cycles */
		;
	}
}

void   __psram_reg_write(unsigned int reg_addr, unsigned int reg_data);

__sleepfunc static void pm_vc18_init(void)
{
	sys_write32(0xB08E04C0,DCDC_VC18_CTL);// for bat low vc18  Voltage drop
}
static int soc_pm_init(const struct device *arg)
{
	unsigned int val = 0;
	soc_pstore_get(SOC_PSTORE_TAG_HR_RESET,&val);
	if(val){
		g_b_boot_abnormal = true;
	}else{
		g_b_boot_abnormal = false; /*boot normal*/
		soc_pstore_set(SOC_PSTORE_TAG_HR_RESET, 1);/*boot set 1, poweroff set 0*/
	}
	printk("WAKE_PD_SVCC=0x%x,wdt=%d, b_err=%d\n", sys_read32(WAKE_PD_SVCC), soc_boot_get_watchdog_is_reboot(), val);

	sys_write32(0x3, CMU_S1CLKCTL); // hosc+rc4m
	sys_write32(0x3, CMU_S1BTCLKCTL); // hosc+rc4m
	sys_write32(0x1, CMU_S2SCLKCTL); // RC4M enable
	sys_write32(0x9, CMU_S3CLKCTL); // S3 colse hosc and RC4M, enable RAM4 CLK SPIMT ICMT CLK ENABLE
	sys_write32(0x0, CMU_PMUWKUPCLKCTL); //select wk clk RC32K, if sel RC4M/4 ,must enable RC4M IN sleep
	sys_write32(0x1, CMU_GPIOCLKCTL); //select gpio clk RC4M
	pm_vc18_init();
	if (soc_dvfs_opt()) {
		sys_write32(0x06a02053, VOUT_CTL1_S3);//VDD=0.7, vdd1.2=0.7

		if(soc_boot_is_mini()){
			/* s1 vdd set as 1.1v */
			sys_write32((sys_read32(VOUT_CTL1_S1) & ~(0xF << 0)) | 0xb, VOUT_CTL1_S1);
			psram_delay_chain_set(0x8, 0x6);
		}else{
			/* s1 vdd set as 1.2v */
			sys_write32((sys_read32(VOUT_CTL1_S1) & ~(0xF << 0)) | 0xd, VOUT_CTL1_S1);
			/* set psram delay chain in 1.1v */
			psram_delay_chain_set(0xb, 0xb);
			__psram_reg_write(0, 0xb);
		}
			/* vd12 set as 1.2v */
		sys_write32((sys_read32(VOUT_CTL1_S1) & ~(0xff << 8)) | (0xa << 8) | (0xa << 12), VOUT_CTL1_S1);


	} else {
		sys_write32(0x06202053, VOUT_CTL1_S3);//VDD=0.7, vdd1.2=0.7

		/* s1 vdd set as 1.1v */
		sys_write32((sys_read32(VOUT_CTL1_S1) & ~(0xF << 0)) | 0xb, VOUT_CTL1_S1);
		/* set psram delay chain in 1.1v */
		psram_delay_chain_set(0x3, 0x6);

	}

	sys_write32(0x06a02255, VOUT_CTL1_S2);//VDD=0.8, vdd1.2=0.8
	sys_write32(0x5958, RC4M_CTL);

	sys_write32(sys_read32(BDG_CTL_SVCC) & ~(1 << 20), BDG_CTL_SVCC);

	/* disable DSPs power gating */
	sys_write32(sys_read32(PWRGATE_DIG) & ~(0x3 << 25), PWRGATE_DIG);

	/* spi1 cache enable low power mode
	 * TOTO: move to boot
	 */
	 #ifndef CONFIG_SOC_NO_PSRAM
	sys_write32((sys_read32(SPI1_CACHE_CTL) & ~(0x3 << 5)) | (1 << 5), SPI1_CACHE_CTL);
	 #endif

	ctk_close();
	return 0;
}


SYS_INIT(soc_pm_init, PRE_KERNEL_1, 20);

#ifndef CONFIG_SOC_WKEN_BAT
#ifdef CONFIG_ADC
#include <drivers/adc.h>
#include <sys/byteorder.h>

#define DC5V_CH 	2
#define BAT_CH 		1
static int dc5v_bat_get_mv(int ch)
{
	int ret;
	struct adc_sequence seq;
	uint8_t adc_buf[4];
	uint32_t adc_val, mv;
	const struct device *adc_dev;
	struct adc_channel_cfg ch_cfg = {0};

	adc_dev = device_get_binding(CONFIG_PMUADC_NAME);
	if (adc_dev == NULL) {
		printk("ADC device not found\n");
		return -ENODEV;
	}
	ch_cfg.channel_id = ch;
	adc_channel_setup(adc_dev, &ch_cfg);
	seq.channels = BIT(ch); //DC5V
	seq.buffer = &adc_buf[0];
	seq.buffer_size = sizeof(adc_buf);
	ret = adc_read(adc_dev, &seq);
	if(ret < 0){
		printk("adc read fail\n");
		return -EINVAL;
	}
	adc_val = sys_get_le16(seq.buffer);
	if(DC5V_CH == ch)
		mv = (adc_val + 1) * 6000 / 4096;  // (adc_val + 1) * 1465 / 1000;  ///1 = 1.465mv
    else
		mv = adc_val * 300 / 1024;  ///1 = 1.170mv ,bit0-1 is 0

	printk("ch%d: adc=%d, mv=%d\n",ch, adc_val, mv);
	return mv;

}

/*battery insert, system poweroff */
static int bat_poweron_init(const struct device *arg)
{
	unsigned int  wctl;
	int dc5mv, batmv;
	wctl = sys_read32(WKEN_CTL_SVCC);
	if(!(wctl & (3<<30))){// check first boot
		sys_write32(wctl | (3<<30) ,  WKEN_CTL_SVCC); /*first boot, set flag, for check battery first insert*/
		#ifdef CONFIG_ACTS_POWERPATH_CHARGER /*ext charger and power path management must define*/
		dc5mv = dc5v_bat_get_mv(DC5V_CH);
		batmv =  dc5v_bat_get_mv(BAT_CH);
		if((dc5mv > 0) && ( batmv > 0) && (batmv < dc5mv + 100)){
			printk("powerpath, battery first insert, poweroff\n");
			sys_pm_factory_poweroff();
		}
		#else
		unsigned int chg_ctl_bak;
		chg_ctl_bak = sys_read32(CHG_CTL_SVCC);
		sys_write32(chg_ctl_bak | BIT(19), CHG_CTL_SVCC);// SWITCH CV_3.3
		k_busy_wait(300);
		dc5mv = dc5v_bat_get_mv(DC5V_CH);
		batmv =  dc5v_bat_get_mv(BAT_CH);
		sys_write32(chg_ctl_bak, CHG_CTL_SVCC); //recovery
		k_busy_wait(300);
		if((dc5mv > 0) && ( batmv > 0) && (dc5mv < batmv + 100)){
			printk("int battery first insert, poweroff\n");
			sys_pm_factory_poweroff();
		}
		#endif
	}
	return 0;
}

SYS_INIT(bat_poweron_init, APPLICATION, 99);
#endif //CONFIG_ADC
#endif




