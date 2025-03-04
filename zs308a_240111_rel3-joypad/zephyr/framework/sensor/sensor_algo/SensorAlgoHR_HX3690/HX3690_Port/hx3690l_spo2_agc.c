
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

//#include "twi_master.h"
#include "hx3690l.h"
#include "hx3690l_spo2_agc.h"
//#include "hx3690l_spo2_alg.h"
//#include "SEGGER_RTT.h"

#ifdef SPO2_ALG_LIB

extern const uint8_t  hx3690l_spo2_agc_red_idac;  // 6,7,8...
extern const uint8_t  hx3690l_spo2_agc_ir_idac;  // 6,7,8...
extern const uint8_t  red_led_max_init;
extern const uint8_t  ir_led_max_init;
extern uint8_t low_vbat_flag;

//SPO2_INFRARED_THRES
extern const int32_t  spo2_ir_unwear_thres; 
extern const int32_t  spo2_ir_wear_thres; 



static uint8_t s_ppg_state = 0;
static uint8_t s_cal_state = 0;
//static int32_t s_buf[64] = {0}; 
static int32_t agc_buf[64] = {0};

static uint8_t cal_delay = CAL_DELAY_COUNT;
static SPO2_CAL_SET_T  calReg;


static hx3690_spo2_wear_msg_code_t spo2_wear_status = MSG_SPO2_NO_WEAR;
static hx3690_spo2_wear_msg_code_t spo2_wear_status_pre = MSG_SPO2_NO_WEAR;

//


void Init_Spo2_PPG_Calibration_Routine(SPO2_CAL_SET_T *calR,uint8_t led)
{
    calR->flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
    
    calR->R_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->IR_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->AMBDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->R_RF = 0;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->IR_RF = 0;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->R_LED = SPO2_CAL_INIT_LED;
    calR->IR_LED = SPO2_CAL_INIT_LED;
    calR->state = sCalStart;
    calR->int_cnt = 0;
    calR->cur255_cnt =0;
    calR->red_idac = hx3690l_spo2_agc_red_idac;
    calR->ir_idac = hx3690l_spo2_agc_ir_idac;
    if(low_vbat_flag == 1)
    {
        calR->red_idac = (hx3690l_spo2_agc_red_idac*3)>>2;
        calR->ir_idac = (hx3690l_spo2_agc_ir_idac*3)>>2;
        calR->red_max_cur = (red_led_max_init*3)>>2;
        calR->ir_max_cur = (ir_led_max_init*3)>>2;
    }
    else
    {
        calR->red_idac = hx3690l_spo2_agc_red_idac;
        calR->ir_idac = hx3690l_spo2_agc_ir_idac;
        calR->red_max_cur = red_led_max_init;
        calR->ir_max_cur = ir_led_max_init;
    }
}

void Restart_Spo2_PPG_Calibration_Routine(SPO2_CAL_SET_T *calR)
{
    calR->flag = CAL_FLG_LED_DR|CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC|CAL_FLG_RF;
    
    calR->R_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->IR_LEDDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->AMBDAC = 0;   /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    calR->R_RF = 0;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->IR_RF = 0;       /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    calR->R_LED = SPO2_CAL_INIT_LED;
    calR->IR_LED = SPO2_CAL_INIT_LED;
    calR->state = sCalStart;
    calR->int_cnt = 0;
//    calR->cur255_cnt =0;
//    calR->red_idac = hx3690l_spo2_agc_red_idac;
//    calR->ir_idac = hx3690l_spo2_agc_ir_idac;
}

