
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <hr_algo_data.h>

//#include "twi_master.h"
#include "hx3690l.h"
#include "hx3690l_hrs_agc.h"
//#include "hx3690l_hrs_alg.h"
//#include "SEGGER_RTT.h"

#ifdef HRS_ALG_LIB

extern const uint8_t  hx3690l_hrs_agc_idac;
extern const uint8_t  green_led_max_init;
extern uint8_t low_vbat_flag;

//HRS_INFRARED_THRES
extern const int32_t  hrs_ir_unwear_thres; 
extern const int32_t  hrs_ir_wear_thres; 

static uint8_t s_ppg_state = 0;
static uint8_t s_cal_state = 0;
//static int32_t s_buf[64] = {0}; 
static int32_t agc_buf[64] = {0};

static uint8_t cal_delay = CAL_DELAY_COUNT;
static HRS_CAL_SET_T  calReg;
//
static hx3690_hrs_wear_msg_code_t hrs_wear_status = MSG_HRS_NO_WEAR;
static hx3690_hrs_wear_msg_code_t hrs_wear_status_pre = MSG_HRS_NO_WEAR;

static uint8_t no_touch_cnt = 0;

void Init_hrs_PPG_Calibration_Routine(HRS_CAL_SET_T *calR,uint8_t led)
{
    calR->flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
    
    calR->LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->AMBDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->RF = 0;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->LED = HRS_CAL_INIT_LED;
    calR->state = hrsCalStart;
    calR->int_cnt = 0;
    calR->cur255_cnt =0;
    if(low_vbat_flag==1)
    {
       calR->led_idac = (hx3690l_hrs_agc_idac*3)>>2;
       calR->led_max_cur = green_led_max_init>>1;
    }
    else
    {
       calR->led_idac = hx3690l_hrs_agc_idac;
       calR->led_max_cur = green_led_max_init;
    }
}

void Restart_hrs_PPG_Calibration_Routine(HRS_CAL_SET_T *calR)
{
    calR->flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
    
    calR->LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->AMBDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->RF = 0;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->LED = HRS_CAL_INIT_LED;
    calR->state = hrsCalStart;
    calR->int_cnt = 0;
    //calR->cur255_cnt =0;
    //calR->led_idac = hx3690l_hrs_agc_idac;
}

