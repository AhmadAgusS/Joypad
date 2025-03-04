/*******************************************************************************
 * @file    sensor_sleep.c
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2020-08-12
 * @brief   sensor testing
*******************************************************************************/

/******************************************************************************/
//includes
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <zephyr.h>
#include <soc.h>
#include <drivers/i2cmt.h>
#include <sensor_hal.h>
#include <sensor_bus.h>
#include <sensor_algo.h>
#include <sensor_devices.h>
#include <hr_algo.h>
#include <linker/linker-defs.h>
#include "sensor_port.h"

/******************************************************************************/
//constants
/******************************************************************************/

/******************************************************************************/
//variables
/******************************************************************************/
static uint64_t sleep_get_timestamp(void);
static void sleep_evt_handler(sensor_evt_t *evt);

static void sleep_hr_wear_handler(uint8_t wearing_state);
static void sleep_hr_hb_handler(uint8_t hb_val, uint8_t hb_lvl_val, uint16_t rr_val);
static void sleep_hr_spo2_handler(uint8_t spo2_val, uint8_t spo2_lvl_val, uint8_t hb_val,
				uint8_t hb_lvl_val, uint16_t rr_val[4], uint8_t rr_lvl_val, uint8_t rr_cnt, uint16_t spo2_r_val);

static enum WK_CB_RC sleep_wk_callback(enum S_WK_SRC_TYPE wk_src);
static enum WK_RUN_TYPE sleep_wk_prepare(enum S_WK_SRC_TYPE wk_src);
#if CONFIG_SENSOR_ALGO_HR_VCARE	
__sleepfunc static uint32_t hr_get_RTC_count(void);
#endif
#if (CONFIG_SENSOR_ALGO_HR_HX3605 || CONFIG_SENSOR_ALGO_HR_HX3690)
__sleepfunc static int32_t sleep_ms_delay(int32_t ms);
#endif

static __act_s2_sleep_data sensor_os_api_t sleep_os_api =
{
	.dbgOutput = sl_printk,
	.get_timestamp_ns = sleep_get_timestamp,
	.user_handler = sleep_evt_handler,
	.i2c_write = sensor_acc_i2c_write,
	.i2c_read = sensor_acc_i2c_read,
};

static __act_s2_sleep_data hr_os_api_t sleep_hr_os_api =
{
	.dbgOutput = sl_printk,
	.i2c_write = sensor_hr_i2c_write,
	.i2c_read = sensor_hr_i2c_read,
	.wear_handler = sleep_hr_wear_handler,
	.hb_handler = sleep_hr_hb_handler,
	.spo2_handler = sleep_hr_spo2_handler,
	.hrv_handler = NULL,
#if CONFIG_SENSOR_ALGO_HR_VCARE	
	.get_RTC_count = hr_get_RTC_count,
#endif
#if (CONFIG_SENSOR_ALGO_HR_HX3605 || CONFIG_SENSOR_ALGO_HR_HX3690)
	.ms_delay = sleep_ms_delay,
	.us_delay = soc_udelay,
#endif
};

static sensor_os_api_t save_sensor_os_api = {0};
static hr_os_api_t save_hr_os_api = {0};
static __act_s2_sleep_data int wakeup_system = 0;

#if CONFIG_SENSOR_ALGO_HR_VCARE	
__sleepfunc static uint32_t hr_get_RTC_count(void)
{
	return  sensor_hal_get_tm(ID_HR);
}
#endif

#if (CONFIG_SENSOR_ALGO_HR_HX3605 || CONFIG_SENSOR_ALGO_HR_HX3690)
__sleepfunc static int32_t sleep_ms_delay(int32_t ms)
{
	soc_udelay(ms*1000);
	return 0;
}
#endif

static __act_s2_sleep_data struct sleep_wk_fun_data sleep_fn = 
{
	.wk_cb = sleep_wk_callback,
	.wk_prep = sleep_wk_prepare,
};

__sleepfunc static uint64_t sleep_get_timestamp(void)
{
	return soc_sys_uptime_get() * 1000000ull;
}

