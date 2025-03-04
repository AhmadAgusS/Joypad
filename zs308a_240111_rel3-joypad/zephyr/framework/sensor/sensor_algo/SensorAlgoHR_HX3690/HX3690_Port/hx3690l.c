
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <hr_algo_data.h>

//#include "SEGGER_RTT.h"
//#include "app_timer.h"
//#include "nrf_delay.h"
//#include "nrf_gpio.h"
//#include "nrf_drv_gpiote.h"
//#include "twi_master.h"
#include "hx3690l.h"
#include "hx3690q_factory_test.h"

//#include "drv_oled.h"
//#include "opr_oled.h"

#ifdef SPO2_VECTOR
#include "spo2_vec.h" 
uint32_t spo2_send_cnt = 0;
uint32_t red_buf_vec[8];
uint32_t ir_buf_vec[8];
#endif

#ifdef HR_VECTOR
#include "hr_vec.h" 
uint32_t spo2_send_cnt = 0;
uint32_t PPG_buf_vec[8];
#endif

#ifdef GSENSER_DATA
#include "lis3dh_drv.h"
#endif

const uint8_t  hx3690_accurate_first_shot = 0;
const uint8_t  hx3690_up_factor1 = 3;
const uint8_t  hx3690_up_shift1 = 2;
const uint8_t  hx3690_up_factor2 = 31;
const uint8_t  hx3690_up_shift2 = 5;

// configure for JUDGE_HUMAN_ENABLE
const uint8_t  hx3690_human_delta1_thrs     = 40;   //The smaller value easy enter human status,recommand vaule 40 to 60
const uint8_t  hx3690_human_delta1_thrs2    = 20;   //The smaller value easy enter human status,recommand vaule 20 to 30
const uint8_t  hx3690_human_thrs            = 4;    //The smaller value easy enter human status,recommand vaule 2 to 6 or to 10               
const uint16_t  hx3690_human_static_thrs    = 4000; //The bigger value easy enter human status,recommand value 2000 to 9000,

//SPO2 agc
const uint8_t  hx3690l_spo2_agc_red_idac = RED_AGC_OFFSET;  //110 6,7,8...
const uint8_t  hx3690l_spo2_agc_ir_idac = IR_AGC_OFFSET;  //110 6,7,8...
//hrs agc
const uint8_t  hx3690l_hrs_agc_idac = GREEN_AGC_OFFSET;  //default=14 6,7,8...
//hrv agc
const uint8_t  hx3690l_hrv_agc_idac = GREEN_AGC_OFFSET;

const uint8_t  green_led_max_init = 255;
const uint8_t  red_led_max_init = 255;
const uint8_t  ir_led_max_init = 255;
uint8_t low_vbat_flag =0;

//HRS_INFRARED_THRES
const int32_t  hrs_ir_unwear_thres = 10000;
const int32_t  hrs_ir_wear_thres = 20000; 
//HRV_INFRARED_THRES
const int32_t  hrv_ir_unwear_thres = 8000;
const int32_t  hrv_ir_wear_thres = 15000; 
//SPO2_INFRARED_THRES
const int32_t  spo2_ir_unwear_thres = 10000; 
const int32_t  spo2_ir_wear_thres = 30000; 
//CHECK_WEAR_MODE_THRES
const int32_t  check_mode_unwear_thre = 20000;
const int32_t  check_mode_wear_thre = 40000;

//static volatile oled_display_t oled_dis = {0};
uint32_t vec_data_cnt=0;

#if defined(MALLOC_MEMORY)
uint8_t alg_ram[2*1024] = {0};
#else
uint8_t alg_ram[11*1024] = {0};
#endif


//

#ifdef GSENSER_DATA
volatile int16_t gsen_fifo_x[64];  //ppg time 330ms..330/40 = 8.25
volatile int16_t gsen_fifo_y[64];
volatile int16_t gsen_fifo_z[64];
#else
int16_t gen_dummy[64] = {0};
#endif

WORK_MODE_T work_mode_flag = HRS_MODE;
//////// spo2 para and switches
const  uint8_t   COUNT_BLOCK_NUM = 50;            //delay the block of some single good signal after a series of bad signal 
const  uint8_t   SPO2_LOW_XCORR_THRE = 30;        //(64*xcorr)'s square below this threshold, means error signal
const  uint8_t   SPO2_CALI = 1;                       //spo2_result cali mode
const  uint8_t   XCORR_MODE = 1;                  //xcorr mode switch
const  uint8_t   QUICK_RESULT = 1;                //come out the spo2 result quickly ;0 is normal,1 is quick
const  uint16_t  MEAN_NUM = 256;                  //the length of smooth-average ;the value of MEAN_NUM can be given only 256 and 512
const  uint8_t   G_SENSOR = 1;                      //if =1, open the gsensor mode
const  uint8_t   SPO2_GSEN_POW_THRE =100;         //gsen pow judge move, range:0-200;
const  uint32_t  SPO2_BASE_LINE_INIT = 135000;    //spo2 baseline init, = 103000 + ratio(a variable quantity,depends on different cases)*SPO2_SLOPE
const  int32_t   SOP2_DEGLITCH_THRE = 100000;     //remove signal glitch over this threshold
const  int32_t   SPO2_REMOVE_JUMP_THRE = 50000;  //remove signal jump over this threshold
const  uint32_t  SPO2_SLOPE = 50000;              //increase this slope, spo2 reduce more
const  uint16_t  SPO2_LOW_CLIP_END_TIME = 1500;   //low clip mode before this data_cnt, normal clip mode after this
const  uint16_t  SPO2_LOW_CLIP_DN  = 150;         //spo2 reduce 0.15/s at most in low clip mode
const  uint16_t  SPO2_NORMAL_CLIP_DN  = 500;      //spo2 reduce 0.5/s at most in normal clip mode
const  uint8_t   SPO2_LOW_SNR_THRE = 40;          //snr below this threshold, means error signal
const  uint16_t  IR_AC_TOUCH_THRE = 200;          //AC_min*0.3
const  uint16_t  IR_FFT_POW_THRE = 500;           //fft_pow_min
const  uint16_t  HRV_LENGTH = 128; 
const  uint16_t  STATIC_THRE = 150;
#ifdef MALLOC_MEMORY 
void *hx_malloc(size_t size)
{
    return (void*)malloc(size);
}

void hx_free(void * ptr)
{
    free(ptr);
}
#endif

void hx3690l_delay_us(uint32_t us)
{
//    nrf_delay_us(us);
	hr_os_api_t *os_api = hr_algo_get_os_api();
	os_api->dbgOutput("hx3690l_delay_us %d \r\n",us);
	if (os_api->us_delay != NULL) {
		os_api->us_delay((int32_t)us);
	}
}

void hx3690l_delay(uint32_t ms)
{
//    nrf_delay_ms(ms);
	hr_os_api_t *os_api = hr_algo_get_os_api();
	os_api->dbgOutput("hx3690l_delay %d \r\n",ms);
	if (os_api->ms_delay != NULL) {
		os_api->ms_delay((int32_t)ms);
	}
}


bool hx3690l_write_reg(uint8_t addr, uint8_t data) 
{
//    uint8_t data_buf[2];    
//    data_buf[0] = addr;
//    data_buf[1] = data;
//    twi_pin_switch(1);
//    twi_master_transfer(0x88, data_buf, 2, true);    //write   
  hr_os_api_t *os_api = hr_algo_get_os_api();

  if (os_api->i2c_write != NULL) {
    os_api->i2c_write(5, addr, &data, 1);
  } 
    return true;      
}

uint8_t hx3690l_read_reg(uint8_t addr) 
{
    uint8_t data_buf;    
//    twi_pin_switch(1);
//    twi_master_transfer(0x88, &addr, 1, false);      //write
//    twi_master_transfer(0x89, &data_buf, 1, true);//read
	hr_os_api_t *os_api = hr_algo_get_os_api();
	
	if (os_api->i2c_read != NULL) {
		os_api->i2c_read(5, addr, &data_buf, 1);
	}
    return data_buf;      
}

bool hx3690l_brust_read_reg(uint8_t addr , uint8_t *buf, uint8_t length) 
{
//    twi_pin_switch(1);
//    twi_master_transfer(0x88, &addr, 1, false);      //write
//    twi_master_transfer(0x89, buf, length, true); //read

	hr_os_api_t *os_api = hr_algo_get_os_api();
	
	if (os_api->i2c_read != NULL) {
		os_api->i2c_read(5, addr, buf, length);
	}
    return true;      
}
uint8_t chip_id = 0;
bool hx3690l_chip_check(void)
{
    uint8_t i = 0;
    
    for(i=0;i<10;i++)
    {
        hx3690l_write_reg(0x02, 0x30);
        hx3690l_delay(5);
        chip_id = hx3690l_read_reg(0x00);  
        if (chip_id == 0x69)
        {
            AGC_LOG("r0x3E =0x%x\r\n",hx3690l_read_reg(0x3e));
            AGC_LOG("r0x3F =0x%x\r\n",hx3690l_read_reg(0x3f));
            AGC_LOG("r0x40 =0x%x\r\n",hx3690l_read_reg(0x40));
            AGC_LOG("r0x41 =0x%x\r\n",hx3690l_read_reg(0x41));
            return true;
        }        
    }

    return false;
}
uint8_t hx3690l_read_fifo_size(void) // 20200615 ericy read fifo data number
{
    uint8_t fifo_num_temp = 0;
    fifo_num_temp = hx3690l_read_reg(0x14)&0x7f;

    return fifo_num_temp;
}