hx3690_hrs_wear_msg_code_t hx3690_hrs_wear_mode_check(WORK_MODE_T mode,int32_t infrared_data)
{
    if(infrared_data > hrs_ir_wear_thres)
    {
        if(no_touch_cnt < NO_TOUCH_CHECK_NUM)
        {
            no_touch_cnt++; 
        }  
        if(no_touch_cnt >= NO_TOUCH_CHECK_NUM)  
        {
            hrs_wear_status = MSG_HRS_WEAR;
        }   
    }
    else if(infrared_data < hrs_ir_unwear_thres)
    {
        if(no_touch_cnt>0)
        {
            no_touch_cnt--;
        }
        if(no_touch_cnt == 0)
        {
            hrs_wear_status = MSG_HRS_NO_WEAR;                
        }      
    }
    
   // AGC_LOG("hrs wearstates: hrs_wear_status_pre=%d,hrs_wear_status=%d\r\n",\
            hrs_wear_status_pre,hrs_wear_status);
    if(mode == WEAR_MODE)
    {
        return hrs_wear_status;
    }
    if(hrs_wear_status_pre != hrs_wear_status)
    {
        hrs_wear_status_pre = hrs_wear_status;
        if(hrs_wear_status_pre == MSG_HRS_NO_WEAR)
        {
            hx3690l_hrs_low_power();                 
        }
        else if(hrs_wear_status_pre == MSG_HRS_WEAR)
        {
            hx3690_alg_open_deep();
            //hx3690l_hrs_set_mode(PPG_INIT);
            //hx3690l_hrs_set_mode(CAL_INIT);
        }  
    }
    
    return hrs_wear_status;
}
void PPG_hrs_Calibration_Routine(HRS_CAL_SET_T *calR, int32_t led, int32_t amb)
{
    int32_t dif = 0;
    int32_t led_tmp = 0;
    int32_t step_tmp = 0;
    //AGC_LOG("ppg cali led=%d,amb=%d\r\n",led,amb);
    switch(calR->state)
    {
        case hrsCalStart:
            calR->AMBDAC = amb/2608;
            calR->LEDDAC = calR->AMBDAC+calR->led_idac;   /*green led i_pd 14*0.25=3.5ua*/
            calR->flag = CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC;
            if(led>amb)
            {
                calR->led_step = (led-amb)/HRS_CAL_INIT_LED; 
            }            
            calR->state = hrsCalLed;
            break;
        case hrsCalLed:   
            if(calR->led_step > 0)
            {
                led_tmp = led;
                step_tmp = calR->led_step;
                dif = (led_tmp/step_tmp);
                if((HRS_CAL_INIT_LED-dif)>calR->led_max_cur)
                {
                   calR->LED = calR->led_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                }
                else 
                {
                    calR->LED = HRS_CAL_INIT_LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                                
                } 

            }               
            calR->flag = CAL_FLG_LED_DR;
            calR->state = hrsCalLed2;    
            break;
       case hrsCalLed2:   
            if(calR->led_step > 0)
            {
                led_tmp = led;
                step_tmp = calR->led_step;
                dif = (led_tmp/step_tmp);
                if((calR->LED-dif)>calR->led_max_cur)
                {
                   calR->LED = calR->led_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                }
                else 
                {
                    calR->LED = calR->LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                                
                } 

            }               
            calR->flag = CAL_FLG_LED_DR;
            calR->state = hrsCalLed3;
            break;
        case hrsCalLed3:   
            if(calR->led_step > 0)
            {
                led_tmp = led;
                step_tmp = calR->led_step;
                dif = (led_tmp/step_tmp);

                if((calR->LED-dif)>calR->led_max_cur)
                {
                   calR->LED = calR->led_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                }
                else 
                {
                    calR->LED = calR->LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                                
                } 
            }               
            calR->flag = CAL_FLG_LED_DR;
            calR->state = hrsCalRf; 
            break;
        case hrsCalRf:
            calR->RF = 6; //500K
            calR->flag = CAL_FLG_RF;
            calR->state = hrsCalRfEnd;
            //AGC_LOG("SetRF: agc change idc rf = %d\r\n",calR->RF);
            break;
        case hrsCalRfEnd:
            calR->state = hrsCalFinish;
            //AGC_LOG("AGC END: agc finish\r\n");
            break;
        default:
            
            break;
        
    }
    hr_os_api_t *os_api = hr_algo_get_os_api();
    os_api->dbgOutput("AGC: led_drv=%d,ledDac=%d,ambDac=%d,ledstep=%d,rf=%d,\r\n",\
            calR->LED, calR->LEDDAC, calR->AMBDAC,calR->led_step,calR->RF);
}

