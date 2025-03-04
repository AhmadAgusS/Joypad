/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file cbuf interface
 */

#include <kernel.h>
#include <string.h>
#include <arch/arm/aarch32/cortex_m/cmsis.h>
#include <kernel_internal.h>
#include <drivers/rtc.h>
#include <soc_pmu.h>


#define  th_name(th)   (k_thread_name_get(th)!= NULL?k_thread_name_get(th):"NA")

#ifdef CONFIG_ARM_UNWIND
void unwind_backtrace(struct k_thread *th);
static void thread_show_info(const struct k_thread *cthread, void *user_data)
{
	unwind_backtrace((struct k_thread *)cthread);
}

#else


#ifndef EXC_RETURN_FTYPE
/* bit [4] allocate stack for floating-point context: 0=done 1=skipped  */
#define EXC_RETURN_FTYPE           (0x00000010UL)
#endif
static uint32_t sh_adjust_sp_by_fpu(const struct k_thread *th)
{
#if defined(CONFIG_ARM_STORE_EXC_RETURN)
	if((th->arch.mode_exc_return & EXC_RETURN_FTYPE) == 0)
		return 18*4; /*s0-s15 fpscr lr*/
#endif
	return 0;
}

#ifdef CONFIG_DEBUG_COREDUMP
#include <debug/coredump.h>
#include <stdio.h>

#define  PC_MAX_NUM  	 16
#define  THREAD_MAX_NUM  32
#define BK_TH_MAIGIC	0x68
struct bk_th_info {
	uint8_t th_maigic;
	uint8_t th_state;
	uint8_t th_run;
	uint8_t pc_num;
	char th_name[CONFIG_THREAD_MAX_NAME_LEN];
	uint32_t th_pc[PC_MAX_NUM];
};
#define BK_TRACE_MAIGIC	0xBA45
struct bk_trace_info {
	uint16_t bk_maigic;
	uint8_t  bk_maigic1;
	uint8_t  thread_num;
	uint32_t time;
	struct bk_th_info  thread[THREAD_MAX_NUM];
};
#ifdef CONFIG_SOC_NO_PSRAM
__in_section_unique(trace.g_bk_trace.noinit)
#endif
static  struct bk_trace_info  g_bk_trace;

static void bk_trace_init(void)
{
	memset(&g_bk_trace, 0, sizeof(struct bk_trace_info));
	g_bk_trace.bk_maigic = BK_TRACE_MAIGIC;
	g_bk_trace.bk_maigic1 = BK_TH_MAIGIC;
	g_bk_trace.thread_num = 0;
	g_bk_trace.time = 0;
#ifdef CONFIG_RTC_ACTS
	const struct device *rtc = device_get_binding(CONFIG_RTC_0_NAME);
	struct rtc_time tm;
	if(!rtc_get_time(rtc, &tm))
		rtc_tm_to_time(&tm, &g_bk_trace.time);
#endif
}
static struct bk_th_info * bk_get_cur_th_info(void)
{
	if(g_bk_trace.thread_num < THREAD_MAX_NUM)
		return &g_bk_trace.thread[g_bk_trace.thread_num];
	else
		return NULL;
}
static void bk_th_info_init(const char *name, uint8_t stat, uint8_t b_run)
{
	struct bk_th_info *th;
	th = bk_get_cur_th_info();
	if(th == NULL)
		return;
	th->th_maigic = BK_TH_MAIGIC;
	th->th_run = b_run;
	th->th_state = stat;
	strncpy(th->th_name, name, CONFIG_THREAD_MAX_NAME_LEN);
	th->pc_num = 0;
}
static void bk_th_info_set_pc(uint32_t pc)
{
	struct bk_th_info *th;
	th = bk_get_cur_th_info();
	if(th == NULL)
		return;
	if(th->pc_num < PC_MAX_NUM)
		th->th_pc[th->pc_num++] = pc;
}
static void bk_th_info_end(void)
{
	if(g_bk_trace.thread_num < THREAD_MAX_NUM)
		g_bk_trace.thread_num++;
}

void bk_th_coredump(void)
{
	uintptr_t start =(uintptr_t)(&g_bk_trace);
	coredump_memory_dump(start, start+ 8 + g_bk_trace.thread_num *(sizeof(struct bk_th_info)));
}

