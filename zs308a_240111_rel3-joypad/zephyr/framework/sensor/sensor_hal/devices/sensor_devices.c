/*******************************************************************************
 * @file    sensor_devices.c
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2022-08-03
 * @brief   sensor devices module
*******************************************************************************/

/******************************************************************************/
//includes
/******************************************************************************/
#include <sensor_devices.h>
#include <sensor_acc_lis2dw12.h>
#include <sensor_acc_stk8321.h>
#include <sensor_acc_sc7a20.h>
#include <sensor_mag_mmc5603nj.h>
#include <sensor_baro_icp10125.h>
#include <sensor_hr_gh3011.h>
#include <sensor_hr_vc9201.h>
#include <sensor_hr_hx3605.h>
#include <sensor_hr_hx3690.h>

/******************************************************************************/
//sensor device
/******************************************************************************/
const sensor_dev_t sensor_dev[] = 
{
	{ /* ACC */
#if CONFIG_SENSOR_ACC_LIS2DW12
		SENSOR_DEV_ACC_LIS2DW12,
#elif CONFIG_SENSOR_ACC_STK8321
		SENSOR_DEV_ACC_STK8321,
#elif CONFIG_SENSOR_ACC_SC7A20
		SENSOR_DEV_ACC_SC7A20,
#else
		SENSOR_DEV_NULL,
#endif
	},

	{ /* GYRO */
		SENSOR_DEV_NULL,
	},

	{ /* MAG */
#if CONFIG_SENSOR_MAG_MMC5603NJ
		SENSOR_DEV_MAG_MMC5603NJ,
#else
		SENSOR_DEV_NULL,
#endif
	},

	{ /* BARO */
#if CONFIG_SENSOR_BARO_ICP10125
		SENSOR_DEV_BARO_ICP10125,
#else
		SENSOR_DEV_NULL,
#endif
	},

	{ /* TEMP */
		SENSOR_DEV_NULL,
	},

	{ /* HR */
#if CONFIG_SENSOR_HR_GH3011
		SENSOR_DEV_HR_GH3011,
#elif CONFIG_SENSOR_HR_VC9201
		SENSOR_DEV_HR_VC9201,
#elif CONFIG_SENSOR_HR_HX3605
		SENSOR_DEV_HR_HX3605,
#elif CONFIG_SENSOR_HR_HX3690
		SENSOR_DEV_HR_HX3690,
#else
		SENSOR_DEV_NULL,
#endif
	},

	{ /* GNSS */
		SENSOR_DEV_NULL,
	},

	{ /* OFFBODY */
		SENSOR_DEV_NULL,
	},
};

