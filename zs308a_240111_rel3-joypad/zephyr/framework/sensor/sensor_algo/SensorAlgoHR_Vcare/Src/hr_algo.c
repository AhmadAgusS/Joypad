/*******************************************************************************
 * @file    hr_algo.c
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
#include <hr_algo.h>
#include <hr_algo_data.h>
#include "vcHr02Hci.h"

/******************************************************************************/
//constants
/******************************************************************************/
#define DBG(...)			os_api.dbgOutput(__VA_ARGS__)
//#define DBG(...)			do {} while (0)

/******************************************************************************/
//variables
/******************************************************************************/
/* hr os api */
static hr_os_api_t os_api = {0};

/* Heart rate data struct */
extern vcHr02_t vcHr02;

/* Heart rate mode */
extern vcHr02Mode_t vcMode;

/* Sport Mode In Heart Rate Mode */
extern AlgoSportMode_t vcSportMode;

extern bool vcHr02IRQFlag;

extern int HeartRateValue;
extern int real_spo;

/******************************************************************************/
//functions
/******************************************************************************/
/* Init sensor algo */
int hr_algo_init(const hr_os_api_t *api)
{
	// init data section
	memcpy(_data_ram_start, _data_rom_start, (_data_rom_end - _data_rom_start));
	
	// init bss section
	memset(_bss_start, 0, (_bss_end - _bss_start));
	
	// init os api
	os_api = *api;
	
	
	DBG("Vcare hr_algo_init\n");
	return 0;
}

/* Get os api */
hr_os_api_t* hr_algo_get_os_api(void)
{
	return (hr_os_api_t*)&os_api;
}

/* Set os api */
int hr_algo_set_os_api(const hr_os_api_t *api)
{
	os_api = *api;
	return 0;
}

/* Start hr sensor */
int hr_algo_start(int mode)
{
	vcHr02Init(&vcHr02,vcMode);
	return 0;
}

/* Stop hr sensor */
int hr_algo_stop(void)
{
	vcHr02StopSample(&vcHr02);
	return 0;
}

/* Process data through call-back handler */
int hr_algo_process(void)
{
	vcHr02IRQFlag = true;
	vcHr02_process(vcSportMode);
	os_api.hb_handler(HeartRateValue, real_spo, 0);
	DBG("HeartRateValue:%d real_spo:%d\r\n", HeartRateValue,real_spo);
	return 0;
}