hx3690_spo2_wear_msg_code_t hx3690_spo2_check_unwear(WORK_MODE_T mode,int32_t infrared_data)
{
    if(infrared_data < spo2_ir_unwear_thres)
    {
        spo2_wear_status = MSG_SPO2_NO_WEAR;
        if(mode == WEAR_MODE)
        {
            return spo2_wear_status;
        }        
        if(spo2_wear_status_pre != MSG_SPO2_NO_WEAR)
        {
            spo2_wear_status_pre = MSG_SPO2_NO_WEAR;
            hx3690l_spo2_low_power();
        }          
    }
    
    return spo2_wear_status;
}
hx3690_spo2_wear_msg_code_t hx3690_spo2_check_wear(WORK_MODE_T mode,int32_t infrared_data)
{
    if(infrared_data > spo2_ir_wear_thres)
    {
        spo2_wear_status = MSG_SPO2_WEAR;
        if(mode == WEAR_MODE)
        {
            return spo2_wear_status;
        }
        if(spo2_wear_status_pre != MSG_SPO2_WEAR)
        {
            spo2_wear_status_pre = MSG_SPO2_WEAR;
            hx3690_spo2_alg_open_deep();
            //hx3690l_spo2_set_mode(PPG_INIT);
            //hx3690l_spo2_set_mode(CAL_INIT);
        }  
    }
    
    return spo2_wear_status;
}

bool  hx3690_spo2_change_to_wear(WORK_MODE_T mode,int32_t infrared_data)
{
    if(infrared_data > spo2_ir_wear_thres)
    {
        spo2_wear_status = MSG_SPO2_WEAR;
        if(mode == WEAR_MODE)
        {
            return spo2_wear_status;
        }
        if(spo2_wear_status_pre != MSG_SPO2_WEAR)
        {
        	AGC_LOG("hx3690_spo2_check_wear ir = %d\r\n", infrared_data);
            spo2_wear_status_pre = MSG_SPO2_WEAR;
            hx3690_spo2_alg_open_deep();
            return true;
        }  
    }
    
    return false;
}


void PPG_Spo2_Calibration_Routine(SPO2_CAL_SET_T *calR, int32_t r_led, int32_t amb, int32_t ir_led)
{
    int32_t dif = 0;
    int32_t led_tmp = 0;
    int32_t step_tmp = 0;

    switch(calR->state)
    {
        case sCalStart:  
            //AGC_LOG("ir-amb=%d\r\n",ir_led-amb);            
            hx3690_spo2_check_unwear(SPO2_MODE,ir_led-amb); 
            if(spo2_wear_status == MSG_SPO2_NO_WEAR)
            {
                calR->state = sCalFinish;
                //AGC_LOG("status6: cal finish\r\n");
                break;
            }
      
            calR->AMBDAC = amb/2608;
            AGC_LOG("AMBDAC: %d\r\n",calR->AMBDAC);
            calR->R_LEDDAC = calR->AMBDAC+calReg.red_idac;   /*red led i_pd 64*0.25=3.5ua*/
            calR->IR_LEDDAC = calR->AMBDAC+calReg.ir_idac;   /*ir led i_pd 64*0.25=3.5ua*/
            calR->flag = CAL_FLG_LED_DAC|CAL_FLG_AMB_DAC;
            if(r_led>amb)
            {
                calR->R_led_step = (r_led-amb)/SPO2_CAL_INIT_LED; 
            } 
            if(ir_led>amb)
            {
                calR->IR_led_step = (ir_led-amb)/SPO2_CAL_INIT_LED; 
            }            
            calR->state = sCalLed;
            break;
        case sCalLed:   
            if(calR->R_led_step > 0)
            {
                led_tmp = r_led;
                step_tmp = calR->R_led_step;
                dif = (led_tmp/step_tmp);
				if((SPO2_CAL_INIT_LED-dif)>calR->red_max_cur)
                {
                   calR->R_LED = calR->red_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                }
                else 
                {
                    calR->R_LED = SPO2_CAL_INIT_LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                                
                }       
				                     
                led_tmp = ir_led;
                step_tmp = calR->IR_led_step;
                dif = (led_tmp/step_tmp);                            
                if((SPO2_CAL_INIT_LED-dif)>calR->ir_max_cur)
                {
                    calR->IR_LED = calR->ir_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                } 
                else 
                {
                    calR->IR_LED = SPO2_CAL_INIT_LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                }      
            }               
            calR->flag = CAL_FLG_LED_DR;
            calR->state = sCalLed2;
            break;
        case sCalLed2:   
            if(calR->R_led_step > 0)
            {
                led_tmp = r_led;
                step_tmp = calR->R_led_step;
                dif = (led_tmp/step_tmp);
                calR->R_LED = calR->R_LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                if((calR->R_LED-dif)>calR->red_max_cur)
                {
                    calR->R_LED = calR->red_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                }
                else
                {
                    calR->R_LED = calR->R_LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */              
                }                                
                led_tmp = ir_led;
                step_tmp = calR->IR_led_step;
                dif = (led_tmp/step_tmp);
                if((calR->IR_LED-dif)>calR->ir_max_cur)
                {
                    calR->IR_LED = calR->ir_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                } 
                else 
                {
                    calR->IR_LED = calR->IR_LED-dif;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                }                            
            }               
            calR->flag = CAL_FLG_LED_DR;
            calR->state = sCalLed3;
            break;
        case sCalLed3:   
            if(calR->R_led_step > 0)
            {
                led_tmp = r_led;
                step_tmp = calR->R_led_step;
                dif = (led_tmp/step_tmp);
                if(((calR->R_LED-dif + 2))>calR->red_max_cur)
                {
                    calR->R_LED = calR->red_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                } 
                else 
                {
                    calR->R_LED = calR->R_LED-dif + 2;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                }            
                                
                led_tmp = ir_led;
                step_tmp = calR->IR_led_step;
                dif = (led_tmp/step_tmp);
                if(((calR->IR_LED-dif +2 ))>calR->ir_max_cur)
                {
                    calR->IR_LED = calR->ir_max_cur;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */                                    
                } 
                else
                {
                    calR->IR_LED = calR->IR_LED-dif + 2 ;   /* 0~255 = 0 ~ 200ma, step = 0.8ma */
                }        
            }               
            calR->flag = CAL_FLG_LED_DR;
            calR->state = sCalRf;
            break;                        
        case sCalRf:
            calR->R_RF = 6; //500K
            calR->IR_RF = 6; //500K
            calR->flag = CAL_FLG_RF;
            calR->state = sCalRfEnd;
            break;
        case sCalRfEnd:
            calR->state = sCalFinish;
            break;
        default:
            
            break;
        
    }
    //AGC_LOG("AGC. Rled_drv=%d,Irled_drv=%d,RledDac=%d,IrledDac=%d,ambDac=%d,Rledstep=%d,Irledstep=%d,Rrf=%d,Irrf=%d,\r\n",\
            calR->R_LED, calR->IR_LED,calR->R_LEDDAC,calR->IR_LEDDAC,calR->AMBDAC,\
            calR->R_led_step,calR->IR_led_step,calR->R_RF,calR->IR_RF); 
}