void hx3690l_vin_check(uint16_t led_vin)
{
    low_vbat_flag = 0;
    if(led_vin < 3700)
    {
      low_vbat_flag = 1;      
    }
}

void hx3690l_ppg_off(void) // 20200615 ericy chip sleep enable
{
    hx3690l_write_reg(0X6a, 0x00);
    hx3690l_delay(5);  
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0x02, 0x31);
}

void hx3690l_ppg_on(void)
{
    //hx3690l_write_reg(0x6a, 0x00);   //reset fifo
    hx3690l_write_reg(0x02, 0x30);
}

#ifdef HRS_ALG_LIB
void hx3690l_hrs_ppg_init(void) //20200615 ericy ppg fs=25hz, phase3 conversion ready interupt en
{
    uint16_t sample_rate = 25; /*config the data rate of chip alps2_fm ,uint is Hz*/

    uint32_t prf_clk_num = 32000/sample_rate;   /*period in clk num, num = Fclk/fs */

    uint8_t cic_mode_en =0;
    uint8_t cic_b2_en = 0;
    uint8_t samp_delay_leden_num = 0; /* 00=8,01=16,10=32,11=64*/
    uint8_t samp_copy_avg = 0;        /* 0=1, 1=2, 2=4 ,3=8, 4=16*/
    uint8_t data_avg_num = 0;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
    uint8_t phase3_4_internal = 0;    /* phase3 and phase4 prf internal cfg */

    uint8_t phase1_enable = 1;     /*phase1_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase2_enable = 1;     /*phase2_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase3_enable = 1;     /*phase3_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase4_enable = 1;     /*phase4_enable  , 1 mean enable ; 0 mean disable */

    uint8_t phase1_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase2_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase3_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase4_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    //green
    uint8_t phase1_inner_avg = 0;   /* phase1 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase1_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_ldr_sel = GREEN_LED_SLE;     /*ball led 1 = ldr1(red); 2 = ldr2(ir); 4 = ldr3(green); 8 = ldr4 ;
                                    * 3in1 led 1 = ldr1(red); 2 = ldr2(green); 4 = ldr3(ir); 8 = ldr4 ;
                                    * 205U led 1 = ldr1(green); 2 = ldr2(red); 4 = ldr3(ir); 8 = ldr4 ;
                                    * GT01 led 1 = ldr1(green); 2 = ldr2(IR); 4 = ldr3(red); 8 = ldr4 ;
                                    */
    uint8_t phase1_pd_sel = 1;      /* 1 = pd1; 2 = pd2; */
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase1_led_en = 1;      /* phase1 led enable*/
    //als(green)
    uint8_t phase2_inner_avg = 0;   /* phase2 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase2_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase2_ldr_sel = 0;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase2_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase2_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase2_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase2_led_en = 0;      /* phase2 led enable*/
    //ir
    uint8_t phase3_inner_avg = 0;   /* phase3 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase3_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase3_ldr_sel = IR_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase3_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase3_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase3_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase3_led_en = 1;      /* phase3 led enable*/
    //als(ir)
    uint8_t phase4_inner_avg = 0;   /* phase4 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase4_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase4_ldr_sel = 0;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase4_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase4_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase4_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase4_led_en = 0;      /* phase4 led enable*/

    uint8_t init_wait_delay = 5 ; /* 0 = 31clk ; 1 = 64clk ; 2 = 127clk ; 3 = 255clk(d) ;
                                     4 = 511clk; 5 = 1023clk; 6 = 2047; 7 = 2048clk */

    uint8_t afe_reset = 3;        /* 0 = 15clk ; 1 = 31clk ; 2 = 63clk ; 3 = 127clk(d) ;
                                     4 = 255clk; 5 = 511clk; 6 = 1024; 7 = 2048clk */

    uint8_t led_on_time = 2;      /* 0 = 32clk=8us ; 1 = 64clk=16us; 2=128clk=32us ; 3 = 256clk=64us ;
                                     4 = 512clk=128us ; 5 = 1024clk=256us; 6= 2048clk=512us; 7 = 4096clk=1024us */
    hx3690l_write_reg(0x02, 0x31);
    hx3690l_delay(10);
    hx3690l_write_reg(0x02, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X6a, 0X00);	//rest int
    hx3690l_delay(5);
    
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>

    hx3690l_write_reg(0X1d, phase3_4_internal);
    hx3690l_write_reg(0X1e, ((afe_reset<<3)| 0x00) );
    hx3690l_write_reg(0X1f, (led_on_time<<4| phase1_led_en<<3 | phase3_led_en<<2 | phase4_led_en<<1 | phase2_led_en) );

    hx3690l_write_reg(0X26, (init_wait_delay<<4 | 0x0f));
    hx3690l_write_reg(0X27, (phase1_inner_avg | (phase2_inner_avg<<4)));
    hx3690l_write_reg(0X28, (phase3_inner_avg | (phase4_inner_avg<<4)));
    hx3690l_write_reg(0X29, cic_mode_en<<7 | cic_b2_en<<6 | samp_delay_leden_num<<4 | samp_copy_avg);

    hx3690l_write_reg(0X2c, phase1_tia_res);
    hx3690l_write_reg(0X2d, phase3_tia_res);
    hx3690l_write_reg(0X2e, phase4_tia_res);
    hx3690l_write_reg(0X2f, phase2_tia_res);

    hx3690l_write_reg(0X30, phase1_ldr_cur);
    hx3690l_write_reg(0X31, phase3_ldr_cur);
    hx3690l_write_reg(0X32, phase4_ldr_cur);
    hx3690l_write_reg(0X33, phase2_ldr_cur);

    hx3690l_write_reg(0X34, (phase1_pd_sel<<4 |  phase1_ldr_sel));
    hx3690l_write_reg(0X35, (phase3_pd_sel<<4 |  phase3_ldr_sel));
    hx3690l_write_reg(0X36, (phase4_pd_sel<<4 |  phase4_ldr_sel));
    hx3690l_write_reg(0X37, (phase2_pd_sel<<4 |  phase2_ldr_sel));

    hx3690l_write_reg(0X38, phase1_offset_idac);
    hx3690l_write_reg(0X39, phase3_offset_idac);
    hx3690l_write_reg(0X3a, phase4_offset_idac);
    hx3690l_write_reg(0X3b, phase2_offset_idac);
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );

    hr_os_api_t *os_api = hr_algo_get_os_api();


    

// analog circuit cfg
    hx3690l_write_reg(0X60, 0x0a);	//1a= adc self test
    hx3690l_write_reg(0X66, 0x92);	//0x92= r2r idac en; 0x91= mos idac en; 0x93= two idac en;
    hx3690l_write_reg(0X67, 0xbf);	//32k osc cfg relate
    hx3690l_write_reg(0X69, 0xa0);	//bit<0>: rc_comb_en bits<1>=rc_rbp_en bits<7>= vcom_clamp_en bits<6:4>= LED_vdesl
	  os_api->dbgOutput("HX3690 R 0X60 : %x \r\n",hx3690l_read_reg(0X60));
    os_api->dbgOutput("HX3690 R 0X66 : %x \r\n",hx3690l_read_reg(0X66));
    os_api->dbgOutput("HX3690 R 0X67 : %x \r\n",hx3690l_read_reg(0X67));
    os_api->dbgOutput("HX3690 R 0X69 : %x \r\n",hx3690l_read_reg(0X69));
/////////FIFO and adc conversion ready config////////
    hx3690l_write_reg(0X12, 0x20);   // fifo almostfull cfg ,max=0x40;
  #if defined(INT_MODE)
    hx3690l_write_reg(0X13, 0x30);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
  	hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x20);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel 
  #elif defined(FIFO_ALL_MOST_FULL)
    hx3690l_write_reg(0X13, 0x31);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
  	hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x00);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x20);   // fifo int output sel
  #else
    hx3690l_write_reg(0X13, 0x31);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x20);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
  #endif

///////FIFO//////////

    hx3690l_write_reg(0X18,(phase1_enable<<3)|(phase1_adc_osr)|(phase3_enable<<7)|(phase3_adc_osr<<4) );
    hx3690l_write_reg(0X19,(phase4_enable<<3)|(phase4_adc_osr)|(phase2_enable<<7)|(phase2_adc_osr<<4) );

  #ifdef INT_MODE
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);    
    hx3690l_write_reg(0X51, 0x00);
  #else
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x31);	
    hx3690l_write_reg(0X51, 0x00);
  #endif
	hx3690l_write_reg(0X6a, 0X02);	//02= u_low_pow, INT cmos output
}
#endif