HRS_CAL_SET_T PPG_hrs_agc(void)
{
    int32_t led_val, amb_val;
    
    //AGC_LOG("agc  in\r\n");
    calReg.work = false;
    if (!s_cal_state) 
    {
        return  calReg;
    } 
    //AGC_LOG("agc  in\r\n");
#ifdef EXT_INT_AGC    
    calReg.int_cnt ++;
    if(calReg.int_cnt < 8)
    {
         //AGC_LOG("calReg.int_cnt = %d!\r\n",cal_reg->int_cnt);
         return calReg;
    }
    calReg.int_cnt = 0;
#endif   
    calReg.work = true;   
    #if defined(INT_MODE)
    #elif defined(FIFO_ALL_MOST_FULL) 
    #else
    hx3690l_gpioint_cfg(false);
    #endif    
    /*
    s_buf[0] = phase1    //green
    s_buf[1] = phase2    //als
    */
    read_hrs_data_packet(agc_buf);
    led_val = agc_buf[0] - 1047600;
    amb_val = agc_buf[1] - 1047600;
    
    AGC_LOG("cal dat ch1=%d,ch2=%d,led_val=%d,amb_val=%d \r\n",
    agc_buf[0], agc_buf[1], led_val, amb_val);
    
    PPG_hrs_Calibration_Routine(&calReg, led_val, amb_val);
    
    if (calReg.state == hrsCalFinish) {
        hx3690l_hrs_set_mode(CAL_OFF);
    } else {
        hx3690l_hrs_updata_reg();
		#if defined(INT_MODE)
		#elif defined(FIFO_ALL_MOST_FULL) 
		#else
		hx3690l_gpioint_cfg(true);
		#endif 
    }

    return  calReg;
}



void hx3690l_hrs_cal_init(void) // 20200615 ericy afe cali online
{
    #ifdef EXT_INT_AGC 
    uint16_t sample_rate = 200;                      /*config the data rate of chip alps2_fm ,uint is Hz*/
    #else
    uint16_t sample_rate = 200;
    #endif

    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>
    hx3690l_write_reg(0x13,0x30); //FIFO bypass mode enable
    hx3690l_write_reg(0x23,0x20); //phase4 convertion ready enable
    #if defined(FIFO_ALL_MOST_FULL)
    hx3690l_write_reg(0x24,0x00);
    #else
    #endif
  
    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5);             //Delay for reset time
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal
}

void hx3690l_hrs_cal_off(uint8_t enable_50_hz) // 20200615 ericy afe cali offline
{
    uint16_t sample_rate = 25;                       /*config the data rate of chip alps2_fm ,uint is Hz*/
    if (enable_50_hz) 
    {
        sample_rate = 50;
    }

    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>
  #if defined(INT_MODE)
    hx3690l_write_reg(0x13,0x30); 
    hx3690l_write_reg(0x23,0x20); 
  #elif defined(FIFO_ALL_MOST_FULL)
    hx3690l_write_reg(0x13,0x31); //FIFO mode enable
    hx3690l_write_reg(0x23,0x00); //phase3 convertion ready disable	
    hx3690l_write_reg(0x24,0x20); 
  #else
    hx3690l_write_reg(0x13,0x31); //FIFO mode enable
    hx3690l_write_reg(0x23,0x00); //phase3 convertion ready disable	
  #endif

	#ifdef INT_MODE
    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5); 
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal    
  #else
    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5);             //Delay for reset time
    hx3690l_write_reg(0X13, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x31);
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal
  #endif
}

/*
s_buf[0] = phase1 green
s_buf[1] = phase2  als(green)
*/
void read_hrs_data_packet(int32_t *buf) 
{
    uint8_t dataBuf[6];
    
    hx3690l_brust_read_reg(0x03, dataBuf, 3); 
    hx3690l_brust_read_reg(0x0c, dataBuf+3, 3);    
    
    for (uint8_t i=0; i<2; i++) 
    {
        buf[i] = (int32_t)(dataBuf[3*i]|(dataBuf[3*i+1]<<8)|(dataBuf[3*i+2]<<16));
    }
}

void read_hrs_ir_packet(int32_t *buf) // 20200615 ericy read reg_data phase1 and phase3
{
    uint8_t dataBuf[6];
    
    hx3690l_brust_read_reg(0x06, dataBuf, 6);     //phase3(ir) and phase4(als for ir)
    
    for (uint8_t i=0; i<2; i++) 
    {
        buf[i] = (int32_t)(dataBuf[3*i]|(dataBuf[3*i+1]<<8)|(dataBuf[3*i+2]<<16));
    }
}

