/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file soc_sleep.c  sleep  for Actions SoC
 */


#include <zephyr.h>
#include <board.h>
#include <arch/arm/aarch32/cortex_m/cmsis.h>
#include <string.h>
#include <drivers/timer/system_timer.h>
#include <linker/linker-defs.h>
#include <partition/partition.h>
#include "spicache.h"

#define HOSC_CTL_HGMC_SHIFT		8
#define HOSC_CTL_HGMC_MASK		(0x3 << HOSC_CTL_HGMC_SHIFT)

//#define CONFIG_SLEEP_TIMER_DEBUG

//#define CONFIG_SLEEP_STAGE_DEBUG

//#define CONFIG_SLEEP_DISABLE_BT

/*sleep mode*/
#define SLEEP_MODE_NORMALE 		0	 /*normal sleep*/
#define SLEEP_MODE_BT_OFF 		1    /*sleep bt power off*/
#define SLEEP_MODE_LOWPOWER 	2	 /*sleep bt power off, only onoff wakeup*/



#define SP_IN_SLEEP						(CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_SIZE * 1024 - 4)
#define SRAM_SAVE_BASE_ADDR				((uint32_t)(&__kernel_ram_start))
#define SRAM_SAVE_END_ADDR				((uint32_t)(&__kernel_ram_save_end))
#define SRAM_SAVE_LEN					(SRAM_SAVE_END_ADDR - SRAM_SAVE_BASE_ADDR)
#define SENSOR_CODE_MAX_LEN				(SP_IN_SLEEP - SRAM_SAVE_BASE_ADDR - 1024)   /*1024 is for sp*/

#define PSRAM_SAVE_MAXLEN				(0x10000)
#define MAX_NUM_STAAGE					(6)
#define MEMORY_CHECK_INTEGRITY_SIZE     (256)

#define IS_SYSTEM_RUN_ON_PSRAM          (!!(sys_read32(SPICACHE_CTL) & (1 << 15)))

struct sleep_wk_data {
	uint16_t wksrc;
	uint16_t wk_en_bit;
	const char *wksrc_msg;
};

static const struct sleep_wk_data wk_msg[] = {
	{SLEEP_WK_SRC_PMU, 		IRQ_ID_PMU,  	"PMU" },
	{SLEEP_WK_SRC_RTC,      IRQ_ID_RTC,     "RTC" },
	{SLEEP_WK_SRC_BT, 		IRQ_ID_BT,  	"BT" },
	{SLEEP_WK_SRC_GPIO, 	IRQ_ID_GPIO,    "GPIO" },
	{SLEEP_WK_SRC_T0,		IRQ_ID_TIMER0, 	"T0" },
	{SLEEP_WK_SRC_T1,		IRQ_ID_TIMER1,	"T1" },
	{SLEEP_WK_SRC_T2,		IRQ_ID_TIMER2,	"T2" },
	{SLEEP_WK_SRC_T3,		IRQ_ID_TIMER3,	"T3" },
	{SLEEP_WK_SRC_TWS,		IRQ_ID_TWS, 	"TWS" },
	{SLEEP_WK_SRC_SPI0MT,	IRQ_ID_SPI0MT,	"SPI0MT" },
	{SLEEP_WK_SRC_SPI1MT,	IRQ_ID_SPI1MT,	"SPI1MT" },
	{SLEEP_WK_SRC_IIC0MT,	IRQ_ID_IIC0MT,	"IIC0MT" },
	{SLEEP_WK_SRC_IIC1MT,	IRQ_ID_IIC1MT,	"IIC1MT" },
};

#define SLEEP_WKSRC_NUM ARRAY_SIZE(wk_msg)

struct sleep_wk_cb {
	sleep_wk_callback_t wk_cb;
	enum S_WK_SRC_TYPE src;
};

/* sleep context structure */
struct sleep_context_t {
	uint32_t sleep_mode; /*control sleep mode*/
	uint32_t g_sleep_cycle;
	enum S_WK_SRC_TYPE g_sleep_wksrc_src;

	struct sleep_wk_cb g_syste_cb[SLEEP_WKSRC_NUM];
	uint32_t g_num_cb;

#ifdef CONFIG_SLEEP_MEMORY_CHECK_INTEGRITY
	uint32_t check_start_addr;
	uint32_t check_len;
	uint32_t check_sum;
	uint32_t resume_sum;
#endif

#ifdef CONFIG_SLEEP_COPY_SRAM_FOR_SENSOR
	uint32_t psram_bak_sram[PSRAM_SAVE_MAXLEN / 4];
	void *sensor_code_addr;
	uint32_t sensor_code_len;
#endif

#ifdef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
	uint32_t system_part_offset;
	uint32_t sysmap_entry_addr;
#endif
};

static struct sleep_context_t sleep_context_obj;
#define current_sleep (&sleep_context_obj)

static struct sleep_wk_fun_data __act_s2_sleep_data *g_wk_fun[SLEEP_WKSRC_NUM] ;

static volatile uint16_t __act_s2_sleep_data g_sleep_wksrc_en;
static int64_t __act_s2_sleep_data g_sleep_update_time, g_sleep_ms;
static uint32_t __act_s2_sleep_data g_sleep_st, check_cnt, g_sleep_t2cnt;

/* Save and restore the registers */
static const uint32_t backup_regs_addr[] = {
	CMU_S1CLKCTL,
	PMUADC_CTL,
	CMU_MEMCLKEN0,
	CMU_MEMCLKEN1,
	NVIC_ISER0,
	NVIC_ISER1,
	PWRGATE_DIG,
	CMU_SPI0CLK,
};

static uint32_t s2_reg_backups[ARRAY_SIZE(backup_regs_addr)];

