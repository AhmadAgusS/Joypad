/*******************************************************************************
 * @file    sensor_port.c
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
#include <drivers/ipmsg.h>
#include <rbuf/rbuf_msg_sc.h>
#include "sensor_port.h"
#if SENSOR_CPU
#include <sc_code.h>
#endif

/******************************************************************************/
//constants
/******************************************************************************/

/******************************************************************************/
//variables
/******************************************************************************/
static volatile sensor_dat_t sensor_dat[NUM_SENSOR] = { 0 };

/******************************************************************************/
//functions
/******************************************************************************/
#if SENSOR_CPU
static void sc_msg_handler(void *context, void *arg)
{
	rbuf_msg_t *msg = (rbuf_msg_t*)arg;
	
	/* dispatch msg */
	switch(msg->type) {
		case MSG_SC_INIT:
			/* print major and minor */
			printk("\r\nSensorHub v%d.%d\r\n\r\n", msg->data.h[0], msg->data.h[1]);
			break;
	}
}

static void sc_boot(void)
{
	int ret;
	struct device *dev;
	
	/* get device */
	dev = (struct device*)device_get_binding("SC");
	if (!dev) {
		printk("[SC] get device failed!\n");
		return;
	}

	/* load bin */
	ret = ipmsg_load(dev, (void*)sc_code, sizeof(sc_code));
	if (ret) {
		printk("[SC] load bin failed!\n");
		return;
	}

	/* start sensor cpu */
	ret = ipmsg_start(dev, NULL, NULL);
	if (ret) {
		printk("[SC] start failed!\n");
		return;
	}
	
	/* register isr callback */
	ipmsg_register_callback(dev, sc_msg_handler, NULL);
}
#endif

static void sensor_task_callback(int id, sensor_dat_t *dat, void *ctx)
{
	// check data overlay
	if (sensor_dat[id].evt != EVT_NULL) {
		printk("\r\n[%d] overlay!\r\n", id);
	}
	
	// save data
	sensor_dat[id] = *dat;
}

static int sensor_dump_data(int id, sensor_dat_t *dat)
{
	int idx, size;
	float val[8];
	
	for (idx = 0; idx < dat->cnt; idx ++) {
		// get value
		size = sensor_hal_get_value(id, dat, idx, val);
		if (size <= 0) {
			continue;
		}
		
		// show type
		switch (dat->evt) {
			case EVT_TASK:
				printk("T");
				break;
			
			case EVT_IRQ:
				printk("I");
				break;
		}
		
		// show value
		switch(id) {
			case ID_ACC:
				printk("[%d] ACC %.2f %.2f %.2f\r\n", dat->ts, val[0], val[1], val[2]);
				break;
			
			case ID_MAG:
				printk("[%d] MAG %.2f %.2f %.2f\r\n", dat->ts, val[0], val[1], val[2]);
				break;
			
			case ID_BARO:
				printk("[%d] BARO P=%.2f T=%.2f\r\n", dat->ts, val[0], val[1]);
				break;
			
			case ID_HR:
				printk("[%d] HR HRS1=%.2f ALS1=%.2f PS1=%.2f ALS2=%.2f\r\n", 
								dat->ts, val[0], val[1], val[2], val[3]);
				break;
			
			case ID_GYRO:
				printk("[%d] GYRO T=%.2f G=%.2f %.2f %.2f A=%.2f %.2f %.2f\r\n", 
								dat->ts, val[0], val[1], val[2], val[3], val[4], val[5], val[6]);
				break;
		}
	}
	
	return 0;
}

int sensor_init(void)
{
	int ret;
	
#if SENSOR_CPU
	// boot sensor cpu
	sc_boot();
#endif
	
	/* init sensor */
	ret = sensor_hal_init();
	if(ret != 0) {
		printk("sensor init failed!\r\n");
	}
	
	/* add callback */
	sensor_hal_add_callback(ID_ACC, sensor_task_callback, NULL);
	sensor_hal_add_callback(ID_MAG, sensor_task_callback, NULL);
	sensor_hal_add_callback(ID_BARO, sensor_task_callback, NULL);
	sensor_hal_add_callback(ID_HR, sensor_task_callback, NULL);
	sensor_hal_add_callback(ID_GYRO, sensor_task_callback, NULL);
	
	/* enable sensor */	
	sensor_hal_enable(ID_ACC);
	sensor_hal_enable(ID_MAG);
	sensor_hal_enable(ID_BARO);
	sensor_hal_enable(ID_HR);
	sensor_hal_enable(ID_GYRO);
	
	/* dump sensor */	
	sensor_hal_dump();
	
	return ret;
}

int sensor_poll(void)
{
	int id, size;
	uint8_t buf[16];
	sensor_dat_t dat;
	
	// process sensor event
	for (id = 0; id < NUM_SENSOR; id ++) {
		if (sensor_dat[id].evt != EVT_NULL) {
			// copy and clear evt
			dat = sensor_dat[id];
			sensor_dat[id].evt = EVT_NULL;
			
			// dispatch event
			switch(dat.evt) {
				// process task event
				case EVT_TASK:
					// dump data
					sensor_dump_data(id, &dat);
					break;
				
				// process irq event
				case EVT_IRQ:
					// poll sensor data
					size = sensor_hal_poll_data(id, &dat, buf);
					if (size > 0) {
						// dump data
						sensor_dump_data(id, &dat);
					}
					break;
			}
		}
	}
	
	return 0;
}

int sensor_i2c_write(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
	return sensor_hal_write(ID_HR, reg, buf, len);
}

int sensor_i2c_read(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{	
	return sensor_hal_read(ID_HR, reg, buf, len);
}
