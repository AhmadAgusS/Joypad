/*******************************************************************************
 * @file    algo_port.c
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
#include <sensor_hal.h>
#include <sensor_algo.h>
#include <hr_algo.h>
#include <os_common_api.h>
#include "sensor_port.h"
#include "algo_port.h"

/******************************************************************************/
//constants
/******************************************************************************/

/******************************************************************************/
//variables
/******************************************************************************/
static uint64_t usr_get_timestamp(void);
static void usr_evt_handler(sensor_evt_t *evt);

static void hr_wear_handler(uint8_t wearing_state);
static void hr_hb_handler(uint8_t hb_val, uint8_t hb_lvl_val, uint16_t rr_val);
static void hr_spo2_handler(uint8_t spo2_val, uint8_t spo2_lvl_val, uint8_t hb_val,
				uint8_t hb_lvl_val, uint16_t rr_val[4], uint8_t rr_lvl_val, uint8_t rr_cnt, uint16_t spo2_r_val);
				
#if CONFIG_SENSOR_ALGO_HR_VCARE				
static uint32_t hr_get_RTC_count(void);
#endif

const static sensor_os_api_t os_api =
{
	.dbgOutput = printf,
	.get_timestamp_ns = usr_get_timestamp,
	.user_handler = usr_evt_handler,
	.i2c_write = sensor_acc_i2c_write,
	.i2c_read = sensor_acc_i2c_read,
};

const static hr_os_api_t hr_os_api =
{
	.dbgOutput = printf,
	.i2c_write = sensor_hr_i2c_write,
	.i2c_read = sensor_hr_i2c_read,
	.wear_handler = hr_wear_handler,
	.hb_handler = hr_hb_handler,
	.spo2_handler = hr_spo2_handler,
	.hrv_handler = NULL,
#if CONFIG_SENSOR_ALGO_HR_VCARE	
	.get_RTC_count = hr_get_RTC_count,
#endif

#if (CONFIG_SENSOR_ALGO_HR_HX3605 || CONFIG_SENSOR_ALGO_HR_HX3690)
	.ms_delay = k_msleep,
	.us_delay = soc_udelay,
#endif
};

static algo_cb_t usr_algo_cb = NULL;

/******************************************************************************/
//functions
/******************************************************************************/
#if CONFIG_SENSOR_ALGO_HR_VCARE	
static uint32_t hr_get_RTC_count(void)
{
	return  sensor_hal_get_tm(ID_HR);
}
#endif

static uint64_t usr_get_timestamp(void)
{
	return soc_sys_uptime_get() * 1000000ull;
}

static void usr_evt_handler(sensor_evt_t *evt)
{
	sensor_res_t *res = p_sensor_algo_api->get_result();
	
	// dump event
//	p_sensor_algo_api->dump_event(evt);
	
	// enable/disable sensor
	switch(evt->id) {
		case REQ_SENSOR:
			if ((int)evt->fData[2]) {
				sensor_hal_enable((int)evt->fData[1]);
				if ((int)evt->fData[1] == IN_HEARTRATE) {
					p_hr_algo_api->start(HB);
				}
			} else {
				sensor_hal_disable((int)evt->fData[1]);
				if ((int)evt->fData[1] == IN_HEARTRATE) {
					p_hr_algo_api->stop();
				}
			}
			break;
	}

	// callback
	if (usr_algo_cb != NULL) {
		usr_algo_cb(evt->id, res);
	}
}

static void hr_wear_handler(uint8_t wearing_state)
{
	sensor_raw_t sdata;
	
	// input wear data
	memset(&sdata, 0, sizeof(sdata));
	sdata.id = IN_OFFBODY;
	sdata.fData[0] = wearing_state;
	p_sensor_algo_api->input(&sdata);
	
	// call algo
	p_sensor_algo_api->process(sdata.id);
}