SPO2_CAL_SET_T PPG_spo2_agc(void)
{
    int32_t r_led_val, amb_val;
    int32_t ir_led_val;
	
    calReg.work = false;
    if (!s_cal_state) 
    {
        return calReg;
    } 
#ifdef EXT_INT_AGC     
    calReg.int_cnt ++;
    if(calReg.int_cnt < 8)
    {
         //AGC_LOG("calReg.int_cnt = %d!\r\n",calReg.int_cnt);
         
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
    agc_buf[0] = phase1
    agc_buf[1] = phase3
    agc_buf[2] = phase4
    */
    read_spo2_data_packet(agc_buf);
    r_led_val = agc_buf[0] - 1047600;
    amb_val = agc_buf[1] - 1047600;
    ir_led_val = agc_buf[2] - 1047600;

    
    AGC_LOG("cal dat ch1=%d,ch3=%d,ch4=%d,red_val=%d,amb_val=%d,ir_val=%d\r\n",
    agc_buf[0], agc_buf[1], agc_buf[2],r_led_val, amb_val,ir_led_val);
    
    PPG_Spo2_Calibration_Routine(&calReg, r_led_val, amb_val, ir_led_val);
    
    if (calReg.state == sCalFinish) 
    { 
        hx3690l_spo2_set_mode(CAL_OFF);
        if(spo2_wear_status == MSG_SPO2_NO_WEAR)
        {
            hx3690l_spo2_low_power();
        }
    }
    else 
    {
        hx3690l_spo2_updata_reg();
        #if defined(INT_MODE)
        #elif defined(FIFO_ALL_MOST_FULL)
        #else
        hx3690l_gpioint_cfg(true);
        #endif
    }

    return  calReg;
}


void hx3690l_spo2_cal_init(void) // 20200615 ericy afe cali offline
{
    #ifdef EXT_INT_AGC 
    uint16_t sample_rate = 200;                      /*config the data rate of chip alps2_fm ,uint is Hz*/
    #else
    uint16_t sample_rate = 200;   // 25
    #endif
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    uint8_t data_avg_num = 0;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
	
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>
	
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );
	
    hx3690l_write_reg(0x13,0x30); //FIFO bypass mode enable
    hx3690l_write_reg(0x23,0x20); //phase4 convertion ready enable
  
    #if defined(FIFO_ALL_MOST_FULL)
    hx3690l_write_reg(0x24,0x00);
    #else
    #endif

    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5); 
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal    
}


         
void hx3690l_spo2_cal_off(void) // 20200615 ericy afe cali offline
{
    uint16_t sample_rate = 50;                       /*config the data rate of chip alps2_fm ,uint is Hz*/
//    if (enable_50_hz) {
//        sample_rate = 50;
//    }
	  uint8_t data_avg_num = 1;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );		
  
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
s_buf[0] = phase1
s_buf[1] = phase3
s_buf[2] = phase4
//s_buf[3] = phase2
*/
void read_spo2_data_packet(int32_t *buf) // 20200615 ericy read reg_data phase1 and phase3
{
    uint8_t dataBuf[9];
	uint8_t i;
    
    
    hx3690l_brust_read_reg(0x03, dataBuf, 9);     //phase1 ,3,4
  
    for (i=0; i<3; i++)
    {
        buf[i] = (int32_t)(dataBuf[3*i]|(dataBuf[3*i+1]<<8)|(dataBuf[3*i+2]<<16));
    }
}

