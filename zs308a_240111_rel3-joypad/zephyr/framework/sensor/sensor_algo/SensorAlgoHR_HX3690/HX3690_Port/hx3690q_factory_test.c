#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

//#include "SEGGER_RTT.h"
//#include "nrf_delay.h"

//////////////////////////////


#include "hx3690l.h"
#include "hx3690q_factory_test.h"



//void hx3690l_factory_test_config(void)
//{
//    uint16_t sample_rate = 25; /*config the data rate of chip alps2_fm ,uint is Hz*/

//    uint32_t prf_clk_num = 32000/sample_rate;   /*period in clk num, num = Fclk/fs */

//    uint8_t cic_mode_en =0;
//    uint8_t cic_b2_en = 0;
//    uint8_t samp_delay_leden_num = 0; /* 00=8,01=16,10=32,11=64*/
//    uint8_t samp_copy_avg = 0;        /* 0=1, 1=2, 2=4 ,3=8, 4=16*/
//    uint8_t data_avg_num = 0;         /* 0 = 1 ; 1 = 2; 2 =4 ; 3 =8 ; 4 =16 ;*/
//    uint8_t phase3_4_internal = 0;    /* phase3 and phase4 prf internal cfg */

//    uint8_t phase1_enable = 1;     /*phase1_enable  , 1 mean enable ; 0 mean disable */
//    uint8_t phase2_enable = 1;     /*phase2_enable  , 1 mean enable ; 0 mean disable */
//    uint8_t phase3_enable = 1;     /*phase3_enable  , 1 mean enable ; 0 mean disable */
//    uint8_t phase4_enable = 1;     /*phase4_enable  , 1 mean enable ; 0 mean disable */

//    uint8_t phase1_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
//    uint8_t phase2_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
//    uint8_t phase3_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
//    uint8_t phase4_adc_osr = 3;    /* 0 = 128 ; 1 = 256 ; 2 = 512 ; 3 = 1024 ; 4 = 2048*/
//    //als
//    uint8_t phase1_inner_avg = 0;   /* phase1 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
//    uint8_t phase1_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
//    uint8_t phase1_ldr_sel = 0;     /*ball led 1 = ldr1(red); 2 = ldr2(ir); 4 = ldr3(green); 8 = ldr4 ;
//                                    * 3in1 led 1 = ldr1(red); 2 = ldr2(green); 4 = ldr3(ir); 8 = ldr4 ;
//                                      laisi led 1 = ldr1(green); 2 = ldr2(red); 4 = ldr3(ir); 8 = ldr4 ;
//                                    */
//    uint8_t phase1_pd_sel = 1;      /* 1 = pd1; 2 = pd2; */
//    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
//    uint8_t phase1_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
//    uint8_t phase1_led_en = 0;      /* phase1 led enable*/
//    //green
//    uint8_t phase2_inner_avg = 0;   /* phase2 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
//    uint8_t phase2_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
//    uint8_t phase2_ldr_sel = GREEN_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
//    uint8_t phase2_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
//    uint8_t phase2_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
//    uint8_t phase2_ldr_cur = 128;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
//    uint8_t phase2_led_en = 1;      /* phase2 led enable*/
//    //red
//    uint8_t phase3_inner_avg = 0;   /* phase3 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
//    uint8_t phase3_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
//    uint8_t phase3_ldr_sel = RED_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
//    uint8_t phase3_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
//    uint8_t phase3_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
//    uint8_t phase3_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
//    uint8_t phase3_led_en =1;      /* phase3 led enable*/
//    //ir
//    uint8_t phase4_inner_avg = 0;   /* phase4 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
//    uint8_t phase4_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
//    uint8_t phase4_ldr_sel = IR_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
//    uint8_t phase4_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
//    uint8_t phase4_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
//    uint8_t phase4_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
//    uint8_t phase4_led_en = 1;      /* phase4 led enable*/