int bk_th_coredump_set(uint32_t start, uint8_t *buf, uint32_t offset, int len)
{
	uint32_t addr =(uintptr_t)(&g_bk_trace);
	uint32_t size = sizeof(g_bk_trace);
	uint8_t *pb = (uint8_t *) &g_bk_trace;
	if(start != addr)
		return 0;
	if(offset > size){
		//printk("th over\n");
		return 1;
	}
	if(offset+len <= size) {
		memcpy(pb+offset, buf, len);
	}else{
		//printk("th mybe over\n");
		memcpy(pb+offset, buf, size-offset);
	}
	return 1;
}
int  bk_th_coredump_out(int (*out_cb)(uint8_t *data, uint32_t len))
{
	struct rtc_time *tm, t;
	struct bk_th_info *th;
	char buf[164];
	int i,j, len, all = 0;
	if(g_bk_trace.bk_maigic != BK_TRACE_MAIGIC){
		len = sprintf(buf, "th magic fail: 0x%x\r\n",  g_bk_trace.bk_maigic);
		out_cb(buf, len);
		return len;
	}
	tm = &t;
	rtc_time_to_tm(g_bk_trace.time, tm);
	len = snprintf(buf, 127, "time: %02d-%02d-%02d  %02d:%02d:%02d:%03d, num=%d\r\n",
		1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
		tm->tm_sec, tm->tm_ms , g_bk_trace.thread_num);
	out_cb(buf, len);
	all +=len;

	if(g_bk_trace.thread_num > THREAD_MAX_NUM)
		g_bk_trace.thread_num = THREAD_MAX_NUM;

	for(i = 0; i < g_bk_trace.thread_num; i++){
		th = &g_bk_trace.thread[i];
		len = snprintf(buf, 159, "%s %d thread=%s, state=%d:\r\n",(th->th_run? "*" : " "), i, th->th_name, th->th_state);
		out_cb(buf, len);
		all +=len;
		if(th->th_maigic != BK_TH_MAIGIC){
			len = snprintf(buf, 159, "--err maigic fail---\r\n");
			out_cb(buf, len);
			all +=len;
		}else{
			len = snprintf(buf, 159,"%%ZEPHYR_TOOLS%%\\gcc-arm-none-eabi-9-2020-q2-update-win32\\bin\\arm-none-eabi-addr2line.exe -e zephyr.elf -a -f ");
			out_cb(buf, len);
			all +=len;
			if(th->pc_num > PC_MAX_NUM)
				th->pc_num = PC_MAX_NUM;
			len = 0;
			for(j = 0; j < th->pc_num; j++){
				len += snprintf(buf+len, 159-len, "%08x ",th->th_pc[j]);
				if(len >= 159)
					break;
			}
			buf[len++] = '\r';
			buf[len++] = '\n';
			buf[len] = 0;
			out_cb(buf, len);
			all +=len;
		}
	}
	return all;
}
#else
static void bk_trace_init(void) {}
static void bk_th_info_init(const char *name, uint8_t stat, uint8_t b_run){}
static void bk_th_info_set_pc(uint32_t pc) {}
static void bk_th_info_end(void) {}
#endif



static void sh_dump_mem(const char *msg, uint32_t mem_addr, int len)
{
	int i;
	uint32_t *ptr = (uint32_t *)mem_addr;
	printk("%s=0x%08x\n", msg, mem_addr);
	for(i = 0; i < len/4; i++)
	{
		printk("%08x ", ptr[i]);
		if(i % 8 == 7)
			printk("\n");
	}
	printk("\n\n");
}

static void sh_backtrace_print_elx(uint32_t sp, uint32_t sp_start, uint32_t stk_end, const z_arch_esf_t *esf)
{
	uint32_t where = sp_start;
	uint32_t pc = 0;

	if(sp > stk_end){
		printk("stack overflow SP<%08x> at [START <%08x>] from [END <%08x>]\n", sp, sp_start, stk_end);
		return;
	}

    printk("\n%%ZEPHYR_TOOLS%%\\gcc-arm-none-eabi-9-2020-q2-update-win32\\bin\\arm-none-eabi-addr2line.exe -e zephyr.elf -a -f ");
	if(esf){
		printk("%08x %08x ", esf->basic.pc, esf->basic.lr);
		bk_th_info_set_pc(esf->basic.pc);
		bk_th_info_set_pc(esf->basic.lr);
	}
	while (where < stk_end) {
		pc = *(uint32_t*)where;
		if( (pc > 0x10000000) && (pc < 0x10200000) ){
			printk("%08x ", pc);
			bk_th_info_set_pc(pc);
		}

		where += 4;
	}
	printk("\n\n");
	sh_dump_mem("sp=", sp, 512);

}
static void sh_print_thread_info(struct k_thread *th)
{
	const char *tname;
	tname = k_thread_name_get(th);
	printk("\n %s%p %s state: %s backtrace \n", ((th == k_current_get()) ? "*" : " "),
		th, (tname?tname : "NA"), k_thread_state_str(th));
	bk_th_info_init((tname?tname : "NA"), th->base.thread_state, (th == k_current_get()));
}

