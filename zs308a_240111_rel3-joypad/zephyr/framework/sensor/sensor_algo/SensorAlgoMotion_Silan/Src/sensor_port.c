/*******************************************************************************
 * @file    sensor_port.c
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2022-7-21
 * @brief   sensor port api
*******************************************************************************/
#include <stdio.h>
#include <sensor_algo.h>
#include <sensor_algo_data.h>
#include <OsApi.h>
#include <SL_Watch_Pedo_Kcal_Wrist_Sleep_Sway_Algorithm.h>

unsigned char SL_SC7A20_I2c_Spi_Write(bool sl_spi_iic,unsigned char reg, unsigned char dat)
{
    sensor_os_api_t *os_api = sensor_algo_get_os_api();
    int ret = -1;
    
    if (os_api->i2c_write != NULL) {
		ret = os_api->i2c_write(0, reg, &dat, 1);
    }
    return ret;
}

unsigned char SL_SC7A20_I2c_Spi_Read(bool sl_spi_iic,unsigned char reg, unsigned char len, unsigned char *buf)
{
    sensor_os_api_t *os_api = sensor_algo_get_os_api();
    int ret = -1;

    if (os_api->i2c_read != NULL) {
		ret = os_api->i2c_read(0, reg, buf, len);
	}
    return ret;
}