uint8_t hx3690l_hrs_read_data_packet(int32_t *s_buf) // 20200615 ericy read reg_data phase1 and phase3 
{
    uint8_t  databuf1[6] = {0};
		uint8_t  databuf2[6] = {0};
    uint32_t P1 = 0,P2 = 0 ,P3 = 0 ,P4 =0 ;
    //AGC_LOG("sppg = %d,  scal = %d\r\n", s_ppg_state, s_cal_state); 
    if (!s_ppg_state || s_cal_state) 
    {
        return NULL;
    } 
    hx3690l_brust_read_reg(0x03, databuf1, 6); 
    hx3690l_brust_read_reg(0x09, databuf2, 6); 		
		
    P1 = ((databuf1[0])|(databuf1[1]<<8)|(databuf1[2]<<16)); 
		P3 = ((databuf1[3])|(databuf1[4]<<8)|(databuf1[5]<<16)); 
		P4 = ((databuf2[0])|(databuf2[1]<<8)|(databuf2[2]<<16));	
    P2 = ((databuf2[3])|(databuf2[4]<<8)|(databuf2[5]<<16));
 
    s_buf[0] = P1;
		s_buf[1] = P2;
		s_buf[2] = P3;
		s_buf[3] = P4;
    s_buf[4] = calReg.LED;
    
    uint8_t recal = 0;
    if (s_buf[0]<523800 || s_buf[0]>1571400)
    {
        recal = true;
       
        if(hrs_wear_status==MSG_HRS_NO_WEAR)
        {
             recal = false;
        }                
    } 
    uint32_t ir_data = 0;
    if(s_buf[2]>s_buf[3])
    {
      ir_data = s_buf[2]-s_buf[3];
    }
    else
    {
      ir_data = 0;
    }  
    hx3690_hrs_wear_mode_check(HRS_MODE,s_buf[2]-s_buf[3]);
    if (recal) 
    {
        cal_delay--;
        if (cal_delay <= 0) 
        {
            if(calReg.LED > ((calReg.led_max_cur*7)>>3))
            {
                calReg.cur255_cnt++;
                if(calReg.cur255_cnt>0)
                {
                    calReg.cur255_cnt = 0;
                    calReg.led_idac = ((uint16_t)calReg.led_idac*3)/4;
                }
            }
            cal_delay = CAL_DELAY_COUNT;
            hx3690l_hrs_set_mode(RECAL_INIT);
        }
    }
    else                       
    {
        cal_delay = CAL_DELAY_COUNT;
    }
    return 1;
}




void hx3690l_hrs_low_power(void)
{   
    uint16_t sample_rate = 5;                       /*config the data rate of chip alps2_fm ,uint is Hz*/
    uint8_t data_avg_num = 0;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    
    uint8_t phase1_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0; 
    uint8_t phase1_led_en = 0;
    
    uint8_t phase2_led_en = 0;
    uint8_t phase3_led_en = 1;
    uint8_t phase4_led_en = 0;
    
    uint8_t phase3_tia_res = 0; 
    uint8_t phase4_tia_res = 0; 
    
    uint8_t phase3_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    
    uint8_t led_on_time = 1;      /* 0 = 32clk=8us ; 1 = 64clk=16us; 2=128clk=32us ; 3 = 256clk=64us ;
                                     4 = 512clk=128us ; 5 = 1024clk=256us; 6= 2048clk=512us; 7 = 4096clk=1024us */
    
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );	
    
    hx3690l_write_reg(0X2d, phase3_tia_res);
    hx3690l_write_reg(0X2e, phase4_tia_res);

    hx3690l_write_reg(0X2c, phase1_tia_res); 
    hx3690l_write_reg(0X38, phase1_offset_idac);
    hx3690l_write_reg(0X30, phase1_ldr_cur);
    
    hx3690l_write_reg(0X31, phase3_ldr_cur);
    
    hx3690l_write_reg(0X1f, (led_on_time<<4| phase1_led_en<<3 | phase3_led_en<<2 | phase4_led_en<<1 | phase2_led_en) );
    
    #ifdef INT_MODE
    hx3690l_write_reg(0x13,0x30); //FIFO bypass mode enable
    hx3690l_write_reg(0x23,0x20); //phase3 convertion ready disable
    #else
    hx3690l_write_reg(0x13,0x31); //FIFO bypass mode enable
    hx3690l_write_reg(0x23,0x00); //phase3 convertion ready disable
    #endif

    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5);             //Delay for reset time
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal
    
    calReg.LED =  phase1_ldr_cur;

    AGC_LOG(" chip go to low power mode  \r\n" );  