static void sh_backtrace(const struct k_thread *th)
{
	const struct _callee_saved *callee;
	const z_arch_esf_t *esf;
	uint32_t sp, sp_end;

	if(th == NULL)
		th = k_current_get();
	sh_print_thread_info(( struct k_thread *)th);
	sp_end = th->stack_info.start + th->stack_info.size;
	if(th == k_current_get()) {
		sp = __get_PSP();
		if(k_is_in_isr()){
			callee = &th->callee_saved;
			esf = (z_arch_esf_t *)sp;
			sh_backtrace_print_elx(sp, sp+32+sh_adjust_sp_by_fpu(th), sp_end , esf);
		}else{
			sp = __get_PSP();
			sh_backtrace_print_elx(sp, sp, sp_end , NULL);
		}
	}else{
		callee = &th->callee_saved;
		esf = (z_arch_esf_t *)callee->psp;
		sp = callee->psp + 32 + sh_adjust_sp_by_fpu(th); /*r0-r3, r12/lr/pc/xpsr*/
		sh_backtrace_print_elx(callee->psp, sp, sp_end, esf);
	}
	bk_th_info_end();

}

static void sh_backtrace_irq(const z_arch_esf_t *esf)
{
	int i;
	uint32_t sp;
	sp  = __get_MSP();
	for(i = 0; i < 128; i++) {// find irq sp context
		if(memcmp((void*)sp, esf, 32) == 0)
			break;
		sp += 4;
	}
	if(i == 128){
		printk("not find irq sp\n");
		return ;
	}
	printk("show irq stack:\n");
	bk_th_info_init("irq:", 0, 0);
	sh_backtrace_print_elx(sp, sp+32, sp+1024, esf);
	bk_th_info_end();
}


static void stack_dump(const struct k_thread *th)
{
#if 0
	const z_arch_esf_t *esf;
	const struct _callee_saved *callee = &th->callee_saved;
	esf = (z_arch_esf_t *)callee->psp;
	printk("############ thread: %p info############\n", th);
	printk("r0/a1:  0x%08x  r1/a2:  0x%08x  r2/a3:  0x%08x\n",
		esf->basic.a1, esf->basic.a2, esf->basic.a3);
	printk("r3/a4:  0x%08x r12/ip:  0x%08x r14/lr:  0x%08x\n",
		esf->basic.a4, esf->basic.ip, esf->basic.lr);
	printk("r4/v1:  0x%08x  r5/v2:  0x%08x  r6/v3:  0x%08x\n",
		callee->v1, callee->v2, callee->v3);
	printk("r7/v4:  0x%08x  r8/v5:  0x%08x  r9/v6:  0x%08x\n",
		callee->v4, callee->v5, callee->v6);
	printk("r10/v7: 0x%08x  r11/v8: 0x%08x    psp:  0x%08x\n",
			callee->v7, callee->v8, callee->psp);
	printk(" xpsr:  0x%08x\n", esf->basic.xpsr);
	printk("(r15/pc): 0x%08x\n",	esf->basic.pc);
#endif

}

static void thread_show_info(const struct k_thread *cthread, void *user_data)
{
	//const struct k_thread *cur = (const struct k_thread *)user_data;
	stack_dump(cthread);
	sh_backtrace(cthread);
}

/*exceptions printk*/
void k_sys_fatal_error_handler(unsigned int reason,
				      const z_arch_esf_t *esf)
{
	soc_pmu_check_onoff_reset_func();
	bk_trace_init();
	if ((esf != NULL) && arch_is_in_nested_exception(esf)) {
		sh_backtrace_irq(esf);
	}
	printk("stop system\n");
	k_thread_foreach(thread_show_info, NULL);
}
#endif

void show_stack(void)
{
	struct k_thread *cur = (struct k_thread *) k_current_get();
	#ifdef CONFIG_ACTIONS_PRINTK_DMA
	printk_dma_switch(0);
	#endif
	printk("****show thread stack cur=%s ****\n", th_name(cur));
	k_thread_foreach(thread_show_info, NULL);
	#ifdef CONFIG_ACTIONS_PRINTK_DMA
	printk_dma_switch(1);
	#endif
}