void read_spo2_ir_packet(int32_t *buf) // 20200615 ericy read reg_data phase1 and phase3
{
    uint8_t dataBuf[6];
    uint8_t i;
    hx3690l_brust_read_reg(0x06, dataBuf, 6);     //phase3 and phase4
    
    for (i=0; i<2; i++) 
    {
        buf[i] = (int32_t)(dataBuf[3*i]|(dataBuf[3*i+1]<<8)|(dataBuf[3*i+2]<<16));
    }
}

uint8_t hx3690l_spo2_read_data_packet(int32_t *s_buf) // 20200615 ericy read reg_data phase1 and phase3 
{
    uint8_t  databuf1[6] = {0};
		uint8_t  databuf2[6] = {0};
    uint32_t P1 = 0,P2 = 0 ,P3 = 0 ,P4 =0 ;
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
    s_buf[4] = calReg.R_LED;
    s_buf[5] = calReg.IR_LED;   
    
    uint8_t recal = 0;
    if (s_buf[0]<523800 || s_buf[0]>1571400 || s_buf[3]<523800 || s_buf[3]>1571400)
    {
        recal = true;
       
        if(spo2_wear_status==MSG_HRS_NO_WEAR)
        {
             recal = false;
        }                
    } 
    int32_t ir_data = s_buf[3]-s_buf[2]; 
    if (hx3690_spo2_change_to_wear(SPO2_MODE,ir_data))
    {
        return NULL;
    }
 
    if (recal) 
    {
        cal_delay--;

        if (cal_delay <= 0) 
        {
            if((calReg.R_LED > ((calReg.red_max_cur*7)>>3))||(calReg.IR_LED > ((calReg.ir_max_cur*7)>>3)))
            {
                calReg.cur255_cnt++;
                if(calReg.cur255_cnt>0)
                {
                    calReg.cur255_cnt = 0;
                    calReg.red_idac = ((uint16_t)calReg.red_idac*3)/4;
                    calReg.ir_idac = ((uint16_t)calReg.ir_idac*3)/4;
                    DEBUG_PRINTF(0,"%d/%d %d \r\n" ,calReg.red_idac,calReg.ir_idac);
                }
            }
            cal_delay = CAL_DELAY_COUNT;
            hx3690l_spo2_set_mode(RECAL_INIT);
        }
    } 
    else 
    {
        cal_delay = CAL_DELAY_COUNT;
    } 
    return 1;
}


