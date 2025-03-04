/*
 * Copyright (c) 2017 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <drivers/timer/system_timer.h>
#include <sys_clock.h>
#include <spinlock.h>
#include <soc.h>

#define USE_T2_FOR_CYCLE

#define CYC_PER_TICK (sys_clock_hw_cycles_per_sec()	\
		      / CONFIG_SYS_CLOCK_TICKS_PER_SEC)
#define MAX_TICKS ((TIMER_MAX_CYCLES_VALUE / CYC_PER_TICK) - 1)
#define MAX_CYCLES (MAX_TICKS * CYC_PER_TICK)




#define TICKLESS (IS_ENABLED(CONFIG_TICKLESS_KERNEL))

static struct k_spinlock lock;

static uint32_t last_load;

/*
 * This local variable holds the amount of SysTick HW cycles elapsed
 * and it is updated in z_clock_isr() and sys_clock_set_timeout().
 *
 * Note:
 *  At an arbitrary point in time the "current" value of the SysTick
 *  HW timer is calculated as:
 *
 * t = cycle_counter + elapsed();
 */
static uint32_t cycle_count;

/*
 * This local variable holds the amount of elapsed SysTick HW cycles
 * that have been announced to the kernel.
 */
static uint32_t announced_cycles;
static volatile uint8_t  sleep_clk_div = 1;
#define T_SLEEP()	(sleep_clk_div == 8)
#define T_SLEEP_CNT(val)	((val)<<3)
#define T_SLEEP_LOAD(val)	((val)>>3)

static uint32_t acts_timer_elapsed(void)
{
	uint32_t val1;
	uint32_t ctrl;
	uint32_t ret;

	do{ //fix timer is not monotonic, penddig and count not sync
		ctrl = sys_read32(T0_CTL);
		val1 = sys_read32(T0_CNT);
	}while(ctrl != sys_read32(T0_CTL) || (val1 < 10));

	if(ctrl& (1<<T0_CTL_ZIPD)){// overflow
		ret = last_load;
	}else{
		ret = 0;
	}
	if(last_load > val1)// t0 count
		ret += (last_load - val1);

	if(T_SLEEP())
		return T_SLEEP_CNT(ret);
	else
		return ret;

}

static void timer_reload(uint32_t value)
{
	sys_write32(0x1, T0_CTL); //clear pending stop timer
	timer_reg_wait();
	sys_write32(value, T0_VAL);	
	sys_write32(0x26, T0_CTL); /* enable counter down,  reload, irq */
}
#ifdef USE_T2_FOR_CYCLE
static void timer_cyc_init(void)
{
	sys_write32(0x1, T2_CTL); //clear pending stop timer
	timer_reg_wait();
	sys_write32(TIMER_MAX_CYCLES_VALUE, T2_VAL);	
	sys_write32(0x824, T2_CTL); /* enable counter up,  reload,  notirq */
}
#endif


/* Callout out of platform assembly, not hooked via IRQ_CONNECT... */
void acts_timer_isr(void *arg)
{
	ARG_UNUSED(arg);
	uint32_t dticks;

	/* Increment the amount of HW cycles elapsed (complete counter
	 * cycles) and announce the progress to the kernel.
	 */
	sys_write32(sys_read32(T0_CTL), T0_CTL); // clear penddig;
	cycle_count += last_load;
	if (TICKLESS) {
		dticks = (cycle_count - announced_cycles) / CYC_PER_TICK;
		announced_cycles += dticks * CYC_PER_TICK;
		sys_clock_announce(dticks);
	} else {
		sys_clock_announce(1);
	}
	//printk("-ie=%u, 0x%x, %u\n", cycle_count, sys_read32(T0_CTL), last_load);
}