//    uint8_t init_wait_delay = 5 ; /* 0 = 31clk ; 1 = 64clk ; 2 = 127clk ; 3 = 255clk(d) ;
//                                     4 = 511clk; 5 = 1023clk; 6 = 2047; 7 = 2048clk */

//    uint8_t afe_reset = 3;        /* 0 = 15clk ; 1 = 31clk ; 2 = 63clk ; 3 = 127clk(d) ;
//                                     4 = 255clk; 5 = 511clk; 6 = 1024; 7 = 2048clk */

//    uint8_t led_on_time = 3;      /* 0 = 32clk=8us ; 1 = 64clk=16us; 2=128clk=32us ; 3 = 256clk=64us ;
//                                     4 = 512clk=128us ; 5 = 1024clk=256us; 6= 2048clk=512us; 7 = 4096clk=1024us */

//    hx3690l_write_reg(0X6a, 0X00);	//rest int
//    hx3690l_delay(10);
//    hx3690l_write_reg(0X1a, (uint8_t)prf_clk_num);    // prf bit<7:0>
//    hx3690l_write_reg(0X1b, (uint8_t)(prf_clk_num>>8)); // prf bit<15:8>
//    hx3690l_write_reg(0X1c, (uint8_t)(prf_clk_num>>16)); // prf bit<23:16>

//    hx3690l_write_reg(0X1d, phase3_4_internal);
//    hx3690l_write_reg(0X1e, ((afe_reset<<3)| 0x00) );
//    hx3690l_write_reg(0X1f, (led_on_time<<4| phase1_led_en<<3 | phase3_led_en<<2 | phase4_led_en<<1 | phase2_led_en) );

//    hx3690l_write_reg(0X26, (init_wait_delay<<4 | 0x0f));
//    hx3690l_write_reg(0X27, (phase1_inner_avg | (phase2_inner_avg<<4)));
//    hx3690l_write_reg(0X28, (phase3_inner_avg | (phase4_inner_avg<<4)));
//    hx3690l_write_reg(0X29, cic_mode_en<<7 | cic_b2_en<<6 | samp_delay_leden_num<<4 | samp_copy_avg);

//    hx3690l_write_reg(0X2c, phase1_tia_res);
//    hx3690l_write_reg(0X2d, phase3_tia_res);
//    hx3690l_write_reg(0X2e, phase4_tia_res);
//    hx3690l_write_reg(0X2f, phase2_tia_res);

//    hx3690l_write_reg(0X30, phase1_ldr_cur);
//    hx3690l_write_reg(0X31, phase3_ldr_cur);
//    hx3690l_write_reg(0X32, phase4_ldr_cur);
//    hx3690l_write_reg(0X33, phase2_ldr_cur);

//    hx3690l_write_reg(0X34, (phase1_pd_sel<<4 |  phase1_ldr_sel));
//    hx3690l_write_reg(0X35, (phase3_pd_sel<<4 |  phase3_ldr_sel));
//    hx3690l_write_reg(0X36, (phase4_pd_sel<<4 |  phase4_ldr_sel));
//    hx3690l_write_reg(0X37, (phase2_pd_sel<<4 |  phase2_ldr_sel));

//    hx3690l_write_reg(0X38, phase1_offset_idac);
//    hx3690l_write_reg(0X39, phase3_offset_idac);
//    hx3690l_write_reg(0X3a, phase4_offset_idac);
//    hx3690l_write_reg(0X3b, phase2_offset_idac);
//    hx3690l_write_reg(0X3c, data_avg_num<<4 | data_avg_num );
//    hx3690l_write_reg(0X3d, data_avg_num<<4 | data_avg_num );

