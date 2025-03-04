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
#include <sensor_hal.h>
#include <sensor_algo.h>
#include <hr_algo.h>
#include "sensor_port.h"

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

const static sensor_os_api_t os_api =
{
	.dbgOutput = printf,
	.get_timestamp_ns = usr_get_timestamp,
	.user_handler = usr_evt_handler,
	.i2c_write = sensor_i2c_write,
	.i2c_read = sensor_i2c_read,
};

const static hr_os_api_t hr_os_api =
{
	.dbgOutput = printf,
	.i2c_write = sensor_i2c_write,
	.i2c_read = sensor_i2c_read,
	.wear_handler = hr_wear_handler,
	.hb_handler = hr_hb_handler,
	.spo2_handler = hr_spo2_handler,
	.hrv_handler = NULL,
};

/******************************************************************************/
//functions
/******************************************************************************/
static uint64_t usr_get_timestamp(void)
{
    return (uint64_t)k_uptime_get_32() * 1000000;
}

static void usr_evt_handler(sensor_evt_t *evt)
{
	static int orientation = 0;
	static int heart_rate = 0;
	sensor_res_t *res = p_sensor_algo_api->get_result();
	
	// dump event
//	p_sensor_algo_api->dump_event(evt);
	
	// dump result
	switch(evt->id) {
		case REQ_SENSOR:
			if ((int)evt->fData[2]) {
				sensor_hal_enable((int)evt->fData[1]);
			} else {
				sensor_hal_disable((int)evt->fData[1]);
			}
			break;
			
		case ALGO_ACTIVITY_OUTPUT:
			p_sensor_algo_api->dump_result(res);
			break;
		
		case RAW_MAG:
			if (orientation != (int)res->orientation) {
				orientation = (int)res->orientation;
				printk("orientation=%d\r\n", orientation);
			}
			break;
		
		case RAW_HEARTRATE:
			if (heart_rate != (int)res->heart_rate) {
				heart_rate = (int)res->heart_rate;
				printk("heart_rate=%d\r\n", heart_rate);
			}
			break;
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

void algo_init(void)
{
	// init algo
	p_sensor_algo_api->init(&os_api);
	
	// init fifo
	p_sensor_algo_api->input_fifo_init(20);
	
	// enable sensor
	p_sensor_algo_api->enable(ALGO_ACTIVITY_OUTPUT, 0);
	p_sensor_algo_api->enable(IN_MAG, 0);
	p_sensor_algo_api->enable(IN_HEARTRATE, 0);
	p_sensor_algo_api->enable(ALGO_HANDUP, 0);
	
	// init and start heart-rate sensor
	p_hr_algo_api->init(&hr_os_api);
	p_hr_algo_api->start(HB);
}

void algo_handler(int id, sensor_dat_t *dat)
{
	int idx;
	uint64_t ts, delta;
	float val[3];
	sensor_raw_t sdata;

	// input data
	switch(id) {
		case ID_ACC:
		case ID_MAG:
			// start fifo
			if (dat->cnt > 1) {
				ts = dat->ts * 1000000;
				delta = dat->pd * 1000000;
				p_sensor_algo_api->input_fifo_start(id, ts, delta);
			}
			// input data
			for (idx = 0; idx < dat->cnt; idx ++) {
				// get value
				sensor_hal_get_value(id, dat, idx, val);
				// input acc data
				memset(&sdata, 0, sizeof(sensor_raw_t));
				sdata.id = id;
				memcpy(sdata.fData, val, sizeof(float) *3);
				p_sensor_algo_api->input(&sdata);
			}
			// end fifo
			if (dat->cnt > 1) {
				p_sensor_algo_api->input_fifo_end(id);
			}
			// call algo
			p_sensor_algo_api->process(id);
			break;
			
		case ID_BARO:
			// get value
			sensor_hal_get_value(id, dat, 0, val);
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