//yorke close 40m timer    
}

void hx3690l_hrs_normal_power(void)
{     
    calReg.flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
    
    calReg.LED = 100;     // phase1 led driver config   
    calReg.LEDDAC = 64;  // phase1 offset idac cfg
    calReg.AMBDAC = 64;  // phase3 offset idac cfg
    calReg.RF = 5;    // phase tia feed back resister cfg

    
    hx3690l_hrs_updata_reg();

    AGC_LOG(" chip go to normal mode  \r\n" );   
}

void hx3690l_hrs_updata_reg(void)
{
    if (calReg.flag & CAL_FLG_LED_DR) 
    {
        hx3690l_write_reg(0X30, calReg.LED);     // phase1 led driver config
    }
    
    if (calReg.flag & CAL_FLG_LED_DAC) 
    {
        hx3690l_write_reg(0X38, calReg.LEDDAC);  // phase1 offset idac cfg
    }
    
    if (calReg.flag & CAL_FLG_AMB_DAC) 
    {
        hx3690l_write_reg(0X3b, calReg.AMBDAC);  // phase2 offset idac cfg
    }
    
    if (calReg.flag & CAL_FLG_RF) 
    {
        hx3690l_write_reg(0X2c, calReg.RF);    // phase1 tia feed back resister cfg
        //hx3690l_write_reg(0X2d, calReg.RF);    // phase3 tia feed back resister cfg 
        //hx3690l_write_reg(0X2e, calReg.RF);    // phase4 tia feed back resister cfg
        hx3690l_write_reg(0X2f, calReg.RF);    // phase2 tia feed back resister cfg
    }
}



void hx3690l_hrs_set_mode(uint8_t mode_cmd)
{
    switch (mode_cmd) 
    {    
        case PPG_INIT:
            hx3690l_hrs_ppg_init();
            #if defined(INT_MODE)
            hx3690l_gpioint_cfg(true);
            #elif defined(FIFO_ALL_MOST_FULL)
            hx3690l_gpioint_cfg(true);
            #else
            hx3690l_320ms_timer_cfg(true);
            hx3690l_40ms_timer_cfg(true);
            #endif
            s_ppg_state = 1;

            AGC_LOG("ppg init mode\r\n");
            break;

        case PPG_OFF:
            hx3690l_ppg_off();
            s_ppg_state = 0;
            AGC_LOG("ppg off mode\r\n");
            break;
        case PPG_LED_OFF:
            hx3690l_hrs_low_power();
            s_ppg_state = 0;
            AGC_LOG("ppg led off mode\r\n");
            break;

        case CAL_INIT:
            Init_hrs_PPG_Calibration_Routine(&calReg, 64);
            hx3690l_hrs_cal_init();
            hx3690l_hrs_updata_reg();
            #if defined(INT_MODE)
            #elif defined(FIFO_ALL_MOST_FULL)
            #else
            hx3690l_gpioint_cfg(true);
            #endif
            s_cal_state = 1;
            AGC_LOG("cal init mode\r\n");   
            break;         
        case RECAL_INIT:        
            Restart_hrs_PPG_Calibration_Routine(&calReg);
            hx3690l_hrs_cal_init();
            hx3690l_hrs_updata_reg();
            #if defined(INT_MODE)
            #elif defined(FIFO_ALL_MOST_FULL)
            #else
            hx3690l_gpioint_cfg(true);
            #endif
            s_cal_state = 1;
            AGC_LOG("Recal init mode\r\n");
            break;

        case CAL_OFF:
            #if defined(INT_MODE)
            #elif defined(FIFO_ALL_MOST_FULL)
            #else
            hx3690l_gpioint_cfg(false);
            #endif
            hx3690l_hrs_cal_off(0);
            s_cal_state = 0;
            AGC_LOG("cal off mode\r\n");
            break;

        default:
            break;
    }
}