//// analog circuit cfg
//    hx3690l_write_reg(0X60, 0x0a);	//1a= adc self test
//    hx3690l_write_reg(0X66, 0x92);	//0x92= r2r idac en; 0x91= mos idac en; 0x93= two idac en;
//    hx3690l_write_reg(0X67, 0xbf);	//32k osc cfg relate
//    hx3690l_write_reg(0X69, 0xa0);	//bit<0>: rc_comb_en bits<1>=rc_rbp_en bits<7>= vcom_clamp_en bits<6:4>= LED_vdesl
//    hx3690l_write_reg(0X6a, 0X02);	//02= u_low_pow, INT cmos output

///////////FIFO and adc conversion ready config////////
//    hx3690l_write_reg(0X12, 0x32);   // fifo almostfull cfg ,max=0x40;
//    hx3690l_write_reg(0X13, 0x30);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
//                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
//                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
//									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
//    hx3690l_write_reg(0X20, 0x03);   // int width
//    hx3690l_write_reg(0X23, 0x00);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
//    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
/////////FIFO//////////

//    hx3690l_write_reg(0X18,(phase1_enable<<3)|(phase1_adc_osr)|(phase3_enable<<7)|(phase3_adc_osr<<4) );
//    hx3690l_write_reg(0X19,(phase4_enable<<3)|(phase4_adc_osr)|(phase2_enable<<7)|(phase2_adc_osr<<4) );

//    hx3690l_write_reg(0X51, 0x02);
//    hx3690l_delay(5);
//    hx3690l_write_reg(0X51, 0x00);
//	
//}