void hx3690l_spo2_low_power(void)
{   
    uint16_t sample_rate = 25;//25;                       /*config the data rate of chip alps2_fm ,uint is Hz*/
    uint8_t data_avg_num = 0;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
    uint32_t prf_clk_num = 32000/sample_rate;        /*period in clk num, num = Fclk/fs */

    uint8_t phase1_led_en = 0;     
    uint8_t phase2_led_en = 0;
    uint8_t phase3_led_en = 0;
    uint8_t phase4_led_en = 1;
    
    uint8_t phase1_ldr_sel = 0; 
    uint8_t phase1_pd_sel = 1;
    uint8_t phase1_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0; 
    
    uint8_t phase3_tia_res = 0; 
    uint8_t phase4_tia_res = 0; 
    
    uint8_t phase4_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    
    uint8_t led_on_time = 1;      /* 0 = 32clk=8us ; 1 = 64clk=16us; 2=128clk=32us ; 3 = 256clk=64us ;
                                     4 = 512clk=128us ; 5 = 1024clk=256us; 6= 2048clk=512us; 7 = 4096clk=1024us */
    hx3690l_write_reg(0X6a, 0X00);	//rest int
    hx3690l_delay(10);
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );	
    hx3690l_write_reg(0X34, (phase1_pd_sel<<4 |  phase1_ldr_sel));    
    hx3690l_write_reg(0X2d, phase3_tia_res);
    hx3690l_write_reg(0X2e, phase4_tia_res);

    hx3690l_write_reg(0X2c, phase1_tia_res); 
    hx3690l_write_reg(0X38, phase1_offset_idac);
    hx3690l_write_reg(0X30, phase1_ldr_cur);
    
    hx3690l_write_reg(0X32, phase4_ldr_cur);
    
    hx3690l_write_reg(0X1f, (led_on_time<<4| phase1_led_en<<3 | phase3_led_en<<2 | phase4_led_en<<1 | phase2_led_en) );
    hx3690l_write_reg(0X6a, 0X02);
  
  #ifdef INT_MODE
    hx3690l_write_reg(0x13,0x30); //FIFO mode enable
    hx3690l_write_reg(0x23,0x20);  // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4

    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5);             //Delay for reset time
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal
  #else
    hx3690l_write_reg(0x13,0x31); //FIFO mode enable
    hx3690l_write_reg(0x23,0x20);  // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0x51,0x02); //Chip reset
    hx3690l_delay(5);             //Delay for reset time
    hx3690l_write_reg(0X13, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x31);
    hx3690l_write_reg(0x51,0x00); //Chip state machine work normal
  #endif
    
    calReg.R_LED =  phase1_ldr_cur;
    calReg.IR_LED =  phase4_ldr_cur;

    AGC_LOG(" chip go to low power mode  \r\n" );   

//yorke close 40m timer    
}

void hx3690l_spo2_normal_power(void)
{    
     //yorke open 40ms timer
    //put this into alg_open_deep()

}