/*gpio36-48, GPIO0-3 ,6,7 handle in sleep fun*/
/*gpio not use int sleep*/
static const uint32_t backup_regs_gpio[] = {
	/*misc */
	//GPION_CTL(4), /*not use defaut highz*/
	//GPION_CTL(22), /*not use defaut highz*/
	//GPION_CTL(23), /*not use defaut highz*/
	//GPION_CTL(31), /*not use defaut highz*/

#if CONFIG_SPINAND_3 || CONFIG_SPI_FLASH_2
	/*spinand */
	GPION_CTL(8),
	GPION_CTL(9),
	GPION_CTL(10),
	GPION_CTL(11),
	GPION_CTL(12),
	GPION_CTL(13),
#endif
	//GPION_CTL(64), /*power enable, use in sleep*/
	/*LCD */
	//GPION_CTL(5), /*lcd backlight enable*/
	//GPION_CTL(14),
	//GPION_CTL(15),
	//GPION_CTL(16),
	//GPION_CTL(17),
	//GPION_CTL(30),
	//GPION_CTL(34),
	//GPION_CTL(35),
	//GPION_CTL(33), /*lcd reset use in sleep*/

#ifndef CONFIG_SENSOR_MANAGER
	GPION_CTL(49),
	GPION_CTL(50),
#endif

#if 0
	/*sensor*/
	//GPION_CTL(18), /*not use defaut highz*/
	GPION_CTL(19), /*EN_NTC. user in sleep*/
	//GPION_CTL(20), /*not use defaut highz*/
	GPION_CTL(21), /*sensor irq ,use in sleep*/
	//GPION_CTL(24), /* HR_PWR_EN ,use in sleep*/
	GPION_CTL(25), /*VDD1.8 eanble ,use in sleep*/
	GPION_CTL(33), /*GPS wake up Host ,use in sleep*/
	/*TP*/
	//GPION_CTL(26), /*not use defaut highz*/
	//GPION_CTL(27), /*not use，defaut highz*/
	GPION_CTL(32), /*tp irq*/
	/*debug uart0 */
	//GPION_CTL(28), /*use in sleep*/
	//GPION_CTL(29), /*use in sleep*/
	/*I2CMT0*/
	//GPION_CTL(49), /*use in sleep*/
	//GPION_CTL(50), /*use in sleep*/
	/*I2CMT1*/
	GPION_CTL(51), /*use in sleep*/
	GPION_CTL(52), /*use in sleep*/

	//GPION_CTL(53), /*not use defaut highz*/
	//GPION_CTL(54), /*not use defaut highz*/
	//GPION_CTL(55), /*not use defaut highz*/
	//GPION_CTL(56), /*not use defaut highz*/
	/*i2c0*/
	GPION_CTL(57), /*not use in sleep*/
	GPION_CTL(58), /*not use in sleep*/
	/*i2c1*/
	GPION_CTL(59), /*not use in sleep*/
	GPION_CTL(60), /*not use in sleep*/

	//GPION_CTL(61), /*sensor HR_EINT irq , use in sleep*/
	GPION_CTL(62), /*sensor GPS_PPS , not use in sleep*/
	GPION_CTL(63), /*sensor irq , not use in sleep*/
#endif

#ifdef SLEEP_GPIO_REG_SET_HIGHZ
	SLEEP_GPIO_REG_SET_HIGHZ
#endif
};

#ifdef SLEEP_AOD_GPIO_REG_UNSET_HIGHZ
static const uint32_t backup_regs_aod_gpio[] = {
	SLEEP_AOD_GPIO_REG_UNSET_HIGHZ
};
#endif

static uint32_t s2_gpio_reg_backups[ARRAY_SIZE(backup_regs_gpio)];

#ifdef CONFIG_SPI_NOR_FLASH_4B_ADDRESS
extern void sys_norflash_exit_4b(void);
extern void sys_norflash_enter_4b(void);
#endif

#if defined(CONFIG_BOARD_NANDBOOT)|| defined(CONFIG_BOARD_EMMCBOOT) || !defined(CONFIG_SPI_FLASH_ACTS)
void  sys_norflash_power_ctrl(uint32_t is_powerdown)
{

}
#else
extern void sys_norflash_power_ctrl(uint32_t is_powerdown);
#endif

/*g_cyc2ms_mul = (1000<<16) / soc_rc32K_freq() / */
static uint32_t __act_s2_sleep_data g_cyc2ms_mul;  //

static __sleepfunc uint32_t rc32k_cyc_to_ms(uint32_t cycle)
{
	//return (uint32_t)((uint64_t)cycle * 1000 / soc_rc32K_freq());
	uint64_t tmp = g_cyc2ms_mul;
	tmp = tmp * cycle;
	return (tmp >> 16);
}

#ifdef CONFIG_SLEEP_TIMER_DEBUG
__act_s2_sleep_data uint32_t g_save_stage_t[MAX_NUM_STAAGE];

__sleepfunc void sleep_stage(int index)
{
	if(index < MAX_NUM_STAAGE)
		g_save_stage_t[index] = sys_read32(T2_CNT);
}

__sleepfunc int uart_out_ch(int c, void *ctx)
{
	ARG_UNUSED(ctx);

	/* Wait for transmitter to be ready */
	while (sys_read32(UART0_REG_BASE + 12) &  BIT(6));

	/* send a character */
	if(c == '\n')
		sys_write32('\r', UART0_REG_BASE + 8);

	sys_write32(c, UART0_REG_BASE+8);

	return 0;
}

__sleepfunc void uart_out_flush(void)
{
	/* Wait for transmitter complete */
	while (sys_read32(UART0_REG_BASE + 12) &  BIT(21));
}

__sleepfunc int sl_printk(const char *fmt, ...)
{
	va_list args;

	uart_out_flush();

	va_start(args, fmt);
	pbrom_libc_api->p_cbvprintf(uart_out_ch, NULL, fmt, args);
	va_end(args);

	uart_out_flush();

	return 0;
}

void dump_stage_timer(uint32_t st, uint32_t end)
{
	uint32_t t,i;
	t = (end-st);
	printk("sleep use %d ms\n", rc32k_cyc_to_ms(t));
	for(i = 0; i< MAX_NUM_STAAGE; i++){
		t = g_save_stage_t[i] - st;
		if(t > 320){
			printk("%d use %d ms\n",i, rc32k_cyc_to_ms(t));
		}else{
			printk("%d use %d us\n",i, rc32k_cyc_to_ms(t) * 1000);
		}
		st = g_save_stage_t[i];
	}
}

#else
#ifdef CONFIG_SLEEP_STAGE_DEBUG
__sleepfunc void sleep_stage(unsigned int index)
{
	 unsigned int val,stage;
	 stage = (index&0x7f)+0x80;
	 val = sys_read32(RTC_REMAIN2)& (~(0xff<<8));
	 sys_write32( val | (stage << 8), RTC_REMAIN2);
}

#else
void sleep_stage(int index)
{

}
#endif

__sleepfunc int sl_printk(const char *fmt, ...)
{
	ARG_UNUSED(fmt);
	return 0;
}

void dump_stage_timer(uint32_t st, uint32_t end)
{
	printk("sleep use %d ms, check_cnt=%d\n", rc32k_cyc_to_ms(end - st), check_cnt);
}

#endif /* CONFIG_SLEEP_TIMER_DEBUG */

#ifdef CONFIG_SLEEP_MEMORY_CHECK_INTEGRITY
static uint32_t check_sum(uint32_t *buf, int len)
{
	int i;
	uint32_t chk = 0;

	for (i = 0; i < (len / 4); i++)
		chk += buf[i];

	return chk;
}

static void suspend_check(void)
{
	uint32_t chk;
	chk = check_sum((uint32_t *)current_sleep->check_start_addr,
					current_sleep->check_len);
	current_sleep->check_sum = chk;

	//printk("suspend checksum:0x%x\n", chk);
}

static void resume_check(void)
{
	uint32_t chk;
	chk = check_sum((uint32_t *)current_sleep->check_start_addr,
					current_sleep->check_len);
	current_sleep->resume_sum = chk;

}