void hx3690l_factory_ft_card_test_config(void)//»Ò¿¨10mm¸ß¶È²âÊÔÅäÖÃ
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
    //als
    uint8_t phase1_inner_avg = 0;   /* phase1 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase1_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_ldr_sel = 0;     /*ball led 1 = ldr1(red); 2 = ldr2(ir); 4 = ldr3(green); 8 = ldr4 ;
                                    * 3in1 led 1 = ldr1(red); 2 = ldr2(green); 4 = ldr3(ir); 8 = ldr4 ;
                                      laisi led 1 = ldr1(green); 2 = ldr2(red); 4 = ldr3(ir); 8 = ldr4 ;
                                    */
    uint8_t phase1_pd_sel = 1;      /* 1 = pd1; 2 = pd2; */
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase1_led_en = 0;      /* phase1 led enable*/
    //green
    uint8_t phase2_inner_avg = 0;   /* phase2 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase2_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase2_ldr_sel = GREEN_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase2_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase2_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase2_ldr_cur = 128;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase2_led_en = 1;      /* phase2 led enable*/
    //red
    uint8_t phase3_inner_avg = 0;   /* phase3 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase3_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase3_ldr_sel = RED_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase3_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase3_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase3_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase3_led_en =1;      /* phase3 led enable*/
    //ir
    uint8_t phase4_inner_avg = 0;   /* phase4 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase4_tia_res = 4;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase4_ldr_sel = IR_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase4_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase4_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase4_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase4_led_en = 1;      /* phase4 led enable*/

    uint8_t init_wait_delay = 5 ; /* 0 = 31clk ; 1 = 64clk ; 2 = 127clk ; 3 = 255clk(d) ;
                                     4 = 511clk; 5 = 1023clk; 6 = 2047; 7 = 2048clk */

    uint8_t afe_reset = 3;        /* 0 = 15clk ; 1 = 31clk ; 2 = 63clk ; 3 = 127clk(d) ;
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
    hx3690l_write_reg(0X12, 0x32);   // fifo almostfull cfg ,max=0x40;
    hx3690l_write_reg(0X13, 0x30);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x00);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
///////FIFO//////////

    hx3690l_write_reg(0X18,(phase1_enable<<3)|(phase1_adc_osr)|(phase3_enable<<7)|(phase3_adc_osr<<4) );
    hx3690l_write_reg(0X19,(phase4_enable<<3)|(phase4_adc_osr)|(phase2_enable<<7)|(phase2_adc_osr<<4) );

    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X51, 0x00);
	  hx3690l_write_reg(0X6a, 0X02);	//02= u_low_pow, INT cmos output
}

void hx3690l_factory_ft_leak_light_test_config(void)//Â©¹â²âÊÔÅäÖÃ
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
    //als
    uint8_t phase1_inner_avg = 0;   /* phase1 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase1_tia_res = 6;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase1_ldr_sel = 0;     /*ball led 1 = ldr1(red); 2 = ldr2(ir); 4 = ldr3(green); 8 = ldr4 ;
                                    * 3in1 led 1 = ldr1(red); 2 = ldr2(green); 4 = ldr3(ir); 8 = ldr4 ;
                                      laisi led 1 = ldr1(green); 2 = ldr2(red); 4 = ldr3(ir); 8 = ldr4 ;
                                    */
    uint8_t phase1_pd_sel = 1;      /* 1 = pd1; 2 = pd2; */
    uint8_t phase1_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase1_ldr_cur = 0;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase1_led_en = 0;      /* phase1 led enable*/
    //green
    uint8_t phase2_inner_avg = 0;   /* phase2 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase2_tia_res = 6;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase2_ldr_sel = GREEN_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase2_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase2_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase2_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase2_led_en = 1;      /* phase2 led enable*/
    //red
    uint8_t phase3_inner_avg = 0;   /* phase3 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase3_tia_res = 6;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase3_ldr_sel = RED_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase3_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase3_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase3_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase3_led_en =1;      /* phase3 led enable*/
    //ir
    uint8_t phase4_inner_avg = 0;   /* phase4 adc avg num 0=1, 1=2, 2=4, 3=8 , 4=16*/
    uint8_t phase4_tia_res = 6;     /* 0= 10K; 1= 20k; 2= 50k; 3= 100k; 4= 150k; 5= 200k; 6= 500k; 7= 1M*/
    uint8_t phase4_ldr_sel = IR_LED_SLE;     /* 1 = ldr1; 2 = ldr2; 4 = ldr3; 8 = ldr4 ; */
    uint8_t phase4_pd_sel = 1;      /* 1 = pd1; 2 = pd2; 4 = pd3; */
    uint8_t phase4_offset_idac = 0; /* 0~127 = 0 ~ 32ua , step = 0.25ua */
    uint8_t phase4_ldr_cur = 64;     /* 0~255 = 0 ~ 200ma, step = 0.8ma */
    uint8_t phase4_led_en = 1;      /* phase4 led enable*/

    uint8_t init_wait_delay = 5 ; /* 0 = 31clk ; 1 = 64clk ; 2 = 127clk ; 3 = 255clk(d) ;
                                     4 = 511clk; 5 = 1023clk; 6 = 2047; 7 = 2048clk */

    uint8_t afe_reset = 3;        /* 0 = 15clk ; 1 = 31clk ; 2 = 63clk ; 3 = 127clk(d) ;
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
    hx3690l_write_reg(0X12, 0x32);   // fifo almostfull cfg ,max=0x40;
    hx3690l_write_reg(0X13, 0x30);   /* bits<7:4> fifo data sel, 0000 = p1-p2;0001= p1,p2;0010=p3,p4;
                                       0011=p1,p2,p3,p4;0100=p3-(p2+p4)/2;0101=p1-p2,p3-p4;0110=p2-(p1+p3)/2;
                                       bits<3:2> fifo int clear mode, 00 = selfclear;01=reserve;10=manual clear;
									   bits<1:0> fifo mode sel, 00=bypass,01=fifo,10=stream,11=reserve;*/
    hx3690l_write_reg(0X20, 0x03);   // int width
    hx3690l_write_reg(0X23, 0x00);   // phase int sel  80=p1 / 10=p2 / 40=p3 / 20 =p4
    hx3690l_write_reg(0X24, 0x00);   // fifo int output sel