static void hr_hb_handler(uint8_t hb_val, uint8_t hb_lvl_val, uint16_t rr_val)
{
	sensor_raw_t sdata;
	
	// input wear data
	memset(&sdata, 0, sizeof(sdata));
	sdata.id = IN_HEARTRATE;
	sdata.fData[0] = hb_val;
	sdata.fData[1] = (hb_lvl_val == 0) ? 3 : 0;
	sdata.fData[2] = 200;
	sdata.fData[3] = 40;
	p_sensor_algo_api->input(&sdata);
	
	// call algo
	p_sensor_algo_api->process(sdata.id);
}

static void hr_spo2_handler(uint8_t spo2_val, uint8_t spo2_lvl_val, uint8_t hb_val,
				uint8_t hb_lvl_val, uint16_t rr_val[4], uint8_t rr_lvl_val, uint8_t rr_cnt, uint16_t spo2_r_val)
{
	// TODO
}

void algo_init(algo_cb_t cb)
{
	usr_algo_cb = cb;
	
	// init algo
	p_sensor_algo_api->init(&os_api);
	
	// init fifo
	if (p_sensor_algo_api->input_fifo_init) {
		p_sensor_algo_api->input_fifo_init(20);
	}

	// init heart-rate algo
	p_hr_algo_api->init(&hr_os_api);

#if CONFIG_SENSOR_ALGO_HR_VCARE
    /* 32K counter for vcare hr algo */
    sensor_hal_config_tm(ID_HR, 0);
#endif
}

void algo_handler(int id, sensor_dat_t *dat)
{
#ifndef CONFIG_SENSOR_ALGO_MOTION_CYWEE_DML
	int idx;
	uint64_t ts, delta;
#endif // !CONFIG_SENSOR_ALGO_MOTION_CYWEE_DML

	float val[3];
	sensor_raw_t sdata;
	int ret;

	SYS_LOG_DBG("sensor algo_handler id=%d, evt=%d", id, dat->evt);

	// input data
	switch(id) {
		case ID_ACC:
		case ID_MAG:
#ifndef CONFIG_SENSOR_ALGO_MOTION_CYWEE_DML
			// start fifo
			if (dat->cnt > 1) {
				ts = dat->ts * 1000000ull;
				delta = dat->pd * 1000000ull;
				if (p_sensor_algo_api->input_fifo_start) {
					p_sensor_algo_api->input_fifo_start(id, ts, delta);
				}
			}
			// input data
			for (idx = 0; idx < dat->cnt; idx ++) {
				// get value
				sensor_hal_get_value(id, dat, idx, val);
				SYS_LOG_DBG("%s %.2f %.2f %.2f", 
					sensor_hal_get_name(id), val[0], val[1], val[2]);

				// input acc data
				memset(&sdata, 0, sizeof(sensor_raw_t));
				sdata.id = id;
				memcpy(sdata.fData, val, sizeof(float) *3);
				p_sensor_algo_api->input(&sdata);
			}
			// end fifo
			if (dat->cnt > 1) {
				if (p_sensor_algo_api->input_fifo_end) {
					p_sensor_algo_api->input_fifo_end(id);
				}
			}
#endif // !CONFIG_SENSOR_ALGO_MOTION_CYWEE_DML		
			// call algo
			p_sensor_algo_api->process(id);
			break;
			
		case ID_BARO:
			if (dat->cnt == 0) {
				ret = sensor_hal_poll_data(id, dat, NULL);
				if (ret) {
					SYS_LOG_ERR("sensor %d poll failed!", id);
					break;
				}
			}
			// get value
			sensor_hal_get_value(id, dat, 0, val);
			SYS_LOG_DBG("%s %0.2f %0.2f\r\n", sensor_hal_get_name(id), val[0], val[1]);

			// input baro data
			memset(&sdata, 0, sizeof(sdata));
			sdata.id = IN_BARO;
			sdata.fData[0] = val[0];
			sdata.fData[1] = 1.0f;
			sdata.fData[2] = val[1];
			p_sensor_algo_api->input(&sdata);
			// call algo
			p_sensor_algo_api->process(id);
			break;
		
		case ID_HR:
			// call algo
			p_hr_algo_api->process();
			break;
	}
}