void hx3690l_spo2_updata_reg(void)
{
    if (calReg.flag & CAL_FLG_LED_DR) 
    {
        hx3690l_write_reg(0X30, calReg.R_LED);     // phase1 led driver config
        hx3690l_write_reg(0X32, calReg.IR_LED);     // phase4 led driver config 
    }
    
    if (calReg.flag & CAL_FLG_LED_DAC) 
    {
        hx3690l_write_reg(0X38, calReg.R_LEDDAC);  // phase1 offset idac cfg
        hx3690l_write_reg(0X3a, calReg.IR_LEDDAC);  // phase4 offset idac cfg
    }
    
    if (calReg.flag & CAL_FLG_AMB_DAC) 
    {
        hx3690l_write_reg(0X39, calReg.AMBDAC);  // phase3 offset idac cfg
    }
    
    if (calReg.flag & CAL_FLG_RF) 
    {
        hx3690l_write_reg(0X2c, calReg.R_RF);    // phase1 tia feed back resister cfg
        hx3690l_write_reg(0X2d, calReg.IR_RF);    // phase3 tia feed back resister cfg
        hx3690l_write_reg(0X2e, calReg.IR_RF);    // phase4 tia feed back resister cfg
//        hx3690l_write_reg(0X2f, calReg.R_RF);    // phase2 tia feed back resister cfg
    }
}
void hx3690l_spo2_set_mode(uint8_t mode_cmd)
{
    switch (mode_cmd) 
    {
        case PPG_INIT:
            hx3690l_spo2_ppg_init();
        
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
            hx3690l_spo2_low_power();
            s_ppg_state = 0;
            AGC_LOG("ppg led off mode\r\n");
            break;

        case CAL_INIT:
            Init_Spo2_PPG_Calibration_Routine(&calReg,64);
            hx3690l_spo2_cal_init();
            hx3690l_spo2_updata_reg();
        
            #if defined(INT_MODE)
            #elif defined(FIFO_ALL_MOST_FULL)
            #else
            hx3690l_gpioint_cfg(true);
            #endif
        
            s_cal_state = 1;
            AGC_LOG("cal init mode\r\n"); 
            break;        
        case RECAL_INIT:
            Restart_Spo2_PPG_Calibration_Routine(&calReg);
            hx3690l_spo2_cal_init();
            hx3690l_spo2_updata_reg();

            #if defined(INT_MODE)
            #elif defined(FIFO_ALL_MOST_FULL)
            #else
            hx3690l_gpioint_cfg(true);
            #endif
        
            s_cal_state = 1;
            AGC_LOG("recal init mode\r\n");
            break;

        case CAL_OFF:
            #if defined(INT_MODE)
            #elif defined(FIFO_ALL_MOST_FULL)
            #else
            hx3690l_gpioint_cfg(false);
            #endif
        
            hx3690l_spo2_cal_off();
            s_cal_state = 0;
            AGC_LOG("cal off mode\r\n");
            break;

        default:
            break;
    }
}

SENSOR_ERROR_T hx3690l_spo2_enable(void)
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

    if(!hx3690_spo2_alg_open())
    {
        AGC_LOG("spo2 alg open fail,or dynamic ram not enough!\r\n");
        return SENSOR_OP_FAILED;
    }
    spo2_wear_status = MSG_SPO2_NO_WEAR;
    spo2_wear_status_pre = MSG_SPO2_NO_WEAR;
    hx3690l_spo2_set_mode(PPG_INIT);

    AGC_LOG("hx3690l enable!\r\n");

    return SENSOR_OK;
}

void hx3690l_spo2_disable(void)
{
    hx3690l_320ms_timer_cfg(false);
    hx3690l_40ms_timer_cfg(false); 

    hx3690l_spo2_set_mode(CAL_OFF);
    hx3690l_spo2_set_mode(PPG_OFF);
    hx3690_spo2_alg_close();    

    AGC_LOG("hx3690l disable!\r\n");
}

hx3690_spo2_wear_msg_code_t hx3690_spo2_get_wear_status(void)
{
    return  spo2_wear_status;
}

SPO2_CAL_SET_T get_spo2_agc_status(void)
{
    SPO2_CAL_SET_T cal;

    cal.flag = calReg.flag;
    cal.int_cnt = calReg.int_cnt;
    cal.R_LED = calReg.R_LED;     // phasex led driver config
    cal.IR_LED = calReg.IR_LED;     // phasex led driver config
    cal.R_LEDDAC = calReg.R_LEDDAC;  // phasex led offset idac cfg
    cal.IR_LEDDAC = calReg.IR_LEDDAC;  // phasex led offset idac cfg
    cal.AMBDAC = calReg.AMBDAC;  // phasex offset idac cfg
    cal.R_RF = calReg.R_RF;      // phasex tia feed back resister cfg
    cal.IR_RF = calReg.IR_RF;      // phasex tia feed back resister cfg
    cal.R_led_step = calReg.R_led_step;
    cal.IR_led_step = calReg.IR_led_step;
    cal.state = calReg.state;
    cal.red_idac = calReg.red_idac;
    cal.ir_idac = calReg.ir_idac;

    return cal;
}