static void wakeup_check_dump(void)
{
	//uint32_t chk;

	//chk = check_sum((uint32_t *)current_sleep->check_start_addr,
					//current_sleep->check_len);
	printk("checksum resume:0x%x suspend:0x%x,len=0x%x\n", current_sleep->resume_sum, current_sleep->check_sum, current_sleep->check_len);

	if (current_sleep->check_sum != current_sleep->resume_sum){
		printk("\n----- error memory integrity check -----------\n");
		//while(1);
	}

}
#endif

#ifdef CONFIG_SLEEP_COPY_SRAM_FOR_SENSOR
int sleep_sensor_code_set(void *code_addr, uint32_t code_len)
{
	printk("sensor code:base=0x%x, maxlen=0x%x, code_len=0x%x\n",
			SRAM_SAVE_BASE_ADDR, SENSOR_CODE_MAX_LEN,  code_len);

	if((code_addr == NULL) || (code_len > SENSOR_CODE_MAX_LEN)){
		printk("sensor code invalid\n");
		return -1;
	}

	current_sleep->sensor_code_addr = code_addr;
	current_sleep->sensor_code_len = code_len;

	return 0;
}

static void mem_sram_save_to_psram(void)
{
	memcpy(current_sleep->psram_bak_sram,
			(void*)SRAM_SAVE_BASE_ADDR, SRAM_SAVE_LEN);



	if ((current_sleep->sensor_code_addr != NULL) && current_sleep->sensor_code_len){
		//printk("copy sensor code 0x%x to sram\n", sensor_code_len);
		memcpy((void*)SRAM_SAVE_BASE_ADDR, current_sleep->sensor_code_addr,
				current_sleep->sensor_code_len);
	}
}

static void mem_psram_recovery_tosram(void)
{
	memcpy((void*)SRAM_SAVE_BASE_ADDR, current_sleep->psram_bak_sram, SRAM_SAVE_LEN);
}

#endif /* CONFIG_SLEEP_COPY_SRAM_FOR_SENSOR */

int sleep_register_wk_callback(enum S_WK_SRC_TYPE wk_src, struct sleep_wk_fun_data *fn_data)
{
	if (fn_data == NULL)
		return -1;

	fn_data->next = g_wk_fun[wk_src];
	g_wk_fun[wk_src] = fn_data;

	return 0;
}

void sys_s3_wksrc_set(enum S_WK_SRC_TYPE src)
{
	g_sleep_wksrc_en |= (1 << src);
}

void sys_s3_wksrc_clr(enum S_WK_SRC_TYPE src)
{
	g_sleep_wksrc_en &= ~(1 << src);
}

enum S_WK_SRC_TYPE sys_s3_wksrc_get(void)
{
	return current_sleep->g_sleep_wksrc_src;
}

void sys_s3_wksrc_init(void)
{
	current_sleep->g_sleep_wksrc_src = SLEEP_WK_SRC_T1;
}

static enum S_WK_SRC_TYPE sys_sleep_check_wksrc(void)
{
	int i;
	uint32_t wk_pd0, wk_pd1,wkbit;

	current_sleep->g_sleep_wksrc_src = 0;

	wk_pd0 = sys_read32(NVIC_ISPR0);
	wk_pd1 = sys_read32(NVIC_ISPR1);

	//printk("WK NVIC_ISPR0=0x%x\n", wk_pd0);
	//printk("WK NVIC_ISPR1=0x%x\n", wk_pd1);

	for (i = 0; i < SLEEP_WKSRC_NUM; i++) {
		if ((1 << wk_msg[i].wksrc) & g_sleep_wksrc_en) {
			wkbit = wk_msg[i].wk_en_bit;
			if (wkbit >= 32) {
				wkbit -= 32;
				if (wk_pd1 & (1 << wkbit))
					break;
			} else {
				if (wk_pd0 & (1 << wkbit))
					break;
			}
		}
	}

	if (i != SLEEP_WKSRC_NUM) {
		current_sleep->g_sleep_wksrc_src = wk_msg[i].wksrc;
		printk("wksrc = %s\n", wk_msg[i].wksrc_msg);
	} else {
		printk("no wksrc\n");
		current_sleep->g_sleep_wksrc_src = 0;
	}

	return current_sleep->g_sleep_wksrc_src;
}

static void sys_set_wksrc_before_sleep(void)
{
	int i;
	uint32_t wk_en0, wk_en1;

	if(current_sleep->sleep_mode !=SLEEP_MODE_NORMALE){
		g_sleep_wksrc_en = 1<<SLEEP_WK_SRC_PMU;
		if(current_sleep->sleep_mode == SLEEP_MODE_BT_OFF)
			g_sleep_wksrc_en |= 1 << SLEEP_WK_SRC_IIC0MT;
	}
#if 0
	printk("NVIC_ISPR0-1=0x%x,0x%x\n", sys_read32(NVIC_ISPR0), sys_read32(NVIC_ISPR1));
	printk("NVIC_ISER0-1=0x%x,0x%x\n", sys_read32(NVIC_ISER0), sys_read32(NVIC_ISER1));
	printk("NVIC_IABR0-1=0x%x,0x%x\n", sys_read32(NVIC_IABR0), sys_read32(NVIC_IABR1));
	printk("g_sleep_wksrc_en =0x%x\n", g_sleep_wksrc_en);
#endif

	sys_write32(sys_read32(NVIC_ISER0), NVIC_ICER0);
	sys_write32(sys_read32(NVIC_ISER1), NVIC_ICER1);
	//sys_write32(0xffffffff, NVIC_ICPR0);
	//sys_write32(0xffffffff, NVIC_ICPR1);

	if (g_sleep_wksrc_en) {

		wk_en0 = wk_en1 = 0;

		for (i = 0; i < SLEEP_WKSRC_NUM; i++) {
			if ((1 << wk_msg[i].wksrc) & g_sleep_wksrc_en){

				printk("%d wksrc=%s \n",i, wk_msg[i].wksrc_msg);

				if (wk_msg[i].wk_en_bit >= 32) {
					wk_en1 |=  1 << (wk_msg[i].wk_en_bit - 32);
				} else {
					wk_en0 |=  1 << (wk_msg[i].wk_en_bit);
				}
			}
		}

		if(wk_en0)
			sys_write32(wk_en0, NVIC_ISER0);

		if(wk_en1)
			sys_write32(wk_en1, NVIC_ISER1);
	}

#if 0
	printk("NVIC_ISPR0-1=0x%x,0x%x\n", sys_read32(NVIC_ISPR0), sys_read32(NVIC_ISPR1));
	printk("NVIC_ISER0-1=0x%x,0x%x\n", sys_read32(NVIC_ISER0), sys_read32(NVIC_ISER1));
	printk("NVIC_IABR0-1=0x%x,0x%x\n", sys_read32(NVIC_IABR0), sys_read32(NVIC_IABR1));
#endif
}