#ifdef SPO2_ALG_LIB
void hx3690l_spo2_ppg_init(void) //20200615 ericy ppg fs=25hz, phase3 conversion ready interupt en
{
    uint16_t sample_rate = 50; /*config the data rate of chip alps2_fm ,uint is Hz*/

    uint32_t prf_clk_num = 32000/sample_rate;   /*period in clk num, num = Fclk/fs */

    uint8_t cic_mode_en =0;
    uint8_t cic_b2_en = 0;
    uint8_t samp_delay_leden_num = 0; /* 00=8,01=16,10=32,11=64*/
    uint8_t samp_copy_avg = 0;        /* 0=1, 1=2, 2=4 ,3=8, 4=16*/
    uint8_t data_avg_num = 1;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
    uint8_t phase3_4_internal = 0;    /* phase3 and phase4 prf internal cfg */

    uint8_t phase1_enable = 1;     /*phase1_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase2_enable = 0;     /*phase2_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase3_enable = 1;     /*phase3_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase4_enable = 1;     /*phase4_enable  , 1 mean enable ; 0 mean disable */

    uint8_t phase1_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase2_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase3_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase4_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    //red
    uint8_t phase1_inner_avg = 0;   /* phase1 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase1_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_ldr_sel = RED_LED_SLE;     /*ball led 1 = ldr1(red); 2 = ldr2(ir); 4 = ldr3(green); 8 = ldr4 ;
                                    * 3in1 led 1 = ldr1(red); 2 = ldr2(green); 4 = ldr3(ir); 8 = ldr4 ;
                                    * 205U led 1 = ldr1(green); 2 = ldr2(red); 4 = ldr3(ir); 8 = ldr4 ;
                                    * GT01 led 1 = ldr1(green); 2 = ldr2(IR); 4 = ldr3(red); 8 = ldr4 ;
                                    */
    uint8_t phase1_pd_sel = 1;      /* 1 = pd1; 2 = pd2; */
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase1_led_en = 1;      /* phase1 led enable*/
    //no use
    uint8_t phase2_inner_avg = 0;   /* phase2 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase2_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase2_ldr_sel = 0;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase2_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase2_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase2_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase2_led_en = 0;      /* phase2 led enable*/
    //als(for red and ir)
    uint8_t phase3_inner_avg = 0;   /* phase3 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase3_tia_res = 6;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase3_ldr_sel = 0;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase3_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase3_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase3_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase3_led_en = 0;      /* phase3 led enable*/
    //IR
    uint8_t phase4_inner_avg = 0;   /* phase4 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase4_tia_res = 6;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase4_ldr_sel = IR_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase4_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase4_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase4_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase4_led_en = 1;      /* phase4 led enable*/

    uint8_t init_wait_delay = 5 ; /* 0 = 31clk ; 1 = 64clk ; 2 = 127clk ; 3 = 255clk(d) ;
                                     4 = 511clk; 5 = 1023clk; 6 = 2047; 7 = 2048clk */

    uint8_t afe_reset = 4;        /* 0 = 15clk ; 1 = 31clk ; 2 = 63clk ; 3 = 127clk(d) ;
                                     4 = 255clk; 5 = 511clk; 6 = 1024; 7 = 2048clk */

    uint8_t led_on_time = 3;      /* 0 = 32clk=8us ; 1 = 64clk=16us; 2=128clk=32us ; 3 = 256clk=64us ;
                                     4 = 512clk=128us ; 5 = 1024clk=256us; 6= 2048clk=512us; 7 = 4096clk=1024us */
    hx3690l_write_reg(0x02, 0x31);
    hx3690l_delay(10);
    hx3690l_write_reg(0x02, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X6a, 0X00);	//rest int
    hx3690l_delay(5);

    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>

    hx3690l_write_reg(0X1d, phase3_4_internal);
    hx3690l_write_reg(0X1e, ((afe_reset<<3)| 0x00) );
    hx3690l_write_reg(0X1f, (led_on_time<<4| phase1_led_en<<3 | phase3_led_en<<2 | phase4_led_en<<1 | phase2_led_en) );

    hx3690l_write_reg(0X26, (init_wait_delay<<4 | 0x0f));
    hx3690l_write_reg(0X27, (phase1_inner_avg | (phase2_inner_avg<<4)));
    hx3690l_write_reg(0X28, (phase3_inner_avg | (phase4_inner_avg<<4)));
    hx3690l_write_reg(0X29, cic_mode_en<<7 | cic_b2_en<<6 | samp_delay_leden_num<<4 | samp_copy_avg);

    hx3690l_write_reg(0X2c, phase1_tia_res);
    hx3690l_write_reg(0X2d, phase3_tia_res);
    hx3690l_write_reg(0X2e, phase4_tia_res);
    hx3690l_write_reg(0X2f, phase2_tia_res);

    hx3690l_write_reg(0X30, phase1_ldr_cur);
    hx3690l_write_reg(0X31, phase3_ldr_cur);
    hx3690l_write_reg(0X32, phase4_ldr_cur);
    hx3690l_write_reg(0X33, phase2_ldr_cur);

    hx3690l_write_reg(0X34, (phase1_pd_sel<<4 |  phase1_ldr_sel));
    hx3690l_write_reg(0X35, (phase3_pd_sel<<4 |  phase3_ldr_sel));
    hx3690l_write_reg(0X36, (phase4_pd_sel<<4 |  phase4_ldr_sel));
    hx3690l_write_reg(0X37, (phase2_pd_sel<<4 |  phase2_ldr_sel));

    hx3690l_write_reg(0X38, phase1_offset_idac);
    hx3690l_write_reg(0X39, phase3_offset_idac);
    hx3690l_write_reg(0X3a, phase4_offset_idac);
    hx3690l_write_reg(0X3b, phase2_offset_idac);
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );

// analog circuit cfg
    hx3690l_write_reg(0X60, 0x0a);	//1a= adc self test
    hx3690l_write_reg(0X66, 0x92);	//0x92= r2r idac en; 0x91= mos idac en; 0x93= two idac en;
    hx3690l_write_reg(0X67, 0xbf);	//32k osc cfg relate
    hx3690l_write_reg(0X69, 0xa0);	//bit<0>: rc_comb_en bits<1>=rc_rbp_en bits<7>= vcom_clamp_en bits<6:4>= LED_vdesl

/////////FIFO and adc conversion ready config////////    
    hx3690l_write_reg(0X12, 0x18);   // fifo almostfull cfg ,max=0x40;
  #if defined(INT_MODE)
    hx3690l_write_reg(0X13, 0x30);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x20);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
  #elif defined(FIFO_ALL_MOST_FULL)
    hx3690l_write_reg(0X13, 0x31);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x00);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x20);   // fifo int output sel
  #else
    hx3690l_write_reg(0X13, 0x31);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x20);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
  #endif
///////FIFO//////////

    hx3690l_write_reg(0X18,(phase1_enable<<3)|(phase1_adc_osr)|(phase3_enable<<7)|(phase3_adc_osr<<4) );
    hx3690l_write_reg(0X19,(phase4_enable<<3)|(phase4_adc_osr)|(phase2_enable<<7)|(phase2_adc_osr<<4) );

  #ifdef INT_MODE
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X51, 0x00);
  #else
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x31);
    hx3690l_write_reg(0X51, 0x00);
  #endif   
	hx3690l_write_reg(0X6a, 0X02);	//02= u_low_pow, INT cmos output
}
#endif

#ifdef HRV_ALG_LIB
void hx3690l_hrv_ppg_init(void) //20200615 ericy ppg fs=25hz, phase3 conversion ready interupt en
{
    uint16_t sample_rate = 125; /*config the data rate of chip alps2_fm ,uint is Hz*/

    uint32_t prf_clk_num = 32000/sample_rate;   /*period in clk num, num = Fclk/fs */

    uint8_t cic_mode_en =0;
    uint8_t cic_b2_en = 0;
    uint8_t samp_delay_leden_num = 0; /* 00=8,01=16,10=32,11=64*/
    uint8_t samp_copy_avg = 0;        /* 0=1, 1=2, 2=4 ,3=8, 4=16*/
    uint8_t data_avg_num = 0;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
    uint8_t phase3_4_internal = 0;    /* phase3 and phase4 prf internal cfg */

    uint8_t phase1_enable = 1;     /*phase1_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase2_enable = 0;     /*phase2_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase3_enable = 1;     /*phase3_enable  , 1 mean enable ; 0 mean disable */
    uint8_t phase4_enable = 1;     /*phase4_enable  , 1 mean enable ; 0 mean disable */

    uint8_t phase1_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase2_adc_osr = 0;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase3_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    uint8_t phase4_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
    //green
    uint8_t phase1_inner_avg = 0;   /* phase1 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase1_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_ldr_sel = GREEN_LED_SLE;     /*ball led 1 = ldr1(red); 2 = ldr2(ir); 4 = ldr3(green); 8 = ldr4 ;
                                    * 3in1 led 1 = ldr1(red); 2 = ldr2(green); 4 = ldr3(ir); 8 = ldr4 ;
                                    * 205U led 1 = ldr1(green); 2 = ldr2(red); 4 = ldr3(ir); 8 = ldr4 ;
                                    * GT01 led 1 = ldr1(green); 2 = ldr2(IR); 4 = ldr3(red); 8 = ldr4 ;
                                    */
    uint8_t phase1_pd_sel = 1;      /* 1 = pd1; 2 = pd2; */
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase1_led_en = 1;      /* phase1 led enable*/
    //als(green)
    uint8_t phase2_inner_avg = 0;   /* phase2 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase2_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase2_ldr_sel = 0;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase2_pd_sel = 0;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase2_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase2_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase2_led_en = 0;      /* phase2 led enable*/
    //ir
    uint8_t phase3_inner_avg = 0;   /* phase3 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase3_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase3_ldr_sel = IR_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase3_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase3_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase3_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase3_led_en = 1;      /* phase3 led enable*/
    //als(ir)
    uint8_t phase4_inner_avg = 0;   /* phase4 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase4_tia_res = 0;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase4_ldr_sel = 0;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase4_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase4_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase4_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase4_led_en = 0;      /* phase4 led enable*/

    uint8_t init_wait_delay = 5 ; /* 0 = 31clk ; 1 = 64clk ; 2 = 127clk ; 3 = 255clk(d) ;
                                     4 = 511clk; 5 = 1023clk; 6 = 2047; 7 = 2048clk */

    uint8_t afe_reset = 4;        /* 0 = 15clk ; 1 = 31clk ; 2 = 63clk ; 3 = 127clk(d) ;
                                     4 = 255clk; 5 = 511clk; 6 = 1024; 7 = 2048clk */

    uint8_t led_on_time = 3;      /* 0 = 32clk=8us ; 1 = 64clk=16us; 2=128clk=32us ; 3 = 256clk=64us ;
                                     4 = 512clk=128us ; 5 = 1024clk=256us; 6= 2048clk=512us; 7 = 4096clk=1024us */
    hx3690l_write_reg(0x02, 0x31);
    hx3690l_delay(10);
		hx3690l_write_reg(0x02, 0x30);
    hx3690l_delay(5);
    hx3690l_write_reg(0X6a, 0X00);	//rest int
    hx3690l_delay(5);
    
    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>

    hx3690l_write_reg(0X1d, phase3_4_internal);
    hx3690l_write_reg(0X1e, ((afe_reset<<3)| 0x00) );
    hx3690l_write_reg(0X1f, (led_on_time<<4| phase1_led_en<<3 | phase3_led_en<<2 | phase4_led_en<<1 | phase2_led_en) );

    hx3690l_write_reg(0X26, (init_wait_delay<<4 | 0x0f));
    hx3690l_write_reg(0X27, (phase1_inner_avg | (phase2_inner_avg<<4)));
    hx3690l_write_reg(0X28, (phase3_inner_avg | (phase4_inner_avg<<4)));
    hx3690l_write_reg(0X29, cic_mode_en<<7 | cic_b2_en<<6 | samp_delay_leden_num<<4 | samp_copy_avg);

    hx3690l_write_reg(0X2c, phase1_tia_res);
    hx3690l_write_reg(0X2d, phase3_tia_res);
    hx3690l_write_reg(0X2e, phase4_tia_res);
    hx3690l_write_reg(0X2f, phase2_tia_res);

    hx3690l_write_reg(0X30, phase1_ldr_cur);
    hx3690l_write_reg(0X31, phase3_ldr_cur);
    hx3690l_write_reg(0X32, phase4_ldr_cur);
    hx3690l_write_reg(0X33, phase2_ldr_cur);

    hx3690l_write_reg(0X34, (phase1_pd_sel<<4 |  phase1_ldr_sel));
    hx3690l_write_reg(0X35, (phase3_pd_sel<<4 |  phase3_ldr_sel));
    hx3690l_write_reg(0X36, (phase4_pd_sel<<4 |  phase4_ldr_sel));
    hx3690l_write_reg(0X37, (phase2_pd_sel<<4 |  phase2_ldr_sel));

    hx3690l_write_reg(0X38, phase1_offset_idac);
    hx3690l_write_reg(0X39, phase3_offset_idac);
    hx3690l_write_reg(0X3a, phase4_offset_idac);
    hx3690l_write_reg(0X3b, phase2_offset_idac);
    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );

// analog circuit cfg
    hx3690l_write_reg(0X60, 0x0a);	//1a= adc self test
    hx3690l_write_reg(0X66, 0x92);	//0x92= r2r idac en; 0x91= mos idac en; 0x93= two idac en;
    hx3690l_write_reg(0X67, 0xbf);	//32k osc cfg relate
    hx3690l_write_reg(0X69, 0xa0);	//bit<0>: rc_comb_en bits<1>=rc_rbp_en bits<7>= vcom_clamp_en bits<6:4>= LED_vdesl

/////////FIFO and adc conversion ready config////////
  #if defined(HRV_INT_MODE)
    hx3690l_write_reg(0X13, 0x30);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
  	hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x20);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel 
  #elif defined(FIFO_ALL_MOST_FULL)
		hx3690l_write_reg(0X12, 0x28);   // fifo almostfull cfg ,max=0x40;
    hx3690l_write_reg(0X13, 0x11);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
  	hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x00);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x20);   // fifo int output sel
  #else
		hx3690l_write_reg(0X12, 0x40);   // fifo almostfull cfg ,max=0x40;
    hx3690l_write_reg(0X13, 0x11);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x20);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
  #endif

///////FIFO//////////

    hx3690l_write_reg(0X18,(phase1_enable<<3)|(phase1_adc_osr)|(phase3_enable<<7)|(phase3_adc_osr<<4) );
    hx3690l_write_reg(0X19,(phase4_enable<<3)|(phase4_adc_osr)|(phase2_enable<<7)|(phase2_adc_osr<<4) );

  #ifdef HRV_INT_MODE
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X51, 0x00);
  #else
    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x10);
    hx3690l_delay(5);
    hx3690l_write_reg(0X13, 0x11);	
    hx3690l_write_reg(0X51, 0x00);
  #endif
	  hx3690l_write_reg(0X6a, 0X02);	//02= u_low_pow, INT cmos output
}
#endif

bool timer_40ms_en = 0;
bool timer_320ms_en = 0;
uint8_t timer_320ms_cnt = 0;

void hx3690l_320ms_timer_cfg(bool en)
{
    // if(en)
    // {
    //     hx3690_timers_start();
    // }
    // else
    // {
    //     hx3690_timers_stop();
    // }
  timer_320ms_en = en;
	timer_320ms_cnt = 0;
  hr_os_api_t *os_api = hr_algo_get_os_api();
	os_api->dbgOutput("320ms %d \r\n",en);
}
void hx3690l_40ms_timer_cfg(bool en)
{
    // if(en)
    // {
        // #if defined(GSENSER_DATA)||!defined(EXT_INT_AGC)  
        // gsen_read_timers_start();   
        // #endif 
    // }
    // else
    // {
        // #if defined(GSENSER_DATA)||!defined(EXT_INT_AGC)  
        // gsen_read_timers_stop();   
        // #endif 
    // }
    timer_40ms_en = en;
    hr_os_api_t *os_api = hr_algo_get_os_api();
	  os_api->dbgOutput("40ms %d \r\n",en);
}
void hx3690l_gpioint_cfg(bool en)
{
    if(en)
    {
        //  hx3690_gpioint_enable();
    }
    else
    {
        //  hx3690_gpioint_disable();
    }
}
    
bool hx3690l_init(WORK_MODE_T mode)
{
    work_mode_flag = mode;//HRS_MODE,SPO2_MODE
    
    hx3690l_vin_check(3800);
    if(work_mode_flag == HRS_MODE || work_mode_flag == LIVING_MODE)
    {
        #ifdef HRS_ALG_LIB
        #ifdef SPO2_ALG_LIB
        hx3690l_spo2_disable();
				hx3690l_hrs_disable();
        #endif

        if(hx3690l_hrs_enable()== SENSOR_OP_FAILED) 
        {
            return false;
        }
        #endif
    }
    else  
    if(work_mode_flag == SPO2_MODE)
    {
        #ifdef SPO2_ALG_LIB
        #ifdef HRS_ALG_LIB
        hx3690l_spo2_disable();
				hx3690l_hrs_disable();
        #endif
        if(hx3690l_spo2_enable()== SENSOR_OP_FAILED) 
        {
            return false;
        }
        #endif

    }
		else
    if(work_mode_flag == HRV_MODE)
    {
        #ifdef HRV_ALG_LIB
        #ifdef SPO2_ALG_LIB
        hx3690_spo2_alg_close();
        #endif

			  #ifdef HRS_ALG_LIB
        hx3690_alg_close();
        #endif

        if(hx3690l_hrv_enable()== SENSOR_OP_FAILED) 
        {
            return false;
        }
        #endif
    }

    else 
    if(work_mode_flag == WEAR_MODE)
    {
        if(hx3690l_check_touch_enable()== SENSOR_OP_FAILED) 
        {
            return false;
        }
    }
    else 
    if(work_mode_flag == FT_LEAK_LIGHT_MODE)
    {
        hx3690l_factroy_test(LEAK_LIGHT_TEST);
    }
    else 
    if(work_mode_flag == FT_GRAY_CARD_MODE)
    {
        hx3690l_factroy_test(GRAY_CARD_TEST);
    }
	#ifdef LAB_TEST_INT_MODE	
	else
	if(work_mode_flag == LAB_TEST_MODE)
	{
		 hx3690l_lab_test(LAB_TEST_MODE);
	}
	#endif
    
    return true;
}
//I/O interrupt falling edge
void hx3690l_agc_Int_handle(void)
{       
#ifdef EXT_INT_AGC
    
    if(work_mode_flag == HRS_MODE || work_mode_flag == LIVING_MODE)
    {   
        #ifdef HRS_ALG_LIB
        HRS_CAL_SET_T cal;
        cal = PPG_hrs_agc();
        
        if(cal.work)
        {
            AGC_LOG("AGC: led_drv=%d,ledDac=%d,ambDac=%d,ledstep=%d,rf=%d,\r\n",
            cal.LED, cal.LEDDAC, cal.AMBDAC,cal.led_step,cal.RF);
        }
        #endif
    }
		else
    if(work_mode_flag == HRV_MODE)
    {   
        #ifdef HRV_ALG_LIB
        HRV_CAL_SET_T cal;
        cal = PPG_hrv_agc();
        
        if(cal.work)
        {
            AGC_LOG("AGC: led_drv=%d,ledDac=%d,ambDac=%d,ledstep=%d,rf=%d,\r\n",
            cal.LED, cal.LEDDAC, cal.AMBDAC,cal.led_step,cal.RF);
        }
        #endif
    }

    else  
    if(work_mode_flag == SPO2_MODE)
    {
        #ifdef SPO2_ALG_LIB 
        SPO2_CAL_SET_T cal;
        cal = PPG_spo2_agc();
        
        if(cal.work)
        {
            AGC_LOG("AGC. Rled_drv=%d,Irled_drv=%d,RledDac=%d,IrledDac=%d,ambDac=%d,Rledstep=%d,Irledstep=%d,Rrf=%d,Irrf=%d,\r\n",\
            cal.R_LED, cal.IR_LED,cal.R_LEDDAC,cal.IR_LEDDAC,cal.AMBDAC,\
            cal.R_led_step,cal.IR_led_step,cal.R_RF,cal.IR_RF);
        }
        #endif
    }   
        
#endif 

#ifdef LAB_TEST_INT_MODE		
		if(work_mode_flag == LAB_TEST_MODE)// add ericy 20210127
		{
			 hx3690l_lab_test_read();
		}
#endif
		
}
//Timer interrupt 40ms repeat mode
void gsen_read_timeout_handler(void * p_context)
{
#ifndef EXT_INT_AGC
    if(work_mode_flag == HRS_MODE || work_mode_flag == LIVING_MODE)
    {
        #ifdef HRS_ALG_LIB
        HRS_CAL_SET_T cal;
        cal = PPG_hrs_agc();
        
        if(cal.work)
        {
            AGC_LOG("AGC: led_drv=%d,ledDac=%d,ambDac=%d,ledstep=%d,rf=%d,\r\n",
            cal.LED, cal.LEDDAC, cal.AMBDAC,cal.led_step,cal.RF);
        }
        #endif
    }
    else  
		if(work_mode_flag == HRV_MODE)
    {
        #ifdef HRV_ALG_LIB
        HRV_CAL_SET_T cal;
        cal = PPG_hrv_agc();
        
        if(cal.work)
        {
            AGC_LOG("AGC: led_drv=%d,ledDac=%d,ambDac=%d,ledstep=%d,rf=%d,\r\n",
            cal.LED, cal.LEDDAC, cal.AMBDAC,cal.led_step,cal.RF);
        }
        #endif
    }
    else  
    if(work_mode_flag == SPO2_MODE)
    {
        #ifdef SPO2_ALG_LIB 
        SPO2_CAL_SET_T cal;
        cal = PPG_spo2_agc();
        
        if(cal.work)
        {
            AGC_LOG("AGC. Rled_drv=%d,Irled_drv=%d,RledDac=%d,IrledDac=%d,ambDac=%d,Rledstep=%d,Irledstep=%d,Rrf=%d,Irrf=%d,\r\n",\
            cal.R_LED, cal.IR_LED,cal.R_LEDDAC,cal.IR_LEDDAC,cal.AMBDAC,\
            cal.R_led_step,cal.IR_led_step,cal.R_RF,cal.IR_RF);
        }
        #endif
    }

#endif 
		
		hx3690l_gesensor_Int_handle();

	#ifdef LAB_TEST_INT_MODE	
		if(work_mode_flag == LAB_TEST_MODE)// add ericy 20210127
		{
			 hx3690l_lab_test_read();
		}
	#endif
		
}

