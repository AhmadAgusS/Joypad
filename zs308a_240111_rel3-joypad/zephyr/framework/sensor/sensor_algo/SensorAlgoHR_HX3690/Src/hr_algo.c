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
#include <hx3690l.h>

/******************************************************************************/
//constants
/******************************************************************************/
#define DBG(...)			if (os_api.dbgOutput) os_api.dbgOutput(__VA_ARGS__)
//#define DBG(...)			do {} while (0)

/******************************************************************************/
//variables
/******************************************************************************/
/* hr os api */
static hr_os_api_t os_api = {0};

extern bool timer_40ms_en;
extern bool timer_320ms_en;
extern uint8_t timer_320ms_cnt;


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
	hx3690l_init(HRS_MODE);
	os_api.dbgOutput("hx3690_start %d \r\n",HRS_MODE);
	return 0;
}

/* Stop hr sensor */
int hr_algo_stop(void)
{
	hx3690l_ppg_off();
	return 0;
}

/* Process data through call-back handler */
int hr_algo_process(void)
{
	if(timer_40ms_en)
		gsen_read_timeout_handler(0);
	if(timer_320ms_en){
		timer_320ms_cnt ++;
		if(timer_320ms_cnt == 8){
			timer_320ms_cnt = 0;
			heart_rate_meas_timeout_handler(0);
		}
	}
	return 0;
}