static uint32_t s2_reg_backups[ARRAY_SIZE(backup_regs_addr)];
static uint32_t s2_gpio_reg_backups[ARRAY_SIZE(backup_regs_gpio)];
#ifdef SLEEP_AOD_GPIO_REG_UNSET_HIGHZ
static uint32_t s2_aod_gpio_reg_backups[ARRAY_SIZE(backup_regs_aod_gpio)];
#endif

static void sys_pm_backup_registers(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(backup_regs_gpio); i++){ /* set gpio highz */
		s2_gpio_reg_backups[i] = sys_read32(backup_regs_gpio[i]);
		sys_write32(0x1000, backup_regs_gpio[i]);
	}

#ifdef SLEEP_AOD_GPIO_REG_UNSET_HIGHZ
	if (soc_get_aod_mode() == 0) {
		for (i = 0; i < ARRAY_SIZE(backup_regs_aod_gpio); i++) { // set gpio highz
			s2_aod_gpio_reg_backups[i] = sys_read32(backup_regs_aod_gpio[i]);
			sys_write32(0x1000, backup_regs_aod_gpio[i]);
		}
	}
#endif

	for (i = 0; i < ARRAY_SIZE(backup_regs_addr); i++)
		s2_reg_backups[i] = sys_read32(backup_regs_addr[i]);
}

static void sys_pm_restore_registers(void)
{

	int i;
	for (i = ARRAY_SIZE(backup_regs_gpio) - 1; i >= 0; i--)
		sys_write32(s2_gpio_reg_backups[i], backup_regs_gpio[i]);

#ifdef SLEEP_AOD_GPIO_REG_UNSET_HIGHZ
	if (soc_get_aod_mode() == 0) {
		for (i = ARRAY_SIZE(backup_regs_aod_gpio) - 1; i >= 0; i--) {
			sys_write32(s2_aod_gpio_reg_backups[i], backup_regs_aod_gpio[i]);
		}
	}
#endif

	for (i = ARRAY_SIZE(backup_regs_addr) - 1; i >= 0; i--)
		sys_write32(s2_reg_backups[i], backup_regs_addr[i]);
}

static enum WK_CB_RC wakeup_system_callback(void)
{
	int i;
	enum WK_CB_RC rc;

	if (current_sleep->g_num_cb) {
		rc = WK_CB_SLEEP_AGAIN;

		printk("wake call fun=%d\n", current_sleep->g_num_cb);

		for (i = 0; i < current_sleep->g_num_cb; i++){

			printk("call wksrc=%d fun\n", current_sleep->g_syste_cb[i].src);

			if (current_sleep->g_syste_cb[i].wk_cb(current_sleep->g_syste_cb[i].src)
				== WK_CB_RUN_SYSTEM)  /* need run system */
				rc = WK_CB_RUN_SYSTEM;
		}

		current_sleep->g_num_cb = 0;
	} else {
		rc = WK_CB_RUN_SYSTEM;
	}

	return rc;
}

static void soc_pmu_onoff_wk_set(void)
{
	//sys_write32(0x3, WIO0_CTL);
	sys_write32(sys_read32(PMU_INTMASK) | (1 << 1), PMU_INTMASK); /* ONOFF SHORT WAKEUP */
	sys_s3_wksrc_set(SLEEP_WK_SRC_PMU);
	//printk("PMUINTMASK=0X%X\n", sys_read32(PMU_INTMASK));
}

//#define CONFIG_GPIO_WAKEUP_TEST
#ifdef CONFIG_GPIO_WAKEUP_TEST
#define GPIO_N_WK  GPIO_21
//#define WIO_N_WK	WIO_1
static void soc_gpio_wakeup_test(void)
{
#ifdef GPIO_N_WK
	printk("gpio=%d wakeup test \n", GPIO_N_WK);
	sys_write32(GPIO_CTL_GPIO_INEN|GPIO_CTL_SMIT|GPIO_CTL_PULLUP| GPIO_CTL_INTC_EN | GPIO_CTL_INTC_MASK |
					GPIO_CTL_INC_TRIGGER_RISING_EDGE|GPIO_CTL_PADDRV_LEVEL(3), GPION_CTL(GPIO_N_WK));

#else
	printk("wio =%d wakeup test \n", WIO_N_WK);
	sys_write32(GPIO_CTL_GPIO_INEN|GPIO_CTL_SMIT|GPIO_CTL_PULLUP| GPIO_CTL_INTC_EN | GPIO_CTL_INTC_MASK |
					GPIO_CTL_INC_TRIGGER_RISING_EDGE|GPIO_CTL_PADDRV_LEVEL(3), WIO_REG_CTL(WIO_N_WK));
#endif
	sys_s3_wksrc_set(SLEEP_WK_SRC_GPIO);
}
static void soc_gpio_check(void)
{
#ifdef GPIO_N_WK
	printk("*******gpio=%d check----\n", GPIO_N_WK);
	if(sys_read32(GPIO_REG_IRQ_PD(GPIO_REG_BASE, GPIO_N_WK)) & GPIO_BIT(GPIO_N_WK)){
		printk("*******gpio=%d wakeup----\n", GPIO_N_WK);
		sys_write32(GPIO_BIT(GPIO_N_WK), GPIO_REG_IRQ_PD(GPIO_REG_BASE, GPIO_N_WK));
	}
#else
	printk("*******wigo=%d check----\n", WIO_N_WK);
	if(sys_read32(WIO_REG_CTL(WIO_N_WK)) & WIO_CTL_INT_PD_MASK){
		printk("*******wio=%d wakeup----\n", WIO_N_WK);
		sys_write32(WIO_REG_CTL(WIO_N_WK), sys_read32(WIO_REG_CTL(WIO_N_WK)));
	}
#endif
}
#endif

static enum WK_CB_RC soc_cmu_sleep_exit(void)
{
	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
#ifdef CONFIG_GPIO_WAKEUP_TEST
	soc_gpio_check();
#endif

	sys_pm_restore_registers();

	sys_write32(0x1, CMU_GPIOCLKCTL); /* select gpio clk RC4M */

	sys_sleep_check_wksrc();

#ifdef CONFIG_SLEEP_MEMORY_CHECK_INTEGRITY
	wakeup_check_dump();
#endif

	return wakeup_system_callback();
}