//Timer interrupt 320ms repeat mode
void heart_rate_meas_timeout_handler(void * p_context)
{ 
    if(work_mode_flag == HRS_MODE)
    {
        #ifdef HRS_ALG_LIB
        hx3690l_hrs_ppg_Int_handle();
        #endif
    }
    else       
    if(work_mode_flag == HRV_MODE)
    {
        #ifdef HRV_ALG_LIB
        hx3690l_hrv_ppg_Int_handle();
        #endif
    }
    else       
    if(work_mode_flag == SPO2_MODE)
    {
        #ifdef SPO2_ALG_LIB
        hx3690l_spo2_ppg_Int_handle();
        #endif
    }
    else   
    if(work_mode_flag == WEAR_MODE)
    {
        hx3690l_wear_ppg_Int_handle();
    }
	else   
    if(work_mode_flag == LIVING_MODE)
    {
        // hx3690l_living_Int_handle();
    }
}

void hx3690l_ppg_Int_handle(void)
{
  #if defined(INT_MODE)
    hx3690l_agc_Int_handle(); 
    if(work_mode_flag == HRS_MODE)
    {
        #ifdef HRS_ALG_LIB
        hx3690l_hrs_ppg_Int_handle();
        #endif
    }
    else       
    if(work_mode_flag == SPO2_MODE)
    {
        #ifdef SPO2_ALG_LIB
        hx3690l_spo2_ppg_Int_handle();
        #endif
    }
    else   
    if(work_mode_flag == WEAR_MODE)
    {
        hx3690l_wear_ppg_Int_handle();
    }
  #elif defined(FIFO_ALL_MOST_FULL)
    hx3690l_agc_Int_handle(); 
    if(work_mode_flag == HRS_MODE)
    {
        #ifdef HRS_ALG_LIB
        hx3690l_hrs_ppg_Int_handle();
        #endif
    }
    else 
    if(work_mode_flag == HRV_MODE)
    {
        #ifdef HRV_ALG_LIB
        hx3690l_hrv_ppg_Int_handle();
        #endif
    }
    else       
    if(work_mode_flag == SPO2_MODE)
    {
        #ifdef SPO2_ALG_LIB
        hx3690l_spo2_ppg_Int_handle();
        #endif
    }
    else   
    if(work_mode_flag == WEAR_MODE)
    {
        hx3690l_wear_ppg_Int_handle();
    }
  #else
    hx3690l_agc_Int_handle();    
  #endif
		
	#ifdef HRV_INT_MODE 
    if(work_mode_flag == HRV_MODE)
    {
        #ifdef HRV_ALG_LIB
        hx3690l_hrv_ppg_Int_handle();
        #endif
    }
	#endif
}


 #ifdef SPO2_DATA_CALI
 static int32_t red_data_fifo[4] = {0};
 static int32_t ir_data_fifo[4] = {0};
 static int32_t red_dc_temp = 0;
 static int32_t ir_dc_temp = 0;
 static int32_t red_data_pre = 0;
 static int32_t ir_data_pre = 0;
 static int32_t red_jump_delta = 0;
 static int32_t ir_jump_delta = 0;
 static int32_t cali_data_cnt = 0;


int32_t hx3690l_red_data_cali(int32_t red_new_raw_data)
{
    uint8_t ii;
    int32_t red_data_final;
    int32_t red_data_cali;
    hx3690_spo2_wear_msg_code_t touch_status;
    touch_status = hx3690_spo2_get_wear_status();
    if(touch_status == MSG_SPO2_NO_WEAR)
    {
      cali_data_cnt = 0;
      red_data_cali = red_new_raw_data;
      for(ii=0;ii<4;ii++)
      {
        red_data_fifo[ii] = 0;
      }
      red_dc_temp = 0;
    }
    else
    {
      for(ii=3;ii>0;ii--)
      {
          red_data_fifo[ii] = red_data_fifo[ii-1];
      }
      red_data_fifo[0] = red_new_raw_data; 
      if(cali_data_cnt>=25) 
      {
        if ((((red_data_fifo[1] - red_data_fifo[2]) > SOP2_DEGLITCH_THRE) && ((red_data_fifo[1] - red_new_raw_data) > SOP2_DEGLITCH_THRE)) || \
           (((red_data_fifo[1] - red_data_fifo[2]) < -SOP2_DEGLITCH_THRE) && ((red_data_fifo[1] - red_new_raw_data) < -SOP2_DEGLITCH_THRE)))
        { 
            red_new_raw_data = red_data_fifo[2]; 
            red_data_fifo[1] = red_data_fifo[2]; 
        }
        else
        {
            red_new_raw_data = red_data_fifo[1];
        }	
        if ((abs((red_new_raw_data - red_jump_delta) - red_data_pre) > SPO2_REMOVE_JUMP_THRE))
        {
            red_jump_delta = red_new_raw_data - red_data_pre;
        }  
        red_data_cali = red_new_raw_data - red_jump_delta; 
        red_data_pre = red_data_cali;
      }
      else
      {
        red_data_cali = red_data_fifo[1];
        red_data_pre = red_data_fifo[1];
      }
    }
    if(cali_data_cnt<=30)
    {
      red_dc_temp = red_data_cali;
      red_data_final = red_new_raw_data;
    }
    else
    {
      red_dc_temp = (red_dc_temp*31 + red_data_cali)>>5;
      red_data_final = red_data_cali - red_dc_temp + 2608*50*hx3690l_spo2_agc_red_idac;
    }
    return red_data_final;
}
int32_t hx3690l_ir_data_cali(int32_t ir_new_raw_data)
{
    uint8_t ii;
    int32_t ir_data_final;
    int32_t ir_data_cali;
    hx3690_spo2_wear_msg_code_t touch_status;
    touch_status = hx3690_spo2_get_wear_status();
    if(touch_status == MSG_SPO2_NO_WEAR)
    {
      cali_data_cnt = 0;
      ir_data_cali = ir_new_raw_data;
      for(ii=0;ii<4;ii++)
      {
        ir_data_fifo[ii] = 0;
      }
      ir_dc_temp = 0;
    }
    else
    {
      for(ii=3;ii>0;ii--)
      {
          ir_data_fifo[ii] = ir_data_fifo[ii-1];
      }
      ir_data_fifo[0] = ir_new_raw_data; 
      cali_data_cnt++;
      if(cali_data_cnt>=25) 
      {
        if ((((ir_data_fifo[1] - ir_data_fifo[2]) > SOP2_DEGLITCH_THRE) && ((ir_data_fifo[1] - ir_new_raw_data) > SOP2_DEGLITCH_THRE)) || \
           (((ir_data_fifo[1] - ir_data_fifo[2]) < -SOP2_DEGLITCH_THRE) && ((ir_data_fifo[1] - ir_new_raw_data) < -SOP2_DEGLITCH_THRE)))
        { 
            ir_new_raw_data = ir_data_fifo[2]; 
            ir_data_fifo[1] = ir_data_fifo[2]; 
        }
        else
        {
            ir_new_raw_data = ir_data_fifo[1];
        }	
        if ((abs((ir_new_raw_data - ir_jump_delta) - ir_data_pre) > SPO2_REMOVE_JUMP_THRE))
        {
            ir_jump_delta = ir_new_raw_data - ir_data_pre;
        }  
        ir_data_cali = ir_new_raw_data - ir_jump_delta; 
        ir_data_pre = ir_data_cali;
      }
      else
      {
        ir_data_cali = ir_data_fifo[1];
        ir_data_pre = ir_data_fifo[1];
      }
    }
    if(cali_data_cnt<=30)
    {
      ir_dc_temp = ir_data_cali;
      ir_data_final = ir_new_raw_data;
    }
    else
    {
      ir_dc_temp = (ir_dc_temp*31 + ir_data_cali)>>5;
      ir_data_final = ir_data_cali - ir_dc_temp + 2608*50*hx3690l_spo2_agc_ir_idac;
    }
    return ir_data_final;
}
#endif

