/*******************************************************************************
 * @file    sensor_port.h
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2020-08-12
 * @brief   sensor testing module
*******************************************************************************/

#ifndef _SENSOR_PORT_H
#define _SENSOR_PORT_H

/******************************************************************************/
//includes
/******************************************************************************/
#include <sensor_hal.h>

/******************************************************************************/
//constants
/******************************************************************************/
#ifdef CONFIG_IPMSG_ACTS_SC
#define SENSOR_CPU				(1)  // read or write by sensor cpu
#else
#define SENSOR_CPU				(0)  // read or write by main cpu
#endif

/******************************************************************************/
//variables
/******************************************************************************/

/******************************************************************************/
//functions
/******************************************************************************/
int sensor_init(void);
int sensor_poll(void);

int sensor_i2c_write(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len);
int sensor_i2c_read(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len);

#endif  /* _SENSOR_PORT_H */

