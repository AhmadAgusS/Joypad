/*******************************************************************************
 * @file    sensor_algo.c
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2021-5-25
 * @brief   sensor algorithm api
*******************************************************************************/

/******************************************************************************/
//includes
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sensor_algo.h>
#include <sensor_algo_data.h>
#include <OsApi.h>
#include <SL_Watch_Pedo_Kcal_Wrist_Sleep_Sway_Algorithm.h>
#include <SL_Watch_Pedo_Kcal_Wrist_Sleep_Sway_Application.h>

/******************************************************************************/
//constants
/******************************************************************************/
#define DBG(...)			os_api.dbgOutput(__VA_ARGS__)
//#define DBG(...)			do {} while (0)

#define PI						(3.14159265f)

/******************************************************************************/
//variables
/******************************************************************************/

/* CWM os api */
static sensor_os_api_t sensor_os_api = {0};
static OsAPI os_api = {0};

/* event handler */
sensor_handler_t user_handler = NULL;

/******************************************************************************/
//functions
/******************************************************************************/

/* Init sensor algo */
int sensor_algo_init(const sensor_os_api_t *api)
{
	// init data section
	memcpy(_data_ram_start, _data_rom_start, (_data_rom_end - _data_rom_start));
	
	// init bss section
	memset(_bss_start, 0, (_bss_end - _bss_start));
	
	// init heap section
	memset(_heap_start, 0, (_heap_end - _heap_start));
	
	// config os api
	sensor_algo_set_os_api(api);
	
//	DBG("sensor_algo_init\r\n");
	SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_SWAY_INIT();
	
	return 0;
}

/* Get os api */
sensor_os_api_t* sensor_algo_get_os_api(void)
{
	return &sensor_os_api;
}

/* Set os api */
int sensor_algo_set_os_api(const sensor_os_api_t *api)
{
	// config os api
	sensor_os_api = *api;
	os_api.malloc = malloc;
	os_api.free = free;
	os_api.GetTimeNs = api->get_timestamp_ns;
	os_api.dbgOutput = api->dbgOutput;
	user_handler = api->user_handler;
	
	return 0;
}

/* Read sensor data and output through sensor call-back function */
int sensor_algo_process(uint32_t id)
{
	unsigned int tmp;
	// DBG("sensor_algo_process\r\n");
	tmp = SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_SWAY_ALGO();	
//	DBG("sensor_algo_process %d\r\n",tmp);
	return 0;
}
/* Read sensor algorithm result */
sensor_res_t* sensor_algo_get_result(void)
{
//	DBG("sensor_algo_get_result %d\r\n",sensor_algo_res.handup);
	return &sensor_algo_res;
}