void hx3690l_gesensor_Int_handle(void)
{
#ifdef GSENSER_DATA
    uint8_t ii = 0;
    AxesRaw_t gsen_buf;
    if(work_mode_flag == WEAR_MODE)
    {
        return;
    }
    
    LIS3DH_GetAccAxesRaw(&gsen_buf);

    for(ii=0;ii<9;ii++)
    {
        gsen_fifo_x[ii] = gsen_fifo_x[ii+1];
        gsen_fifo_y[ii] = gsen_fifo_y[ii+1];
        gsen_fifo_z[ii] = gsen_fifo_z[ii+1];
    }
    gsen_fifo_x[9] = gsen_buf.AXIS_X>>1;
    gsen_fifo_y[9] = gsen_buf.AXIS_Y>>1;
    gsen_fifo_z[9] = gsen_buf.AXIS_Z>>1;
    //SEGGER_RTT_printf(0,"gsen_x=%d gsen_y=%d gsen_z=%d\r\n", \
    gsen_fifo_x[9],gsen_fifo_y[9],gsen_fifo_z[9]);
#endif 
}

#ifdef HRS_ALG_LIB
hrs_sensor_data_t hrs_s_dat;
void hx3690l_hrs_ppg_Int_handle(void)
{
    uint8_t        ii=0;   
    hx3690_results_t alg_results = {MSG_HRS_ALG_NOT_OPEN,MSG_HRS_NO_WEAR,0,0,0,false};

    int32_t *PPG_buf = &(hrs_s_dat.ppg_data[0]);
    uint32_t *als = &(hrs_s_dat.als);
    int32_t *ir_buf = &(hrs_s_dat.ir_data[0]);
    uint8_t *count = &(hrs_s_dat.count);
    int32_t *s_buf = &(hrs_s_dat.s_buf[0]);
    int32_t phase_data[5];
    int32_t green_data = 0;
#ifdef BP_CUSTDOWN_ALG_LIB        
    hx3690_bp_results_t    bp_alg_results ;    
#endif   
    #ifdef HR_VECTOR
      for(ii=0;ii<8;ii++)
      {
        PPG_buf_vec[ii] = hrm_input_data[spo2_send_cnt+ii];
        gsen_fifo_x[ii] = gsen_input_data_x[spo2_send_cnt+ii];
        gsen_fifo_y[ii] = gsen_input_data_y[spo2_send_cnt+ii];
        gsen_fifo_z[ii] = gsen_input_data_z[spo2_send_cnt+ii];
      }
      spo2_send_cnt = spo2_send_cnt+8;
      *count = 8;
      hx3690_alg_send_data(PPG_buf_vec,*count, 0, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z); 
    #endif
    
    #ifndef HR_VECTOR
      #ifdef INT_MODE
      if(hx3690l_hrs_read_data_packet(phase_data) == NULL)
      {
          return;
      }
      DEBUG_PRINTF(0,"%d %d %d %d %d\r\n", phase_data[0], phase_data[1], phase_data[2], phase_data[3], phase_data[4]);
      #else
      if(hx3690l_hrs_read(&hrs_s_dat) == NULL)
      { 
          return;
      }
      for(ii=0;ii<*count;ii++)
      {
          //DEBUG_PRINTF(0,"%d/%d %d %d %d %d %d %d %d %d\r\n" ,1+ii,*count,\
          PPG_buf[ii],PPG_buf[ii],ir_buf[ii],s_buf[ii*4],s_buf[ii*4+1],s_buf[ii*4+2], \
          s_buf[ii*4+3],hrs_s_dat.agc_green);
         // SEGGER_RTT_printf(0,"%d %d %d %d %d %d\r\n", PPG_buf[ii],ir_buf[ii],gsen_fifo_x[ii],gsen_fifo_y[ii],gsen_fifo_z[ii],hrs_s_dat.agc_green);
      }
      #endif
      
      #ifdef INT_MODE
        green_data =  phase_data[0] - phase_data[1];
        #ifdef GSENSER_DATA
        hx3690_alg_send_data(&green_data, 1, 0, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z);   
        #else
        hx3690_alg_send_data(&green_data, 1, 0, gen_dummy, gen_dummy, gen_dummy);
        #endif 
      #else
        #ifdef GSENSER_DATA
        hx3690_alg_send_data(PPG_buf,*count, *als, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z);   
        #else
        hx3690_alg_send_data(PPG_buf,*count,*als, gen_dummy, gen_dummy, gen_dummy);
        #endif 
      #endif
    #endif      

    //display part                                                                                                                 
    alg_results = hx3690_alg_get_results();   

    hr_os_api_t *os_api = hr_algo_get_os_api();
    if(1)
      os_api->hb_handler(alg_results.hr_result, 0, 0);
    else
      os_api->hb_handler(0, 0, 0);
    // os_api->dbgOutput("hrs_wear_status %d \r\n",hrs_wear_status);
    os_api->dbgOutput("tyhx_hrs_alg_get_results %d \r\n",alg_results.hr_result);	
    //DEBUG_PRINTF(0,"====>>>>quality = %d\r\n", alg_results.hr_result_qual);     
#if 0		
    oled_dis.refresh_time++;
    if(oled_dis.refresh_time > 3) //330ms*3 = 990ms ~ 1s
    {
        oled_dis.refresh_flag = 1;
        oled_dis.refresh_time = 0;

        oled_dis.dis_mode = DIS_HR;  
        
        oled_dis.dis_data = alg_results.hr_result;

		#ifdef BP_CUSTDOWN_ALG_LIB                    
		bp_alg_results = hx3690_alg_get_bp_results();  
		if (bp_alg_results.sbp!= 0)
		{
			//opr_display_bp(bp_alg_results.sbp, bp_alg_results.dbp);
		}

		#endif   
    }
#endif   
 //   SEGGER_RTT_printf(0,"oledata: %d,oledstatus: %d\r\n", alg_results.hr_result,alg_results.hrs_wear_status);

    #ifdef HRS_BLE_APP
    {
        rawdata_vector_t rawdata;
        
        HRS_CAL_SET_T cal= get_hrs_agc_status();
        for(ii=0;ii<*count;ii++)
        {
            rawdata.vector_flag = HRS_VECTOR_FLAG;
            rawdata.data_cnt = alg_results.data_cnt-*count+ii;
            rawdata.hr_result = alg_results.hr_result;           
            rawdata.red_raw_data = PPG_buf[ii];
            rawdata.ir_raw_data = ir_buf[ii];
            rawdata.gsensor_x = gsen_fifo_x[ii];
            rawdata.gsensor_y = gsen_fifo_y[ii];
            rawdata.gsensor_z = gsen_fifo_z[ii];
            rawdata.red_cur = cal.LED;
            rawdata.ir_cur = alg_results.hrs_alg_status;
            
            ble_rawdata_vector_push(rawdata);   
        }
    }
    #endif    
}

#if 0
void hx3690l_living_Int_handle(void)
{
	  uint8_t        ii=0;   
    hx3690_living_results_t living_alg_results = {MSG_HRS_NO_WEAR,0,0,0};

    int32_t *PPG_buf = &(hrs_s_dat.ppg_data[0]);
    uint32_t *als = &(hrs_s_dat.als);
    int32_t *ir_buf = &(hrs_s_dat.ir_data[0]);
    uint8_t *count = &(hrs_s_dat.count);
    int32_t *s_buf = &(hrs_s_dat.s_buf[0]);
    int32_t phase_data[5];
    int32_t green_data = 0;
		if(hx3690l_hrs_read(&hrs_s_dat) == NULL)
		{
				
				return;
		}
		for(ii=0;ii<*count;ii++)
		{
				//DEBUG_PRINTF(0,"%d/%d %d %d %d %d %d %d %d %d\r\n" ,1+ii,*count,\
				PPG_buf[ii],PPG_buf[ii],ir_buf[ii],s_buf[ii*4],s_buf[ii*4+1],s_buf[ii*4+2], \
				s_buf[ii*4+3],hrs_s_dat.agc_green);
			 //SEGGER_RTT_printf(0,"%d/%d %d %d %d %d %d %d\r\n",1+ii,*count,PPG_buf[ii],ir_buf[ii],gsen_fifo_x[ii],gsen_fifo_y[ii],gsen_fifo_z[ii],hrs_s_dat.agc_green);
		}
		#ifdef GSENSER_DATA
		hx3690_living_send_data(PPG_buf,*count, *als, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z);   
		#else
		hx3690_living_send_data(PPG_buf,*count,*als, gen_dummy, gen_dummy, gen_dummy);
		#endif 
		living_alg_results = hx3690_living_get_results(); 
		DEBUG_PRINTF(0,"%d %d %d %d\r\n" ,living_alg_results.data_cnt,living_alg_results.motion_status,living_alg_results.signal_quality,living_alg_results.wear_status);
}
#endif
#endif