static void soc_cmu_sleep_prepare(int isdeep)
{
	sys_pm_backup_registers();  /* backup reg */

	sys_s3_wksrc_set(SLEEP_WK_SRC_BT); /* set bt wakeup src */

	soc_pmu_onoff_wk_set(); /* set onoff wakeup src */
#ifdef CONFIG_GPIO_WAKEUP_TEST
	soc_gpio_wakeup_test();
#endif
	/**
	 * RMU_MRCR0=0x1b0d5fb1
	 * RMU_MRCR1=0xc10c000c
	 * CMU_DEVCLKEN0=0x5b0413b1
	 * CMU_DEVCLKEN1=0x1f0c000c bit24-28 is bt
	 */
	sys_write32(0x0, PMUADC_CTL);// close ADC

	/* PWRGATE_DIG  bit20-aduio  bit21-de/se/usb bit25-adncdsp bit26-audiodsp bit27-bt-f bit28-btp */
	if (isdeep) {
		sys_write32(0x90000000, PWRGATE_DIG); /* m4f/bt on   else off */
	} else {
		sys_write32(sys_read32(RMU_MRCR1) & ~(1 << 24), RMU_MRCR1); /* disable bluetooth hub */
		sys_write32(0x88000000, PWRGATE_DIG); /* m4f on, bt off  else off */
	}

	/*spi0 clk switch to hosc*/
	sys_write32(0x0, CMU_SPI0CLK);
	sys_write32(0x0, CMU_GPIOCLKCTL); /* select gpio clk RC32K */

	sys_set_wksrc_before_sleep(); /* init wakeup src */

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk ;  /* deepsleep */

#ifdef CONFIG_SLEEP_COPY_SRAM_FOR_SENSOR
	printk("sensor code=0x%x save addr=0x%x len=0x%x\n\n",
			current_sleep->sensor_code_len,
			SRAM_SAVE_BASE_ADDR, SRAM_SAVE_LEN);
#endif
}

#ifdef CONFIG_DISABLE_IRQ_STAT
static inline  unsigned int n_irq_lock(void)
{
	unsigned int key;
	unsigned int tmp;
	__asm__ volatile(
		"mov %1, %2;"
		"mrs %0, BASEPRI;"
		"msr BASEPRI, %1;"
		"isb;"
		: "=r"(key), "=r"(tmp)
		: "i"(_EXC_IRQ_DEFAULT_PRIO)
		: "memory");

	return key;
}

static inline  void n_irq_unlock(unsigned int key)
{
	__asm__ volatile(
		"msr BASEPRI, %0;"
		"isb;"
		:  : "r"(key) : "memory");
}
#endif

__sleepfunc static void __cpu_enter_sleep(void)
{
	uint32_t devclk[2];
	uint32_t val;

	//jtag_enable();
	/*spi0  cache disable*/
	//sys_clear_bit(SPICACHE_CTL, 0);

	devclk[0] = sys_read32(CMU_DEVCLKEN0);
	devclk[1] = sys_read32(CMU_DEVCLKEN1);

	sys_write32((1 << CLOCK_ID_TIMER) | (1<<CLOCK_ID_EXINT), CMU_DEVCLKEN0);
	sys_write32(0x1f000000| 1 << (CLOCK_ID_I2CMT0-32), CMU_DEVCLKEN1);

	/* RC64M enable for spi0 clk switch to rc64m*/
	sys_set_bit(CMU_S1CLKCTL,2);
	/*cpu clk select rc4M*/
	sys_write32(0x0, CMU_SYSCLK);
	/* NOR use RC64M */
	sys_write32(0x301, CMU_SPI0CLK);
	
	val = sys_read32(VOUT_CTL1_S1);	
	sys_write32((val & ~(0xf)) | 0x8, VOUT_CTL1_S1); //0.95

	soc_udelay(3);/*delay for cpuclk&spi0 clk switch ok*/
	/* RC64M + RC4M enable，hosc disable */
	sys_write32(0x5, CMU_S1CLKCTL);

#ifdef CONFIG_SLEEP_TIMER_DEBUG
	/* hosc enable for uart */
	sys_set_bit(CMU_S1CLKCTL, 1);
	sys_set_bit(CMU_S3CLKCTL, 1);
#endif

	/* enter sleep */
	__asm__ volatile("cpsid	i");
#ifdef CONFIG_DISABLE_IRQ_STAT
	n_irq_unlock(0);
#else
	irq_unlock(0);
#endif
	__asm__ volatile("dsb");
	__asm__ volatile("wfi");
#ifdef CONFIG_DISABLE_IRQ_STAT
	n_irq_lock();
#else
    irq_lock();
#endif
	__asm__ volatile("cpsie	i");

	sys_write32(val, VOUT_CTL1_S1);
	soc_udelay(30);/*delay for vdd*/

	/* CPU USE RC64M */
	sys_write32(0x03, CMU_SYSCLK);

	sys_write32(devclk[0], CMU_DEVCLKEN0);
	sys_write32(devclk[1], CMU_DEVCLKEN1);

	if(sys_test_bit(WD_CTL, 4))/*if wdt enable, feed wdt*/
		sys_set_bit(WD_CTL, 0);

	g_sleep_t2cnt = sys_read32(T2_CNT);/*T2 counter before clock sync*/

	/* spi0 cache enable */
//	sys_set_bit(SPICACHE_CTL, 0);

	/* spi0 cache invalid */
//	sys_write32(1, SPICACHE_INVALIDATE);
//	while ((sys_read32(SPICACHE_INVALIDATE) & 0x1) == 1) {}
}


__sleepfunc static enum WK_CB_RC check_wk_run_sram_nor(uint16_t *wk_en_bit,
				struct sleep_wk_cb *cb, int *cb_num)
{
	int i;
	enum WK_CB_RC rc = WK_CB_SLEEP_AGAIN;
	enum WK_CB_RC rc_t ;
	enum WK_RUN_TYPE runt;
	uint32_t wk_pd0, wk_pd1,wkbit;
	bool b_nor_wk = false;
	struct sleep_wk_fun_data *p;
	wk_pd0 = sys_read32(NVIC_ISPR0);
	wk_pd1 = sys_read32(NVIC_ISPR1);
	*cb_num = 0;

	for(i = 0; i < SLEEP_WKSRC_NUM; i++){
		if(!((1 << i) & g_sleep_wksrc_en))
			continue;

		wkbit = wk_en_bit[i];
		if(wkbit >= 32){
			if(!(wk_pd1 & (1<<(wkbit - 32))))
				continue;

		}else{
			if(!(wk_pd0 & (1<<wkbit)))
				continue;
		}
		if(g_wk_fun[i]){
			p = g_wk_fun[i];
			do{
				if(p->wk_prep){
					runt = p->wk_prep(i);//
					if(runt == WK_RUN_IN_SRAM){
						rc_t = p->wk_cb(i);
						//if(WK_CB_RUN_SYSTEM == p->wk_cb(i))
							//rc = WK_CB_RUN_SYSTEM;
					}else if (runt == WK_RUN_IN_NOR) {
						if(!b_nor_wk){
							b_nor_wk = true;
							sys_norflash_power_ctrl(0);
						}
						rc_t = p->wk_cb(i);
						//if(WK_CB_RUN_SYSTEM == p->wk_cb(i))
							//rc = WK_CB_RUN_SYSTEM;

					}else{
						rc_t = WK_CB_RUN_SYSTEM;
						if(*cb_num < SLEEP_WKSRC_NUM){
							cb[*cb_num].wk_cb = p->wk_cb;
							cb[*cb_num].src = i;
							(*cb_num)++;
						}
					}
					if(rc_t == WK_CB_SLEEP_AGAIN){
						if(wkbit < 32)
							sys_write32(1<<wkbit, NVIC_ICPR0);
						else
							sys_write32(1<<(wkbit-32), NVIC_ICPR1);
					}

				}else{
					rc_t = WK_CB_RUN_SYSTEM;
					if(*cb_num < SLEEP_WKSRC_NUM){
						cb[*cb_num].wk_cb = p->wk_cb;
						cb[*cb_num].src = i;
						(*cb_num)++;
					}
				}
				p = p->next;
				if(rc_t == WK_CB_RUN_SYSTEM)// if wksrc need wake up system ,wakeup sysytem
					rc = WK_CB_RUN_SYSTEM;
			}while(p);

			if(rc_t == WK_CB_CARELESS) /*if  all callback not care of this wksrc, wakeup system handle*/
					rc = WK_CB_RUN_SYSTEM;

		}else{
			rc = WK_CB_RUN_SYSTEM; /*not wake up callback , system handle*/
		}

	}

