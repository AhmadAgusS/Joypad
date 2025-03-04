#ifndef _hx3690l_agc_H_
#define _hx3690l_agc_H_
#include <stdint.h>
#include <stdbool.h>
#include "hx3690l.h"
#define CAL_DELAY_COUNT (8)

#define AGC_OFFSET_STEP    2
#define AGC_LDR_STEP       4


#define CAL_FLG_LED_DR     0x01
#define CAL_FLG_LED_DAC    0x02 
#define CAL_FLG_AMB_DAC    0x04
#define CAL_FLG_RF         0x08

#define HRV_CAL_INIT_LED 64
#define HRV_OFFSET_IDAC 64

#define NO_TOUCH_CHECK_NUM 2     //


typedef enum {    
   
    hrvCalStart, 
    hrvCalLed,
    hrvCalLed2,
    hrvCalLed3,
    hrvCalRf,
    hrvCalRfEnd,
    hrvCalFinish, 
}HRV_STATE_T;



void Init_hrv_PPG_Calibration_Routine(HRV_CAL_SET_T *calR,uint8_t led);
void Restart_hrv_PPG_Calibration_Routine(HRV_CAL_SET_T *calR);
void PPG_hrv_Calibration_Routine(HRV_CAL_SET_T *calR, int32_t led, int32_t amb);


void hx3690l_hrv_cal_init(void);
void hx3690l_hrv_cal_off(uint8_t enable_50_hz);
uint8_t hx3690l_read_fifo_size(void);
void hx3690l_read_fifo_data(uint8_t read_fifo_size,int32_t *buf);
void read_hrv_data_packet(int32_t *buf);
void read_hrv_ir_packet(int32_t *buf);

void hx3690l_hrv_low_power(void);
void hx3690l_hrv_normal_power(void);
void hx3690l_hrv_updata_reg(void);
void hx3690l_hrv_set_mode(uint8_t mode_cmd);
SENSOR_ERROR_T hx3690l_hrv_enable(void);
void hx3690l_hrv_disable(void);
hx3690_hrv_wear_msg_code_t hx3690_hrv_get_wear_status(void);
uint8_t hx3690l_hrv_read(hrv_sensor_data_t * s_dat);
bool hx3690l_hrv_init(void);

hx3690_hrv_wear_msg_code_t hx3690_hrv_wear_mode_check(WORK_MODE_T mode,int32_t infrared_data);
HRV_CAL_SET_T get_hrv_agc_status(void);

#endif