#ifdef SPO2_ALG_LIB
spo2_sensor_data_t spo2_s_dat;
void hx3690l_spo2_ppg_Int_handle(void)
{ 
    uint8_t        ii=0; 
    int32_t red_data_cali, ir_data_cali;
    hx3690_spo2_results_t alg_results = {MSG_SPO2_ALG_NOT_OPEN,MSG_SPO2_NO_WEAR,0,0,0,0,0};
    
    int32_t *red_buf = &(spo2_s_dat.red_data[0]);
    int32_t *ir_buf = &(spo2_s_dat.ir_data[0]);
    uint8_t *count = &(spo2_s_dat.count);
    int32_t *s_buf = &(spo2_s_dat.s_buf[0]);
    int32_t phase_data[6];
    int32_t red_data;
    int32_t ir_data;

    #ifdef SPO2_VECTOR
      for(ii=0;ii<8;ii++)
      {
        red_buf_vec[ii] = vec_red_data[spo2_send_cnt+ii];
        ir_buf_vec[ii] = vec_ir_data[spo2_send_cnt+ii];
      }
      spo2_send_cnt = spo2_send_cnt+8;
      *count = 8;
      for(ii=0;ii<10;ii++)
      {
        gsen_fifo_x[ii] = 0;
        gsen_fifo_y[ii] = 0;
        gsen_fifo_z[ii] = 0;
      }
      hx3690_spo2_alg_send_data(red_buf_vec, ir_buf_vec, *count, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z);
    #endif
      
    #ifndef SPO2_VECTOR
      #ifdef INT_MODE
        if(hx3690l_spo2_read_data_packet(phase_data) == NULL)
        {
            return;
        }
        //DEBUG_PRINTF(0,"%d %d %d %d %d %d\r\n", phase_data[0], phase_data[1], phase_data[2], phase_data[3], phase_data[4], phase_data[5]);
        red_data = phase_data[0]-phase_data[2];
        ir_data = phase_data[3]-phase_data[2];
        #ifdef GSENSER_DATA
        hx3690_spo2_alg_send_data(&red_data, &ir_data, 1, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z);   
        #else
        hx3690_spo2_alg_send_data(&red_data, &ir_data, 1, gen_dummy, gen_dummy, gen_dummy);
        #endif
      #else
        if(hx3690l_spo2_read(&spo2_s_dat) == NULL)
        {
            return;
        }
        for(ii=0;ii<*count;ii++)
        {
            //DEBUG_PRINTF(0,"%d/%d %d %d %d %d %d %d %d\r\n" ,1+ii,*count,\
            red_buf[ii],ir_buf[ii],s_buf[ii*3],s_buf[ii*3+1],s_buf[ii*3+2],\
            spo2_s_dat.agc_red,spo2_s_dat.agc_ir);
        }
        #ifdef GSENSER_DATA
        hx3690_spo2_alg_send_data(red_buf, ir_buf, *count, gsen_fifo_x, gsen_fifo_y, gsen_fifo_z);   
        #else
        hx3690_spo2_alg_send_data(red_buf, ir_buf, *count, gen_dummy, gen_dummy, gen_dummy);
        #endif 
      #endif 
    #endif      

    //display part
    alg_results = hx3690_spo2_alg_get_results(); 
	  DEBUG_PRINTF(0,"%d\r\n", alg_results.ir_quality);
#if 0
    oled_dis.refresh_time++;
    if(oled_dis.refresh_time > 3) //330ms*3 = 990ms ~ 1s
    {
        oled_dis.refresh_flag = 1;
        oled_dis.refresh_time = 0;
        oled_dis.dis_mode = DIS_SPO2;
        oled_dis.dis_data = alg_results.spo2_result;
    }
#endif          
    //SEGGER_RTT_printf(0,"oledata: %d,oledstatus: %d\r\n", alg_results.hr_result,alg_results.alg_status);

    #ifdef HRS_BLE_APP
    {
        rawdata_vector_t rawdata;
        SPO2_CAL_SET_T cal= get_spo2_agc_status();
        for(ii=0;ii<*count;ii++)
        {
          #ifdef SPO2_DATA_CALI
            ir_data_cali = hx3690l_ir_data_cali(ir_buf[ii]);
            red_data_cali = hx3690l_red_data_cali(red_buf[ii]);
            rawdata.red_raw_data = red_data_cali;
            rawdata.ir_raw_data = ir_data_cali;
          #else
            rawdata.red_raw_data = red_buf[ii];
            rawdata.ir_raw_data = ir_buf[ii];
          #endif
            rawdata.vector_flag = SPO2_VECTOR_FLAG;
            rawdata.data_cnt = alg_results.data_cnt-*count+ii;
            rawdata.hr_result = alg_results.spo2_result;            
            rawdata.red_raw_data = red_buf[ii];
            rawdata.ir_raw_data = ir_buf[ii];
            rawdata.gsensor_x = gsen_fifo_x[ii];
            rawdata.gsensor_y = gsen_fifo_y[ii];
            rawdata.gsensor_z = gsen_fifo_z[ii];
            rawdata.red_cur = cal.R_LED;
            rawdata.ir_cur = cal.IR_LED;
            
            ble_rawdata_vector_push(rawdata);                  
        }
    }
    #endif    
}
#endif

#ifdef HRV_ALG_LIB
hrv_sensor_data_t hrv_s_dat;
void hx3690l_hrv_ppg_Int_handle(void)
{
    uint8_t        ii=0;
    //hx3690_hrv_results_t alg_hrv_results = {MSG_HRV_ALG_NOT_OPEN,MSG_HRV_NO_WEAR,0,0,0,false};
    hx3690_hrv_results_t alg_hrv_results= {MSG_HRV_ALG_NOT_OPEN,MSG_HRV_NO_WEAR,0,0,0};
    int32_t *PPG_buf = &(hrv_s_dat.ppg_data[0]);
    uint32_t *als = &(hrv_s_dat.als);
    int32_t *ir_buf = &(hrv_s_dat.ir_data[0]);
    uint8_t *count = &(hrv_s_dat.count);
    int32_t *s_buf = &(hrv_s_dat.s_buf[0]);
    int32_t phase_data[5];
    int32_t green_data = 0;
	  hx3690_hrv_wear_msg_code_t wear_mode,wear_mode_pre;
//		int8_t  TIRED_VALUE;
	  int32_t hrm_raw_data;
		
		#ifdef BP_CUSTDOWN_ALG_LIB
    hx3690_bp_results_t    bp_alg_results;
		#endif
    #ifdef HRV_TESTVEC
    hrm_raw_data = vec_data[vec_data_cnt];
    vec_data_cnt++;
    alg_hrv_results = hx3690l_hrv_alg_send_data(hrm_raw_data, 0, 0);
		//DEBUG_PRINTF(0,"hrm_raw_data: %d\r\n", hrm_raw_data);
    #endif
		
	  #ifndef HRV_TESTVEC
      #ifdef HRV_INT_MODE
      if(hx3690l_hrv_read_data_packet(phase_data) == NULL)
      {
          return;
      }
      DEBUG_PRINTF(0,"%d %d %d %d %d\r\n", phase_data[0], phase_data[1], phase_data[2], phase_data[3], phase_data[4]);
      #else
      if(hx3690l_hrv_read(&hrv_s_dat) == NULL)
      {
          return;
      }
      for(ii=0;ii<*count;ii++)
      {
          //DEBUG_PRINTF(0,"%d/%d %d %d\r\n" ,1+ii,*count,\
          PPG_buf[ii],PPG_buf[ii]);
         // SEGGER_RTT_printf(0,"%d %d %d %d %d %d\r\n", PPG_buf[ii],ir_buf[ii],gsen_fifo_x[ii],gsen_fifo_y[ii],gsen_fifo_z[ii],hrs_s_dat.agc_green);
      }
      #endif

      #ifdef HRV_INT_MODE 
        green_data =  phase_data[0] - phase_data[1];
			  #ifdef HRV_ALG_LIB
			  wear_mode = hx3690_hrv_get_wear_status();
			  wear_mode_pre = hx3690_hrv_get_wear_status_pre();
			  if(wear_mode==MSG_HRS_NO_WEAR)
				{
					alg_hrv_results = 0;
					
				}
				else
				{
          alg_hrv_results = hx3690l_hrv_alg_send_data(green_data, phase_data[0], phase_data[1]);
        }
			  #endif
		  #else
				#ifdef HRV_ALG_LIB
			  wear_mode = hx3690_hrv_get_wear_status();
			  wear_mode_pre = hx3690_hrv_get_wear_status_pre();
			  if(wear_mode==MSG_HRS_NO_WEAR)
				{
					alg_hrv_results.hrv_result = 0;
				}
				else
				{ 
					alg_hrv_results = hx3690l_hrv_alg_send_bufdata(PPG_buf, *count, 0);
        }
        #endif
			#endif
		#endif

    //display part                                           
    oled_dis.refresh_time++;
    if(oled_dis.refresh_time > 3)
    {
        oled_dis.refresh_flag = 1;
        oled_dis.refresh_time = 0;
        oled_dis.dis_mode = DIS_HRV;   
        oled_dis.dis_data = alg_hrv_results.hrv_peak; //alg_hrv_results.hrv_peak;
				#ifdef BP_CUSTDOWN_ALG_LIB                    
				bp_alg_results = hx3690_alg_get_bp_results();  
				if (bp_alg_results.sbp!= 0)
				{
					//opr_display_bp(bp_alg_results.sbp, bp_alg_results.dbp);
				}
				#endif   
    }