	if(rc == WK_CB_SLEEP_AGAIN){
		//sys_write32(0xffffffff, NVIC_ICPR0);
		//sys_write32(0xffffffff, NVIC_ICPR1);
		if(b_nor_wk){
			sys_norflash_power_ctrl(1);
		}
	}else{
		if(!b_nor_wk){
			sys_norflash_power_ctrl(0);
		}
	}
	return rc;
}

#ifdef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
__sleepfunc void spi0_cache_switch_mapping(bool is_psram)
{
	volatile uint16_t loop;
	uint32_t bk_spicache_ctl = sys_read32(SPICACHE_CTL);

  sys_write32(1, SPICACHE_INVALIDATE);

  while ((sys_read32(SPICACHE_INVALIDATE) & 0x1) == 1) {}

	sys_write32(0, SPICACHE_CTL);

	sys_write32(sys_read32(RMU_MRCR0) & ~(1 << 8), RMU_MRCR0);
	loop=10;
	while(loop)loop--;
	sys_write32(sys_read32(RMU_MRCR0) | (1 << 8), RMU_MRCR0);

	if (is_psram) {
		bk_spicache_ctl |= (1 << 15);
		sys_write32(current_sleep->sysmap_entry_addr, SPI_CACHE_ADDR0_ENTRY);
	} else {
		bk_spicache_ctl &= ~(1 << 15);
		sys_write32(current_sleep->system_part_offset, SPI_CACHE_ADDR0_ENTRY);
	}

	sys_write32(bk_spicache_ctl, SPICACHE_CTL);

    sys_write32(1, SPICACHE_INVALIDATE);

    while ((sys_read32(SPICACHE_INVALIDATE) & 0x1) == 1)
        ;
}
#endif

#ifndef CONFIG_SOC_NO_PSRAM
static void soc_psram_flush(void)
{
	spi1_cache_ops(SPI_WRITEBUF_FLUSH, (void *)SPI1_BASE_ADDR, 0x1000);
	spi1_cache_ops(SPI_CACHE_FLUSH_ALL, (void *)SPI1_BASE_ADDR, 0x1000);
	spi1_cache_ops_wait_finshed();
}
#endif