///////FIFO//////////

    hx3690l_write_reg(0X18,(phase1_enable<<3)|(phase1_adc_osr)|(phase3_enable<<7)|(phase3_adc_osr<<4) );
    hx3690l_write_reg(0X19,(phase4_enable<<3)|(phase4_adc_osr)|(phase2_enable<<7)|(phase2_adc_osr<<4) );

    hx3690l_write_reg(0X51, 0x02);
    hx3690l_delay(5);
    hx3690l_write_reg(0X51, 0x00);
		hx3690l_write_reg(0X6a, 0X02);	//02= u_low_pow, INT cmos output
	
}

bool hx3690l_factory_test_read(int32_t *phase1_data,int32_t *phase2_data,int32_t *phase3_data,int32_t *phase4_data)
{
    uint8_t  databuf[12] = {0};
    int32_t P1 = 0, P2 = 0 ,P3 = 0 ,P4 = 0;
    hx3690l_brust_read_reg(0x03, databuf, 12);   
    P1 = ((databuf[0])|(databuf[1]<<8)|(databuf[2]<<16)); 
    P3 = ((databuf[3])|(databuf[4]<<8)|(databuf[5]<<16)); 
    P4 = ((databuf[6])|(databuf[7]<<8)|(databuf[8]<<16)); 
    P2 = ((databuf[9])|(databuf[10]<<8)|(databuf[11]<<16));   

		
    *phase1_data = P1 ;
    *phase2_data = P2 ;	
    *phase3_data = P3 ;
    *phase4_data = P4 ;	
		
    
    DEBUG_PRINTF(0,"p1,p2,p3,p4 %d %d %d %d\r\n" , P1, P2,P3, P4);
    return true; 	
}


bool hx3690l_chip_check_id(void)
{
    uint8_t i = 0;
	uint8_t chip_id =0;
    
    for(i=0;i<10;i++)
    {
        hx3690l_write_reg(0x02, 0x30);
        hx3690l_delay(5);
        chip_id = hx3690l_read_reg(0x00);  
        if (chip_id == 0x69)
        {
            return true;
        }        
    }

    return false;
}