void hx3690l_apo2_read_fifo_data(uint8_t read_fifo_size,int32_t *buf)
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
        
        databuf[2]=hx3690l_read_reg(0x17);
        databuf[1]=hx3690l_read_reg(0x16);
        databuf[0]=hx3690l_read_reg(0x15);
        data_flg = databuf[2]>>5;
        data = (int32_t)(databuf[0]|(databuf[1]<<8)|((databuf[2]&0x1f)<<16));

        //AGC_LOG("data_flg: %d,%d\r\n",databuf[2], data_flg);
        if(ii==0){
			if(data_flg ==3){
				//jj=ii+2;
				ii=2;
                buf[0] = 0;
                buf[1] = 0;
				//hx3690_fifo_data_error_flg=1;
			}
			if(data_flg ==2){
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

uint8_t hx3690l_spo2_read(spo2_sensor_data_t * s_dat)
{
    int32_t Red_src_data;
    int32_t Ir_src_data;
    bool recal = false;
    uint8_t size = 0;
    uint8_t size_byte = 0;
	uint8_t i;
    int32_t *red_buf =  &(s_dat->red_data[0]);
    int32_t *ir_buf =  &(s_dat->ir_data[0]); 
    int32_t *s_buf =  &(s_dat->s_buf[0]);   
    s_dat->agc_red =  calReg.R_LED;
    s_dat->agc_ir =  calReg.IR_LED;

    AGC_LOG("s_ppg_state: %d,s_cal_state: %d\r\n",s_ppg_state,s_cal_state);
    if (!s_ppg_state || s_cal_state) 
    {
        return NULL;
    }    

    size_byte = hx3690l_read_fifo_size();
    AGC_LOG("ppg data size: %d\r\n", size_byte);
    if(size_byte<3)
    {
        return NULL;
    }

    size = size_byte/3;  //count of data generate
    size_byte = size*3;
    
    s_dat->count =  size;
    
    if (size_byte && size_byte <= 64) 
    {
        hx3690l_apo2_read_fifo_data(size_byte,s_buf);
       
        for (i=0; i<size; i++) 
        {
            Red_src_data = s_buf[i*3] - s_buf[i*3+1];
            Ir_src_data = s_buf[i*3+2] - s_buf[i*3+1];

            if ((s_buf[i*3]<523800 || s_buf[i*3]>1571400) 
                ||(s_buf[i*3+2]<523800 || s_buf[i*3+2]>1571400))
            {
                recal = true;
               
                if(spo2_wear_status==MSG_SPO2_NO_WEAR)
                {
                     recal = false;
                }

            }
            

            red_buf[i] = Red_src_data;
            ir_buf[i] = Ir_src_data;
            
            //DEBUG_PRINTF(0,"%d/%d %d %d %d %d %d %d %d %d %d\r\n" ,1+i,size,\
            red_buf[i],ir_buf[i],s_buf[i*3],s_buf[i*3+1],s_buf[i*3+2],\
            calReg.R_LED,calReg.IR_LED,calReg.red_idac,calReg.ir_idac);
            
            if (s_buf[i*3+1] != 0)  // ?¡¤?31a2??¨¢?a0¡ê? ¡¤¨¤?1¨ºy?Y¡ä¨ª??3???¨¨?¨ª¨®
            {
	            if (hx3690_spo2_change_to_wear(SPO2_MODE,Ir_src_data))
	            {
	            		return NULL;
	            }
            }
            
        }

        if (recal) 
        {
            cal_delay--;

            if (cal_delay <= 0) 
            {
                if((calReg.R_LED > ((calReg.red_max_cur*7)>>3))||(calReg.IR_LED > ((calReg.ir_max_cur*7)>>3)))
                {
                    calReg.cur255_cnt++;
                    if(calReg.cur255_cnt>0)
                    {
                        calReg.cur255_cnt = 0;
                        calReg.red_idac = ((uint16_t)calReg.red_idac*3)/4;
                        calReg.ir_idac = ((uint16_t)calReg.ir_idac*3)/4;
                        DEBUG_PRINTF(0,"%d/%d %d \r\n" ,calReg.red_idac,calReg.ir_idac);
                    }
                }
                cal_delay = CAL_DELAY_COUNT;
                hx3690l_spo2_set_mode(RECAL_INIT);
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








