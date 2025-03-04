#ifndef _hx3690l_agc_H_
#define _hx3690l_agc_H_
#include <stdint.h>
#include <stdbool.h>

#define CAL_DELAY_COUNT (8)

#define AGC_OFFSET_STEP    2
#define AGC_LDR_STEP       4


#define CAL_FLG_LED_DR     0x01
#define CAL_FLG_LED_DAC    0x02 
#define CAL_FLG_AMB_DAC    0x04
#define CAL_FLG_RF         0x08

#define HRS_CAL_INIT_LED 64
#define HRS_OFFSET_IDAC 64

#define NO_TOUCH_CHECK_NUM 2     //


typedef enum {    
   
    hrsCalStart, 
    hrsCalLed,
    hrsCalLed2,
    hrsCalLed3,
    hrsCalRf,
    hrsCalRfEnd,
    hrsCalFinish, 
}HRS_STATE_T;



void Init_hrs_PPG_Calibration_Routine(HRS_CAL_SET_T *calR,uint8_t led);
void Restart_hrs_PPG_Calibration_Routine(HRS_CAL_SET_T *calR);
void PPG_hrs_Calibration_Routine(HRS_CAL_SET_T *calR, int32_t led, int32_t amb);


void hx3690l_hrs_cal_init(void);
void hx3690l_hrs_cal_off(uint8_t enable_50_hz);
uint8_t hx3690l_read_fifo_size(void);
void hx3690l_read_fifo_data(uint8_t read_fifo_size,int32_t *buf);
void read_hrs_data_packet(int32_t *buf);
void read_hrs_ir_packet(int32_t *buf);

void hx3690l_hrs_low_power(void);
void hx3690l_hrs_normal_power(void);
void hx3690l_hrs_updata_reg(void);
void hx3690l_hrs_set_mode(uint8_t mode_cmd);
SENSOR_ERROR_T hx3690l_hrs_enable(void);
void hx3690l_hrs_disable(void);
hx3690_hrs_wear_msg_code_t hx3690_hrs_get_wear_status(void);
uint8_t hx3690l_hrs_read(hrs_sensor_data_t * s_dat);
bool hx3690l_hrs_init(void);

hx3690_hrs_wear_msg_code_t hx3690_hrs_wear_mode_check(WORK_MODE_T mode,int32_t infrared_data);
HRS_CAL_SET_T get_hrs_agc_status(void);

#endif