bool hx3690l_factroy_test(uint32_t  test_mode)
{
    int32_t hr_gr_buffer[4] ={0,0,0,0};
//    int32_t hr_ir_buffer[4] ={0,0,0,0};
    int32_t spo2_red_buffer[4] ={0,0,0,0};
    int32_t spo2_ir_buffer[4] ={0,0,0,0};
	int32_t  phase1_data= 0;
	int32_t  phase2_data= 0;
	int32_t  phase3_data= 0;
	int32_t  phase4_data= 0;

	int32_t  green_led_data= 0;
	int32_t  red_led_data= 0;
	int32_t  ir_led_data= 0;	
	
	uint32_t hr_gr_final = 0;
//    uint32_t hr_ir_final = 0;
	uint32_t spo2_red_final = 0;
    uint32_t spo2_ir_final = 0;
	uint32_t testdata_temp = 0;
	bool nrtn = false;
	
    uint8_t ii = 0;
    uint8_t jj = 0;

	if (hx3690l_chip_check_id() == false)
	{
		return false;
	}
	
		switch(test_mode)
    {
        case LEAK_LIGHT_TEST:
        hx3690l_factory_ft_leak_light_test_config();
            break;            
        case GRAY_CARD_TEST:
        hx3690l_factory_ft_card_test_config();
            break;
        default:
			break;
    }


    

	
	hx3690l_delay(100);
	
    for(ii=0;ii<4;ii++)   //40ms read data
    {
		hx3690l_factory_test_read(&phase1_data,&phase2_data,&phase3_data,&phase4_data);	

        if(phase2_data > phase1_data){
            green_led_data = phase2_data - phase1_data;		
        }else{
            green_led_data = 0;
        }

        if(phase3_data > phase1_data){
            red_led_data = phase3_data - phase1_data;		
        }else{
            red_led_data = 0;
        }

        if(phase4_data > phase1_data){
            ir_led_data = phase4_data - phase1_data;		
        }else{
            ir_led_data = 0;
        }

        hr_gr_buffer[ii] = green_led_data;
        spo2_ir_buffer[ii] = ir_led_data;
		spo2_red_buffer[ii] = red_led_data;
        //SEGGER_RTT_printf(0,"%d %d\r\n", gr_buffer[ii], ir_buffer[ii]);

        hx3690l_delay(40);
    }

	
	for(ii=0;ii<3;ii++)
	{
		for(jj=0;jj<3-ii;jj++)
		{
			if(hr_gr_buffer[jj]>=hr_gr_buffer[jj+1])
			{
				testdata_temp = hr_gr_buffer[jj];
				hr_gr_buffer[jj] = hr_gr_buffer[jj+1];
				hr_gr_buffer[jj+1] = testdata_temp;
			}

			if(spo2_red_buffer[jj]>=spo2_red_buffer[jj+1])
			{
				testdata_temp = spo2_red_buffer[jj];
				spo2_red_buffer[jj] = spo2_red_buffer[jj+1];
				spo2_red_buffer[jj+1] = testdata_temp;
			}
			if(spo2_ir_buffer[jj]>=spo2_ir_buffer[jj+1])
			{
				testdata_temp = spo2_ir_buffer[jj];
				spo2_ir_buffer[jj] = spo2_ir_buffer[jj+1];
				spo2_ir_buffer[jj+1] = testdata_temp;
			}
		}
	}
	hr_gr_final = (hr_gr_buffer[1]+hr_gr_buffer[2])/2;
	spo2_red_final = (spo2_red_buffer[1]+spo2_red_buffer[2])/2;
	spo2_ir_final = (spo2_ir_buffer[1]+spo2_ir_buffer[2])/2;

	if(test_mode == GRAY_CARD_TEST)
	{
			if(hr_gr_buffer[0]==hr_gr_buffer[1] && hr_gr_buffer[0]==hr_gr_buffer[2] && hr_gr_buffer[0]==hr_gr_buffer[3])
			{
				hr_gr_final = 0;
			}
			if(spo2_red_buffer[0]==spo2_red_buffer[1] && spo2_red_buffer[0]==spo2_red_buffer[2] && spo2_red_buffer[0]==spo2_red_buffer[3])
			{
				spo2_red_final = 0;
			}
			if(spo2_ir_buffer[0]==spo2_ir_buffer[1] && spo2_ir_buffer[0]==spo2_ir_buffer[2] && spo2_ir_buffer[0]==spo2_ir_buffer[3])
			{
				spo2_ir_final = 0;
			}
	}
	
	DEBUG_PRINTF(0, "g,r,ir:%d %d %d\r\n",hr_gr_final , spo2_red_final ,spo2_ir_final);
	  
    switch(test_mode)
    {
        case LEAK_LIGHT_TEST:
			if((hr_gr_final <FT_OPEN_G_LED_LTH)&&(spo2_red_final< FT_OPEN_R_LED_LTH)&&(spo2_ir_final <FT_OPEN_IR_LED_LTH )){
				nrtn =  true;
			//
			}else{
				nrtn =  false;
			//
			}
            DEBUG_PRINTF(0, "ft leak light test  %d \r\n",nrtn);
            break;            
        case GRAY_CARD_TEST:
			if((hr_gr_final > FT_CARD_G_LED_HTH)&(spo2_red_final > FT_CARD_R_LED_HTH)&(spo2_ir_final > FT_CARD_IR_LED_HTH )){
				nrtn =  true;
			//
			}else{
				nrtn =  false;
			//
			}
            DEBUG_PRINTF(0, "ft gray card test  %d \r\n",nrtn);
            break;
        default:
			break;
    }

	
	return nrtn;

}