int sys_clock_driver_init(const struct device *device)
{
	/* init timer0 as clock event device */
#if ((CONFIG_HOSC_CLK_MHZ*1000000) == CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC)
	sys_write32(0x0, CMU_TIMER0CLK); //select hosc
	sys_write32(0x0, CMU_TIMER2CLK); //select hosc
#elif (4000000 == CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC) 
	sys_write32(0x2, CMU_TIMER0CLK); //4mhz rc4m
	sys_write32(0x2, CMU_TIMER2CLK); //4mhz rc4m
#else
	#error "CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC  cfg error"
#endif
	acts_clock_peripheral_enable(CLOCK_ID_TIMER);
	last_load = CYC_PER_TICK;
	timer_reload(last_load-1);
	#ifdef USE_T2_FOR_CYCLE
	timer_cyc_init();
	#endif
	IRQ_CONNECT(IRQ_ID_TIMER0, 1, acts_timer_isr, 0, 0);
	irq_enable(IRQ_ID_TIMER0);

	return 0;
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{

#if defined(CONFIG_TICKLESS_KERNEL)
	uint32_t delay;

	ticks = (ticks == K_TICKS_FOREVER) ? MAX_TICKS : ticks;
	if(!ticks)
		ticks = 1;

	if(T_SLEEP()){
		cycle_count += acts_timer_elapsed();
		last_load = T_SLEEP_LOAD(ticks * CYC_PER_TICK);
		timer_reload(last_load -1);
		return;
	}

	k_spinlock_key_t key = k_spin_lock(&lock);

	uint32_t pending = acts_timer_elapsed();

	cycle_count += pending;

	uint32_t unannounced = cycle_count - announced_cycles;

	if ((int32_t)unannounced < 0) {
		/* We haven't announced for more than half the 32-bit
		 * wrap duration, because new timeouts keep being set
		 * before the existing one fires.  Force an announce
		 * to avoid loss of a wrap event, making sure the
		 * delay is at least the minimum delay possible.
		 */
		//last_load = MIN_DELAY;
		last_load = CYC_PER_TICK;
	} else {
		/* Desired delay in the future */
		delay = ticks * CYC_PER_TICK;
		/* Round delay up to next tick boundary */
		delay += unannounced;
		delay =
		 ((delay + CYC_PER_TICK - 1) / CYC_PER_TICK) * CYC_PER_TICK;
		delay -= unannounced;
		delay = MAX(delay, CYC_PER_TICK);
		if (delay > MAX_CYCLES) {
			last_load = MAX_CYCLES;
		} else {
			last_load = delay;
		}
	}	
	timer_reload(last_load -1);
	k_spin_unlock(&lock, key);
#endif
	
}

#if defined(CONFIG_PM_DEVICE)
int sys_clock_device_ctrl(const struct device *device, enum pm_device_action action)
{
	if(action == PM_DEVICE_ACTION_RESUME){		
#if ((CONFIG_HOSC_CLK_MHZ*1000000) == CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC)
		sys_clock_set_timeout(1, 0);  /*old clk add pennding to cycle*/
		sleep_clk_div = 1;
		sys_write32(0x0, CMU_TIMER0CLK); //select hosc
		sys_clock_set_timeout(1, 0); /*new clk count*/
#else
		sys_write32(0x2, CMU_TIMER0CLK); //4mhz rc4m
#endif
		
	} else if (action == PM_DEVICE_ACTION_SUSPEND){
		sys_write32(0x2, CMU_TIMER0CLK); //4mhz rc4m
#if ((CONFIG_HOSC_CLK_MHZ*1000000) == CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC)		
		sys_clock_set_timeout(CONFIG_SYS_CLOCK_TICKS_PER_SEC/10, 0); /*old clk add pennding to cycle*/
		sleep_clk_div = 8;
		sys_clock_set_timeout(last_load/CYC_PER_TICK, 0);/*new clk count*/
#else
		sleep_clk_div = 1;
#endif
	}
	return 0;
}

#endif

uint32_t sys_clock_elapsed(void)
{
	if (!TICKLESS) {
		return 0;
	}
	k_spinlock_key_t key = k_spin_lock(&lock);
	uint32_t cyc = acts_timer_elapsed() + cycle_count - announced_cycles;
	k_spin_unlock(&lock, key);
	return cyc / CYC_PER_TICK;
}



uint32_t sys_clock_cycle_get_32(void)
{
#ifdef USE_T2_FOR_CYCLE
	return sys_read32(T2_CNT);
#else
	k_spinlock_key_t key = k_spin_lock(&lock);
	uint32_t ret = acts_timer_elapsed() + cycle_count;
	k_spin_unlock(&lock, key);
	return ret;
#endif
}

void z_clock_idle_exit(void)
{

}
void sys_clock_disable(void)
{
	/* clear pending & disable timer */
	//sys_write32(0x1, T0_CTL);

}