__sleepfunc static void cpu_enter_sleep_tmp(void)
{
	uint32_t corepll_backup, sysclk_bak;
	unsigned int i, num_cb;
	uint16_t wk_en_bit[SLEEP_WKSRC_NUM];
	struct sleep_wk_cb cb[SLEEP_WKSRC_NUM];
	uint32_t bk_clksrc0, bk_clksrc1, bk_vout_s1, val;

#ifdef CONFIG_SLEEP_COPY_SRAM_FOR_SENSOR
	mem_sram_save_to_psram();
#endif

#ifdef CONFIG_SLEEP_MEMORY_CHECK_INTEGRITY
	suspend_check();
#endif
	/* flush spi1 cache */
#ifndef CONFIG_SOC_NO_PSRAM
	soc_psram_flush();
#endif

	for(i = 0; i < SLEEP_WKSRC_NUM; i++) // copy nor to sram, nor is not use in sleep func
		wk_en_bit[i] = wk_msg[i].wk_en_bit;

	/* In case of  sensor run in PSRAM, switch SPI0 mapping to SPINOR */
#ifdef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
	bool is_sensor_run_psram = IS_SYSTEM_RUN_ON_PSRAM;
	if (is_sensor_run_psram) {
#ifdef CONFIG_SPI_NOR_FLASH_4B_ADDRESS
		sys_norflash_exit_4b();
#endif
		spi0_cache_switch_mapping(0);
	}
#else
	uint32_t spi1_clk, ddr_mode;
	sys_set_bit(CMU_S1CLKCTL,2);	// enable rc64m for psram
	spi1_clk = 	sys_read32(CMU_SPI1CLK); // bak spi1 clk
	ddr_mode =  sys_read32(SPI1_DDR_MODE_CTL);
	if(!soc_boot_is_mini())
		sys_write32((ddr_mode & ~(0xf << 16)) | (4<<16), SPI1_DDR_MODE_CTL);  // spi1 clk 64MHZ,must adjust RLC to 4 ( default is 5)
	sys_write32(0x300, CMU_SPI1CLK); // psram now use rc64m
#endif /* CONFIG_SLEEP_SENSOR_RUN_ON_NOR */

	/*nor enter deep power down */
	sys_norflash_power_ctrl(1);

	sleep_stage(0);

	/* psram enter low self refresh mode */
	psram_self_refresh_control(true);

	/* psram enter low power mode */
	psram_power_control(true);

	sysclk_bak =  sys_read32(CMU_SYSCLK);

	/*first switch cpu clk source (hosc)*/
	sys_write32((sysclk_bak&(~0x7)) | 0x1, CMU_SYSCLK);
	soc_udelay(1);
	/*cpu clk select HOSC*/
	sys_write32(0x1, CMU_SYSCLK);

	corepll_backup = sys_read32(COREPLL_CTL);
	bk_vout_s1 = sys_read32(VOUT_CTL1_S1);

	/* disable COREPLL */
	sys_write32(sys_read32(COREPLL_CTL) & ~(1 << 7), COREPLL_CTL);
	sys_write32(0, COREPLL_CTL);

	/*disable avdd, corepll use must enable*/
	sys_clear_bit(AVDDLDO_CTL, 0);

	/* S1 VDD 1.05V
	 * TODO use 0.95V
	 */
	sys_write32((sys_read32(VOUT_CTL1_S1) & ~(0xf)) | 0xa, VOUT_CTL1_S1);

#ifndef CONFIG_SOC_NO_PSRAM
	/* disable SPI1 cache for PSRAM */
	sys_clear_bit(SPI1_CACHE_CTL, 0);
#endif

	bk_clksrc0 = sys_read32(CMU_MEMCLKSRC0);
	bk_clksrc1 = sys_read32(CMU_MEMCLKSRC1);

	while(1) {
		/* RAM4 shareRAM select  RC4MHZ in s2) */
		sys_write32((bk_clksrc0 & (~0x3ff)) | (0x0<<5), CMU_MEMCLKSRC0); /* RC4M */

		sleep_stage(1);
		__cpu_enter_sleep();
		sleep_stage(2);
		#ifndef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
		psram_power_control(false); // psram resume for sensor run
		#endif
		sys_write32((bk_clksrc0 & (~0x3ff)) | (0x4 << 5), CMU_MEMCLKSRC0); /* RC64M */
		sys_write32((bk_clksrc1 & (~0xe)) | (0x4 << 1), CMU_MEMCLKSRC1); /* RC64M and ANX ram */

		sleep_stage(3);
		if (WK_CB_RUN_SYSTEM == check_wk_run_sram_nor(wk_en_bit, cb, &num_cb)) {
			sleep_stage(4);
			break;
		}
#ifndef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
		psram_power_control(true); // now psram suspend
#endif

	}

	sleep_stage(5);

	/* restore memory clock sources */
	sys_write32(bk_clksrc0, CMU_MEMCLKSRC0);
	sys_write32(bk_clksrc1, CMU_MEMCLKSRC1);

	/* hosc+rc4m+64M */
	sys_write32(0x7, CMU_S1CLKCTL);

#ifndef CONFIG_SOC_NO_PSRAM
	/* enable spi1 cache */
	sys_set_bit(SPI1_CACHE_CTL, 0);
#endif

	/* restore VDD */
	sys_write32(bk_vout_s1, VOUT_CTL1_S1);

	/*enable avdd, for pll*/
	sys_set_bit(AVDDLDO_CTL, 0);

	/* resume psram */
	psram_power_control(false);

	for(i = 0; i < 300; i++){
		if(sys_read32(HOSCLDO_CTL) & (1<<25))
			break;
		soc_udelay(5);
	}

	sys_write32(corepll_backup, COREPLL_CTL);
	/*spi0 clk switch to hosc*/
	sys_write32(0x0, CMU_SPI0CLK);
	for(i = 0; i < 300; i++){
		if(sys_read32(COREPLL_CTL) & (1<<8))
			break;
		soc_udelay(5);
	}
	val = sys_read32(HOSC_CTL) & (~(HOSC_CTL_HGMC_MASK | 0xf0));
	val |= 0x30;
	sys_write32(val, HOSC_CTL);

	check_cnt = i;
	soc_udelay(10);

	/*first switch clk ahb div*/
	sys_write32((sys_read32(CMU_SYSCLK)&0x7) | (sysclk_bak & (~0x7)) , CMU_SYSCLK);
	soc_udelay(1);
	sys_write32(sysclk_bak, CMU_SYSCLK);

	psram_self_refresh_control(false);

#ifdef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
	if (is_sensor_run_psram) {
		spi0_cache_switch_mapping(1);
#ifdef CONFIG_SPI_NOR_FLASH_4B_ADDRESS
		sys_norflash_enter_4b();
#endif
	}
#else
	sys_write32(spi1_clk, CMU_SPI1CLK); // restore spi1 clk for spsram
	sys_write32(ddr_mode, SPI1_DDR_MODE_CTL);
#endif
#ifndef CONFIG_SOC_NO_PSRAM
	/*spi1 rx fifo reset*/
	sys_clear_bit(SPI1_CTL, 4);
	soc_udelay(1);
	sys_set_bit(SPI1_CTL, 4);
	/* invalid spi1 cache */
	spi1_cache_ops(SPI_CACHE_INVALID_ALL, (void *)SPI1_BASE_ADDR, 0x1000);
#endif

#ifdef CONFIG_SLEEP_MEMORY_CHECK_INTEGRITY
	resume_check();
#endif

	current_sleep->g_num_cb = num_cb;
	memcpy(current_sleep->g_syste_cb, cb,
			num_cb * sizeof(struct sleep_wk_cb));

#ifdef CONFIG_SLEEP_COPY_SRAM_FOR_SENSOR
	mem_psram_recovery_tosram();
#endif

}

/*switch sp to sram*/
static void cpu_enter_sleep(void)
{
	//static unsigned int sp;

	//sp = __get_PSP();

	/* change sp to sram top */
	//set_PSP(SP_IN_SLEEP);
	cpu_enter_sleep_tmp();
	//set_PSP(sp);  /* recovery sp */
}

#ifdef CONFIG_SLEEP_TIMER_DEBUG
#define soc_enter_sleep_switch_uart(x)		NULL
#else
#ifdef CONFIG_SLEEP_UART_SILENT
#define soc_enter_sleep_switch_uart(x)		NULL
#else
static const struct acts_pin_config bt_pin_cfg_uart[] = {
	{28, 23 | GPIO_CTL_PADDRV_LEVEL(1) | GPIO_CTL_PULLUP},
	{29, 23 | GPIO_CTL_PADDRV_LEVEL(1) | GPIO_CTL_PULLUP},
};

static const struct acts_pin_config lark_pin_cfg_uart[] = {
	{28, 5 | GPIO_CTL_PADDRV_LEVEL(1) | GPIO_CTL_PULLUP},
	{29, 5 | GPIO_CTL_PADDRV_LEVEL(1) | GPIO_CTL_PULLUP},
};

static void soc_enter_sleep_switch_uart(bool bt_uart)
{
	if(current_sleep->sleep_mode)
		return;

	if (bt_uart) {
		acts_pinmux_setup_pins(bt_pin_cfg_uart, ARRAY_SIZE(bt_pin_cfg_uart));
	} else {
		acts_pinmux_setup_pins(lark_pin_cfg_uart, ARRAY_SIZE(lark_pin_cfg_uart));
	}
}
#endif /* CONFIG_SLEEP_UART_SILENT */
#endif /* CONFIG_SLEEP_TIMER_DEBUG */