SENSOR_ERROR_T hx3690l_hrs_enable(void)
{
    if (!hx3690l_chip_check()) 
    {
        AGC_LOG("hx3690l check id failed!\r\n");
        return SENSOR_OP_FAILED;
    }

    AGC_LOG("hx3690l check id success!\r\n");

    if (s_ppg_state) 
    {
        AGC_LOG("ppg already on!\r\n");
        return SENSOR_OP_FAILED;
    }
    if(!hx3690_alg_open())
    {
        AGC_LOG("hrs alg open fail,or dynamic ram not enough!\r\n");
    }
    
    hrs_wear_status = MSG_HRS_NO_WEAR;
    hrs_wear_status_pre = MSG_HRS_NO_WEAR;
    
    hx3690l_hrs_set_mode(PPG_INIT);

    AGC_LOG("hx3690l enable!\r\n");

    return SENSOR_OK;
}

void hx3690l_hrs_disable(void)
{
    hx3690l_320ms_timer_cfg(false);
    hx3690l_40ms_timer_cfg(false);
    hx3690l_gpioint_cfg(false);
	
    hx3690l_hrs_set_mode(CAL_OFF);
    hx3690l_hrs_set_mode(PPG_OFF);
    hx3690_alg_close();

    AGC_LOG("hx3690l disable!\r\n");

}

hx3690_hrs_wear_msg_code_t hx3690_hrs_get_wear_status(void)
{
    return  hrs_wear_status;
}

HRS_CAL_SET_T get_hrs_agc_status(void)
{
    HRS_CAL_SET_T cal;

    cal.flag =  calReg.flag;
    cal.int_cnt =  calReg.int_cnt;
    cal.LED=  calReg.LED;     // phasex led driver config
    cal.LEDDAC=  calReg.LEDDAC;  // phasex led offset idac cfg
    cal.AMBDAC=  calReg.AMBDAC;  // phasex offset idac cfg
    cal.RF=  calReg.RF;      // phasex tia feed back resister cfg
    cal.led_step=  calReg.led_step;
    cal.state=  calReg.state;

    return cal;
}

//void hx3690l_hrs_int_read_data(int32_t *s_buf)
//{
//  if(int_read_data_swi == 0)
//  {
//    
//  }
//  else
//  {
//    
//  }
//}