//    #ifdef HRS_BLE_APP
//    {
//        rawdata_vector_t rawdata;
//        
//        HRS_CAL_SET_T cal= get_hrs_agc_status();
//        for(ii=0;ii<*count;ii++)
//        {
//            rawdata.vector_flag = HRS_VECTOR_FLAG;
//            rawdata.data_cnt = alg_results.data_cnt-*count+ii;
//            rawdata.hr_result = alg_results.hr_result;           
//            rawdata.red_raw_data = PPG_buf[ii];
//            rawdata.ir_raw_data = ir_buf[ii];
//            rawdata.gsensor_x = gsen_fifo_x[ii];
//            rawdata.gsensor_y = gsen_fifo_y[ii];
//            rawdata.gsensor_z = gsen_fifo_z[ii];
//            rawdata.red_cur = cal.LED;
//            rawdata.ir_cur = alg_results.hrs_alg_status;
//            ble_rawdata_vector_push(rawdata);   
//        }
//    }
//    #endif  

}
#endif
void hx3690l_wear_ppg_Int_handle(void)
{
    int32_t ir_data[16];
    uint8_t count,ii;
    hx3690_hrs_wear_msg_code_t hx3690l_wear_status;
    count = hx3690l_check_touch_read_fifo(ir_data);
    hx3690l_wear_status = hx3690l_check_touch_send_data(ir_data,count);
    for(ii=0; ii<count; ii++)
    {
       DEBUG_PRINTF(0,"%d/%d %d\r\n" ,1+ii,count,ir_data[ii]);
    }
}


#if 0
void display_refresh(void)
{
  #ifdef BIG_SCREEN
    char dis_buf[]="0000";

    

    if(oled_dis.refresh_flag)
    {
        oled_dis.refresh_flag = 0;

        switch(oled_dis.dis_mode)
        {  
            case DIS_WAIT:
               OLED_DisplayString(0,4,16,16,"Hrs ----"); // init error
               OLED_DisplayString(0,6,16,16,"Spo2 ---"); // init error
               //opr_display_wait(0,oled_dis.dis_data);
               break;
            case DIS_BP:
               //opr_display_bp(oled_dis.dis_sbp,oled_dis.dis_dbp);
               break;
            case DIS_HR:
                sprintf(dis_buf, "%02d", oled_dis.dis_data);
                OLED_DisplayString(0,4,16,16,"Hrs");
                OLED_DisplayString(64,4,16,16,"   ");
                OLED_DisplayString(64,4,16,16,dis_buf); // init error
               //opr_display_hr(1,oled_dis.dis_data);
               break;
            case DIS_SPO2:

                sprintf(dis_buf, "%02d", oled_dis.dis_data);
                OLED_DisplayString(0,6,16,16,"Spo2"); // init error
                OLED_DisplayString(80,6,16,16,"   "); // init error
                OLED_DisplayString(80,6,16,16,dis_buf); // init error
               //opr_display_hr(0,oled_dis.dis_data);
               break;
            case DIS_HRV:
                sprintf(dis_buf, "%02d", oled_dis.dis_data);
                OLED_DisplayString(0,4,16,16,"Hrv");
                OLED_DisplayString(64,4,16,16,"   ");
                OLED_DisplayString(64,4,16,16,dis_buf); // init error
               //opr_display_hr(1,oled_dis.dis_data);
               break;
                                    
            default:
                sprintf(dis_buf, "%02d", oled_dis.dis_data);
                OLED_DisplayString(0,4,16,16,"Hrs");
                OLED_DisplayString(64,4,16,16,"   ");
                OLED_DisplayString(64,4,16,16,dis_buf); // init error
               //opr_display_hr(1,oled_dis.dis_data);
               break;
        }
    }
  #else
    if(oled_dis.refresh_flag)
    {
        oled_dis.refresh_flag = 0;

        switch(oled_dis.dis_mode)
        {  
            case DIS_WAIT:
                
               opr_display_wait(0,oled_dis.dis_data);
               break;
            case DIS_BP:
               opr_display_bp(oled_dis.dis_sbp,oled_dis.dis_dbp);
               break;
            case DIS_HR:
							if(sports_mode_menu==NORMAL_MODE)
							{
								opr_display_hr(3,oled_dis.dis_data);
							}
							else if(sports_mode_menu==RIDE_MODE)
							{
								opr_display_hr(1,oled_dis.dis_data);
							}
							else if(sports_mode_menu==WALK_MODE)
							{
								opr_display_hr(2,oled_dis.dis_data);
							}
               break;
            case DIS_HRV:
               opr_display_hr(1,oled_dis.dis_data);
               break;
            case DIS_SPO2:
               opr_display_hr(0,oled_dis.dis_data);
               break;
                                    
            default:
               opr_display_hr(1,oled_dis.dis_data);
               break;
        }
    }
  #endif
}
#endif

#ifdef HRS_BLE_APP

#define  MAX_VECTOR     50
volatile rawdata_vector_t rawdata_vector[MAX_VECTOR]; 
uint32_t vec_push_p = 0;
uint32_t vec_pull_p = 0;
uint8_t ble_send_start = 0;
void ble_rawdata_vector_push(rawdata_vector_t rawdata);

#ifdef HRS_BLE_APP_1S
extern  void ble_hrs_heart_rate_send(uint16_t hrs);
#endif

void ble_rawdata_clear(void)
{
    int nIndex = 0;
    if (ble_send_start == 1)
    {
        vec_push_p = 0;
        vec_pull_p = 0;
        ble_send_start = 0;
        for(nIndex = 0; nIndex< MAX_VECTOR-1; nIndex++)
        {
            rawdata_vector[nIndex].vector_flag = HRS_VECTOR_FLAG;
        }
    }
    DEBUG_PRINTF(0, "ble_rawdata_clear\n");
    
}

void ble_rawdata_vector_push(rawdata_vector_t rawdata)
{
    int nIndex = 0;

    if (ble_send_start == 0)
    {
        vec_push_p = 0;
    }
    else
    {
        vec_push_p++;
        if (vec_push_p > MAX_VECTOR-1)
        {
            vec_push_p = 0;
        }
    }
    nIndex = vec_push_p;
    
    rawdata_vector[nIndex].vector_flag = rawdata.vector_flag;
    rawdata_vector[nIndex].data_cnt = rawdata.data_cnt;
    rawdata_vector[nIndex].hr_result = rawdata.hr_result;     
    rawdata_vector[nIndex].red_raw_data = rawdata.red_raw_data;
    rawdata_vector[nIndex].ir_raw_data = rawdata.ir_raw_data;
    rawdata_vector[nIndex].gsensor_x = rawdata.gsensor_x;
    rawdata_vector[nIndex].gsensor_y = rawdata.gsensor_y;
    rawdata_vector[nIndex].gsensor_z = rawdata.gsensor_z;
    rawdata_vector[nIndex].red_cur = rawdata.red_cur;
    rawdata_vector[nIndex].ir_cur = rawdata.ir_cur;
}


uint32_t ble_rawdata_send_handler( )
{
    uint32_t error_code = NRF_ERROR_INVALID_STATE;
             //DEBUG_PRINTF(0,"cnt = %d, red=%d, ir=%d,X = %d, y= %d, z=%d\n", rawdata_vector[vec_pull_p].data_cnt, \
                        rawdata_vector[vec_pull_p].red_raw_data,rawdata_vector[vec_pull_p].ir_raw_data, \
                        rawdata_vector[vec_pull_p].gsensor_x, rawdata_vector[vec_pull_p].gsensor_y, \
                        rawdata_vector[vec_pull_p].gsensor_z);
    do 
    {
        if (rawdata_vector[vec_pull_p].vector_flag >0)
        {

            error_code =  ble_hrs_heart_rate_send_ext(rawdata_vector[vec_pull_p].vector_flag,\
                        rawdata_vector[vec_pull_p].data_cnt,rawdata_vector[vec_pull_p].hr_result,\
                        rawdata_vector[vec_pull_p].red_raw_data,\
                        rawdata_vector[vec_pull_p].ir_raw_data,\
                        rawdata_vector[vec_pull_p].gsensor_x,\
                        rawdata_vector[vec_pull_p].gsensor_y,\
                        rawdata_vector[vec_pull_p].gsensor_z,\
                        rawdata_vector[vec_pull_p].red_cur ,rawdata_vector[vec_pull_p].ir_cur);
            
            

            if ( NRF_SUCCESS == error_code)
            {
                ble_send_start = 1;
                rawdata_vector[vec_pull_p].vector_flag = 0;
                vec_pull_p++;
                if (vec_pull_p > MAX_VECTOR-1)
                {
                    vec_pull_p = 0;
                } 
            }            
        }
    }
    while((NRF_SUCCESS == error_code)&&(rawdata_vector[vec_pull_p].vector_flag >0));

    return error_code;

}
#endif