__sleepfunc static void sleep_evt_handler(sensor_evt_t *evt)
{
	sensor_res_t *res = p_sensor_algo_sleep_api->get_result();
	
	// enable/disable sensor
	switch(evt->id) {
		case ALGO_HANDUP:
			sl_printk("handup=%d\r\n", res->handup);
			if (res->handup == 1) {
				wakeup_system = 1;
			}
			break;
		case RAW_HEARTRATE:
			sl_printk("hr=%d\r\n", (int)res->heart_rate);
			if (res->heart_rate > 0) {
				wakeup_system = 0; 
			}
			break;
	}
}

__sleepfunc static void sleep_algo_handler(uint8_t *buf, int len, uint32_t ms)
{
	int idx;
	sensor_raw_t sdata;

	if (buf != NULL) {
		// start fifo
		if (p_sensor_algo_sleep_api->input_fifo_start) {
			p_sensor_algo_sleep_api->input_fifo_start(ID_ACC, ms * 1000000, 20 * 1000000);
		}
		// input data
		for (idx = 0; idx < len / 6; idx ++) {
			// get value
			sdata.id = ID_ACC;
			sensor_hal_cvt_data(ID_ACC, &buf[6*idx], 6, sdata.fData);
			// input acc data
			p_sensor_algo_sleep_api->input(&sdata);
		}
		// end fifo
		if (p_sensor_algo_sleep_api->input_fifo_end) {
			p_sensor_algo_sleep_api->input_fifo_end(ID_ACC);
		}
	}

	// call algo
	p_sensor_algo_sleep_api->process(ID_ACC);
}

__sleepfunc static void sleep_hr_wear_handler(uint8_t wearing_state)
{
	sensor_raw_t sdata;
	
	// input wear data
	memset(&sdata, 0, sizeof(sdata));
	sdata.id = IN_OFFBODY;
	sdata.fData[0] = wearing_state;
	p_sensor_algo_sleep_api->input(&sdata);
	
	// call algo
	p_sensor_algo_sleep_api->process(sdata.id);
}

__sleepfunc static void sleep_hr_hb_handler(uint8_t hb_val, uint8_t hb_lvl_val, uint16_t rr_val)
{
	sensor_raw_t sdata;
	
	// input wear data
	memset(&sdata, 0, sizeof(sdata));
	sdata.id = IN_HEARTRATE;
	sdata.fData[0] = hb_val;
	sdata.fData[1] = (hb_lvl_val == 0) ? 3 : 0;
	sdata.fData[2] = 200;
	sdata.fData[3] = 40;
	p_sensor_algo_sleep_api->input(&sdata);
	
	// call algo
	p_sensor_algo_sleep_api->process(sdata.id);
}

__sleepfunc static void sleep_hr_spo2_handler(uint8_t spo2_val, uint8_t spo2_lvl_val, uint8_t hb_val,
				uint8_t hb_lvl_val, uint16_t rr_val[4], uint8_t rr_lvl_val, uint8_t rr_cnt, uint16_t spo2_r_val)
{
	// TODO
}