void hx3690l_hrs_read_fifo_data(uint8_t read_fifo_size,int32_t *buf)
{
    uint8_t data_flg = 127;
    int32_t data;
    uint8_t databuf[3];
    uint8_t ii=0;
     //uint8_t jj=0;
    for(ii=0; ii<read_fifo_size; ii++) 
    {
        hx3690l_write_reg(0x17, 0x00); // write any walue to 0x17 will update a new data
        hx3690l_delay_us(100);
        hx3690l_brust_read_reg(0x15, databuf, 3); 
        //databuf[2]=hx3690l_read_reg(0x17);
        //databuf[1]=hx3690l_read_reg(0x16);
        //databuf[0]=hx3690l_read_reg(0x15);
        data_flg = databuf[2]>>5;
        data = (int32_t)(databuf[0]|(databuf[1]<<8)|((databuf[2]&0x1f)<<16));

        if(ii==0){
			if(data_flg ==3){
				//jj=ii+2;
				ii=3;
                buf[0] = 0;
                buf[1] = 0;
                buf[2] = 0;
				//hx3690_fifo_data_error_flg=1;
			}
			if(data_flg ==2){
				//jj=ii+1;
                
				ii=2;
                buf[0] = 0;
                buf[1] = 0;
			}
            if(data_flg ==1){
				//jj=ii+1;
                
				ii=1;
                buf[0] = 0;
			}			
		}
		
        if(data_flg == 0) 
        {
            buf[ii]= data;
        } 
        else if(data_flg == 1)
        {
            buf[ii]= data;
        } 
        else if(data_flg == 2)
        {
            buf[ii]= data;
        } 
        else if(data_flg == 3) 
        {
            buf[ii]= data;
        }
        //jj= jj+1;
    }
}
uint8_t hx3690l_hrs_read(hrs_sensor_data_t * s_dat)
{
    int32_t PPG_src_data;
    int32_t Ir_src_data;
    bool recal = false;
    uint8_t size = 0;
    uint8_t size_byte = 0;
    int32_t *PPG_buf =  &(s_dat->ppg_data[0]);
    int32_t *ir_buf =  &(s_dat->ir_data[0]);  
    int32_t *s_buf =  &(s_dat->s_buf[0]);   
    s_dat->agc_green =  calReg.LED;

    if (!s_ppg_state || s_cal_state) 
    {
        return NULL;
    }    
    
    size_byte = hx3690l_read_fifo_size();
    //AGC_LOG("ppg data size: %d\r\n", size_byte);
    
    if(size_byte<4)
    {
        return NULL;
    }

    size = size_byte/4;
    size_byte = size*4;
    
    s_dat->count =  size;
    
    if (size_byte && size_byte <= 64) 
    {
        hx3690l_hrs_read_fifo_data(size_byte,s_buf);
        
        
        //fifo data order is phase1,phase2
        for (uint8_t i=0; i<size; i++) 
        {
            PPG_src_data = s_buf[i*4] - s_buf[i*4+1];
            Ir_src_data = s_buf[i*4+2] - s_buf[i*4+3];            
            if ((s_buf[i*4]<523800 || s_buf[i*4]>1571400)
                ||(s_buf[i*4+1]<523800 || s_buf[i*4+1]>1571400)
                ||(s_buf[i*4+3]<523800 || s_buf[i*4+3]>1571400))            
            {
                recal = true;
               
                if(hrs_wear_status==MSG_HRS_NO_WEAR)
                {
                     recal = false;
                }                
            }

            PPG_buf[i] = PPG_src_data;//+1047600;
            ir_buf[i] = Ir_src_data;
            
            hx3690_hrs_wear_mode_check(HRS_MODE,Ir_src_data); 

            //AGC_LOG("%d/%d %d %d %d %d %d %d %d %d\r\n" ,1+i,size,   \
            PPG_src_data,PPG_buf[i],ir_buf[i],s_buf[i*4],s_buf[i*4+1],s_buf[i*4+2], \
            s_buf[i*4+3],calReg.LED);
            
        }

        if (recal) 
        {
            cal_delay--;

            if (cal_delay <= 0) 
            {
                if(calReg.LED > ((calReg.led_max_cur*7)>>3))
                {
                    calReg.cur255_cnt++;
                    if(calReg.cur255_cnt>0)
                    {
                        calReg.cur255_cnt = 0;
                        calReg.led_idac = ((uint16_t)calReg.led_idac*3)/4;
                    }
                }
                cal_delay = CAL_DELAY_COUNT;
                hx3690l_hrs_set_mode(RECAL_INIT);
                AGC_LOG("========>>>>LED_IDAC = %d\r\n",calReg.led_idac);
            }
        }
        else                       
        {
            cal_delay = CAL_DELAY_COUNT;
        }
    }
    

    return 1;
}

#endif




