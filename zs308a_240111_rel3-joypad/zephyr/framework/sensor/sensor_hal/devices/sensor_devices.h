/*******************************************************************************
 * @file    sensor_devices.h
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2022-08-03
 * @brief   sensor devices module
*******************************************************************************/

#ifndef _SENSOR_DEVICES_H
#define _SENSOR_DEVICES_H

/******************************************************************************/
//includes
/******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <sensor_dev.h>
#include <sensor_bus.h>
#include <drivers/i2cmt.h>
#include <drivers/spimt.h>

/******************************************************************************/
//constants
/******************************************************************************/
#define SENSOR_DEV_NULL		{ 0 }, { 0 }, { 0 }, { 0 }, NULL

/* ACC Sensor GPIO Config */
#define ACC_POWER			CONFIG_SENSOR_ACC_POWER_GPIO
#define ACC_RESET			CONFIG_SENSOR_ACC_RESET_GPIO
#define ACC_ISR				CONFIG_SENSOR_ACC_ISR_GPIO
#define ACC_TRIG			CONFIG_SENSOR_ACC_TRIG_IO

/* Heart-rate Sensor GPIO Config */
#define HR_POWER			CONFIG_SENSOR_HR_POWER_GPIO
#define HR_RESET			CONFIG_SENSOR_HR_RESET_GPIO
#define HR_ISR				CONFIG_SENSOR_HR_ISR_GPIO
#define HR_TRIG				CONFIG_SENSOR_HR_TRIG_IO

/* Magnet Sensor GPIO Config */
#define MAG_POWER			CONFIG_SENSOR_MAG_POWER_GPIO
#define MAG_RESET			CONFIG_SENSOR_MAG_RESET_GPIO
#define MAG_ISR				CONFIG_SENSOR_MAG_ISR_GPIO
#define MAG_TRIG			CONFIG_SENSOR_MAG_TRIG_IO

/* Baro Sensor GPIO Config */
#define BARO_POWER			CONFIG_SENSOR_BARO_POWER_GPIO
#define BARO_RESET			CONFIG_SENSOR_BARO_RESET_GPIO
#define BARO_ISR			CONFIG_SENSOR_BARO_ISR_GPIO
#define BARO_TRIG			CONFIG_SENSOR_BARO_TRIG_IO

/******************************************************************************/
//sensor device
/******************************************************************************/
extern const sensor_dev_t sensor_dev[];

#endif  /* _SENSOR_DEVICES_H */