__sleepfunc static enum WK_CB_RC sleep_wk_callback(enum S_WK_SRC_TYPE wk_src)
{
	int len;
	uint8_t *buf;
	uint32_t pd_reg, int_stat;

	sl_printk("callback wk_src=%d\r\n", wk_src);

	switch ((int)wk_src) {
		case SLEEP_WK_SRC_IIC0MT:
			// acc sensor
			buf = i2c_task_get_data(0, 0, ACC_TRIG, &len);
			if (buf != NULL) {
				sl_printk("ACC buf=0x%p, len=%d, time=%d\r\n", buf, len, (uint32_t)soc_sys_uptime_get());

				/* process data */
				sleep_algo_handler(buf, len, soc_sys_uptime_get());
			}
			break;
	
		case SLEEP_WK_SRC_GPIO:
			// check pending
			pd_reg = GPION_IRQ_PD(HR_ISR);
			int_stat = sys_read32(pd_reg);
			if(int_stat & GPIO_BIT(HR_ISR)){
				// clear pending
				sys_write32(GPIO_BIT(HR_ISR), pd_reg);
				
				// hr sensor
				sl_printk("hr proc start\r\n");
				p_hr_algo_api->process();
				sl_printk("hr proc end\r\n");
			}else{
				return WK_CB_CARELESS;
			}
			break;

		case SLEEP_WK_SRC_T3:
			sensor_hal_clear_tm_pending(TIMER3);
			/* process data */
			sl_printk("time %d\r\n", (uint32_t)soc_sys_uptime_get());
#if CONFIG_SENSOR_ALGO_MOTION_SILAN
			sleep_algo_handler(NULL, 0, 0);
#endif
#if (CONFIG_SENSOR_ALGO_HR_HX3605 || CONFIG_SENSOR_ALGO_HR_HX3690)
			p_hr_algo_api->process();
#endif
#if CONFIG_SENSOR_ALGO_MOTION_CYWEE_DML
			// call algo
			p_sensor_algo_sleep_api->process(ID_ACC);
#endif
			break;
	}

	if (wakeup_system) {
		wakeup_system = 0;
		return WK_CB_RUN_SYSTEM;
	} else {
		return WK_CB_SLEEP_AGAIN;
	}
}

#ifdef __UVISION_VERSION
__attribute__((no_stack_protector))
#else
__attribute__((optimize("no-stack-protector")))
#endif
__sleepfunc static enum WK_RUN_TYPE sleep_wk_prepare(enum S_WK_SRC_TYPE wk_src)
{
//	sl_printk("prepare wk_src=%d\r\n", wk_src);
	return WK_RUN_IN_NOR; // WK_RUN_IN_SYTEM for debug
}

/******************************************************************************/
//functions
/******************************************************************************/
#if defined(CONFIG_PM)
#include <pm/pm.h>
/*call before enter sleep*/
static void sensor_pm_notifier_entry(enum pm_state state)
{
	// save os api
	save_sensor_os_api = *p_sensor_algo_api->get_os_api();
	save_hr_os_api = *p_hr_algo_api->get_os_api();
	
	// set os api
	p_sensor_algo_api->set_os_api(&sleep_os_api);
	p_hr_algo_api->set_os_api(&sleep_hr_os_api);

	// task busy wait
	sensor_bus_task_wait(1);
}

/*call after exit sleep*/
static void sensor_pm_notifier_exit(enum pm_state state)
{
	// restore os api
	p_sensor_algo_api->set_os_api(&save_sensor_os_api);
	p_hr_algo_api->set_os_api(&save_hr_os_api);

	// task sem wait
	sensor_bus_task_wait(0);
}

static struct pm_notifier sensor_notifier = {
	.state_entry = sensor_pm_notifier_entry,
	.state_exit = sensor_pm_notifier_exit,
};
#endif

int sensor_sleep_init(void)
{
	// acc wakeup
	sys_s3_wksrc_set(SLEEP_WK_SRC_IIC0MT);
	sys_s3_wksrc_set(SLEEP_WK_SRC_T3);

#ifdef CONFIG_SENSOR_SLEEP_ROM_BASE
	sleep_sensor_code_set(__sensor_algo_sleep_start, __sensor_algo_sleep_size);
#endif

	sleep_register_wk_callback(SLEEP_WK_SRC_IIC0MT, &sleep_fn);
	sleep_register_wk_callback(SLEEP_WK_SRC_T3, &sleep_fn);
	
	// hr wakeup
	sys_s3_wksrc_set(SLEEP_WK_SRC_GPIO);
	sleep_register_wk_callback(SLEEP_WK_SRC_GPIO, &sleep_fn);

#if defined(CONFIG_PM)
	pm_notifier_register(&sensor_notifier);
#endif
	return 0;
}

int sensor_sleep_suspend(void)
{
	return sensor_hal_suspend();
}

int sensor_sleep_resume(void)
{
	return sensor_hal_resume();
}