void dump_reg(const char *promt)
{
	printk("%s LOSC_CTL=0x%x,sleep_mode=%d\n", promt, sys_read32(LOSC_CTL), current_sleep->sleep_mode);
#if 0
	int i;
	for (i = 0; i < 77 ; i++)// nor pin
		printk("gpio%d=0x%x\n", i, sys_read32(GPION_CTL(i)));
	printk("RMU_MRCR0=0x%x\n", sys_read32(RMU_MRCR0));
	printk("RMU_MRCR1=0x%x\n", sys_read32(RMU_MRCR1));
	printk("CMU_DEVCLKEN0=0x%x\n", sys_read32(CMU_DEVCLKEN0));
	printk("CMU_DEVCLKEN1=0x%x\n", sys_read32(CMU_DEVCLKEN1));
	printk("PMU_DET=0x%x\n", sys_read32(PMU_DET));
	printk("NVIC_ISPR0=0x%x\n", sys_read32(NVIC_ISPR0));
	printk("NVIC_ISPR1=0x%x\n", sys_read32(NVIC_ISPR1));
	printk("CMU_MEMCLKEN0=0x%x\n", sys_read32(CMU_MEMCLKEN0));
	printk("CMU_MEMCLKEN1=0x%x\n", sys_read32(CMU_MEMCLKEN1));
	printk("CMU_MEMCLKSRC0=0x%x\n", sys_read32(CMU_MEMCLKSRC0));
	printk("CMU_MEMCLKSRC1=0x%x\n", sys_read32(CMU_MEMCLKSRC1));
	printk("PWRGATE_DIG=0x%x\n", sys_read32(PWRGATE_DIG));
	printk("VOUT_CTL1_S2=0x%x\n", sys_read32(VOUT_CTL1_S2));
	printk("VOUT_CTL1_S3=0x%x\n", sys_read32(VOUT_CTL1_S3));
	printk("PWRGATE_RAM=0x%x\n", sys_read32(PWRGATE_RAM));
	printk("WIO0_CTL=0x%x\n", sys_read32(WIO0_CTL));
	printk("WIO1_CTL=0x%x\n", sys_read32(WIO1_CTL));
	printk("WIO2_CTL=0x%x\n", sys_read32(WIO2_CTL));
	printk("WIO3_CTL=0x%x\n", sys_read32(WIO3_CTL));
#endif

}

unsigned int soc_sleep_cycle(void)
{
	return current_sleep->g_sleep_cycle;
}

 __sleepfunc int64_t soc_sys_uptime_get(void)
{
	if(g_sleep_st){
		unsigned int ms;
		while(g_sleep_t2cnt == sys_read32(T2_CNT)); // wait for clock sync
		ms =  rc32k_cyc_to_ms(sys_read32(T2_CNT)-g_sleep_st);
		return (g_sleep_update_time+g_sleep_ms) + ms;
	}else{
		return (k_uptime_get()+g_sleep_ms);
	}
}
static void soc_timer_sleep_prepare(void)
{
	g_sleep_update_time = k_uptime_get();
	acts_clock_peripheral_disable(CLOCK_ID_TIMER); // for switch clk source
	sys_write32(0x4, CMU_TIMER2CLK); /* select rc32k before enter S3 */
	g_sleep_st = sys_read32(T2_CNT);
	acts_clock_peripheral_enable(CLOCK_ID_TIMER);
}

static void soc_timer_exit_sleep(void)
{
	unsigned int end;
	acts_clock_peripheral_disable(CLOCK_ID_TIMER); // for switch clk source
	end =  sys_read32(T2_CNT);
	sys_write32(0x0, CMU_TIMER2CLK); /* select hosc for k_cycle_get */
	acts_clock_peripheral_enable(CLOCK_ID_TIMER);
	end = end - g_sleep_st;
	dump_stage_timer(g_sleep_st, end);
	current_sleep->g_sleep_cycle += soc_rc32K_mutiple_hosc()*end;
	g_sleep_ms += rc32k_cyc_to_ms(end);
	g_sleep_st = 0;
}

static uint8_t g_lcd_aod_mode;

void soc_set_aod_mode(int is_aod)
{
	g_lcd_aod_mode = is_aod ? 1 : 0;
	if (is_aod) {
		sys_s3_wksrc_set(SLEEP_WK_SRC_RTC);
	} else {
		sys_s3_wksrc_clr(SLEEP_WK_SRC_RTC);
	}
}

int soc_get_aod_mode(void)
{
	return g_lcd_aod_mode;
}

void soc_set_sleep_mode(uint8_t mode)
{
	if(mode > SLEEP_MODE_LOWPOWER)
		mode = SLEEP_MODE_LOWPOWER;
	current_sleep->sleep_mode = mode;
	printk("sleep mode=%d\n", current_sleep->sleep_mode);
}


void soc_enter_deep_sleep(void)
{
	dump_reg("before");
	soc_pmu_check_onoff_reset_func();
#ifdef CONFIG_SLEEP_DISABLE_BT
	soc_cmu_sleep_prepare(0);
#else
	soc_cmu_sleep_prepare(!current_sleep->sleep_mode);
#endif
	soc_timer_sleep_prepare();
	soc_enter_sleep_switch_uart(true);
	cpu_enter_sleep();/* wfi,enter to sleep */
	soc_enter_sleep_switch_uart(false);
	soc_timer_exit_sleep();
	soc_cmu_sleep_exit();
	dump_reg("BT after");
#ifdef CONFIG_SLEEP_STAGE_DEBUG
	soc_pstore_set(SOC_PSTORE_TAG_SLEEP_DBG_STAGE, 0);
#endif
}

void soc_enter_light_sleep(void)
{
	dump_reg("before");

	soc_cmu_sleep_prepare(0);
	cpu_enter_sleep();/* wfi,enter to sleep */
	soc_cmu_sleep_exit();

	dump_reg("after");
}
static void s2_init(void)
{
	ppi_trig_src_en(TIMER0, 1);
	ppi_task_trig_config(PPI_CH11, SPIMT1_TASK6, TIMER0);
}
static int soc_sleep_init(const struct device *dev)
{
	int i;

	ARG_UNUSED(dev);
#ifdef CONFIG_SLEEP_STAGE_DEBUG
	uint32_t val;
	soc_pstore_get(SOC_PSTORE_TAG_SLEEP_DBG_STAGE, &val);
	printk("SLEEP_DBG_STAGE=0x%x\n", val);
#endif
	g_lcd_aod_mode = 0;
	current_sleep->g_sleep_cycle = 0;
	current_sleep->sleep_mode = 0;
	g_sleep_ms = 0;
	g_sleep_st = 0;
	g_sleep_wksrc_en = 0;
	g_cyc2ms_mul = (1000<<16) / soc_rc32K_freq();

	for(i = 0; i < SLEEP_WKSRC_NUM; i++)
		g_wk_fun[i] = NULL;

#ifdef CONFIG_SLEEP_SENSOR_RUN_ON_NOR
	#if 0
	const struct partition_entry *system_part = NULL;
	system_part = partition_get_part(PARTITION_FILE_ID_SYSTEM);
	if (!system_part) {
		printk("failed to get system part!!!\n");
		return -1;
	}
	current_sleep->system_part_offset = system_part->offset;
	current_sleep->sysmap_entry_addr = sys_read32(SPI_CACHE_ADDR0_ENTRY);
	#else
	uint32_t *poff =(uint32_t *)soc_boot_get_info();
	current_sleep->system_part_offset = poff[10];
	current_sleep->sysmap_entry_addr = sys_read32(SPI_CACHE_ADDR0_ENTRY);
	printk("app entry=0x%x\n", poff[10]);
	#endif
#endif

#ifdef CONFIG_SLEEP_MEMORY_CHECK_INTEGRITY
	current_sleep->check_start_addr = (uint32_t)__bss_start;
	current_sleep->check_len = MEMORY_CHECK_INTEGRITY_SIZE;
#endif
	s2_init();
	return 0;
}

SYS_INIT(soc_sleep_init, PRE_KERNEL_1, 80);


