
#include "SL_Watch_Pedo_Kcal_Wrist_Sleep_Sway_Algorithm.h"
#include "sensor_algo.h"


#if SL_Sensor_Algo_Release_Enable==0x00
#include "usart.h"
extern signed short          SL_DEBUG_DATA[10][128];
extern unsigned char         SL_DEBUG_DATA_LEN;
extern unsigned char         sl_sleep_sys_cnt;
#endif

#if SL_Sensor_Algo_Release_Enable==0x00
extern signed short          SL_ACCEL_DATA_Buf[3][16];
extern unsigned char         SL_FIFO_ACCEL_PEDO_NUM;
signed short                 hp_buf[16];
#endif


#define SL_SC7A20_SPI_IIC_MODE  1
/***SL_SC7A20_SPI_IIC_MODE==0  :SPI*******/
/***SL_SC7A20_SPI_IIC_MODE==1  :IIC*******/

static signed char sl_init_status=0x00;
/**Please modify the following parameters according to the actual situation**/
signed char SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_SWAY_INIT(void)
{
#if SL_Sensor_Algo_Release_Enable==0x00
	unsigned char sl_version_value=0x00;
#endif
//	bool  fifo_status       =0;
	unsigned char  sl_person_para[4] ={178,60,26,1};
	unsigned char  Turn_Wrist_Para[3]={1,3,0};//refer pdf
	
#if SL_Sensor_Algo_Release_Enable==0x00
	//sl_version_value==0x11  SC7A20
	//sl_version_value==0x26  SC7A20E
	SL_SC7A20_I2c_Spi_Read(1, SL_SC7A20_VERSION_ADDR,1, &sl_version_value);
		USART_printf( USART1, "Version ID=0x%x!\r\n",sl_version_value);
	if(sl_version_value==SL_SC7A20_VERSION_VALUE)
		USART_printf( USART1, "SC7A20!\r\n");
	else if(sl_version_value==SL_SC7A20E_VERSION_VALUE)
		USART_printf( USART1, "SC7A20E!\r\n");
#endif
	
/*****if use spi mode please config first******/    
#if SL_SC7A20_SPI_IIC_MODE == 0//spi
    sl_init_status=SL_SC7A20_Driver_Init(1,0x04);//iic mode pull up config 
	//init sensor
	sl_init_status=SL_SC7A20_Driver_Init(0,0x04);//spi or iic select
#else//i2c
 	sl_init_status=SL_SC7A20_Driver_Init(1,0x00);//spi or iic select   
	//0x08-->close sdo pull up
#endif
    /**********sc7a20 fifo test***************/
	//fifo_status=SL_SC7A20_FIFO_TEST();
	
    /**********set pedo sensitivity***********/
    SL_PEDO_TH_SET(32,10,1,30,50);//36 12 0 30 50for motor
	/*which axis used to pedo depend on algo**/
	SL_PEDO_SET_AXIS(3);
    /**********set int para*******************/
    SL_PEDO_INT_SET(2,1,1);//0 or 1 sleep is different
	/**********set motion para****************/
	SL_Pedo_Person_Inf_Init(&sl_person_para[0]);//personal para init
    /**********set turn wrist para************/
    SL_Turn_Wrist_Init(&Turn_Wrist_Para[0]);
    /**********set sleep sensitivity**********/
    SL_Sleep_Para(120,10,1);
	
	return sl_init_status;
}

static unsigned int   SL_STEP                  = 0;
static unsigned int   SL_STEP_TEMP             = 0;
static unsigned char  SL_STEP_MOTION           = 0;
static unsigned int	  SL_DISTANCE              = 0;
static unsigned int	  SL_KCAL                  = 0;
static unsigned char  SL_CADENCE_STEP          = 0;//step per minutes
static unsigned short SL_CADENCE_AMP           = 0;//Equal scale scaling 
static unsigned char  SL_CADENCE_DEGREE        = 0;//degree
static signed char    SL_WRIST                 = 0;

static unsigned char  SL_SLEEP_ACTIVE          = 0;
static unsigned char  SL_SLEEP_STATUS          = 0;
static unsigned char  SL_SLEEP_ADOM            = 0;
static unsigned char  SL_SLEEP_CNT             = 0;

static bool           SL_CLOCK_STATUS          = 0;
static bool           SL_SWAY_STATUS           = 0;
static bool           SL_SWAY_ENABLE           = 0;

static unsigned char  SL_INT_TIME_CNT          = 0;
#define SL_INT_TIME_TH   12  //per lsb is 0.5s
unsigned char sl_sleep_sys_hour;

extern sensor_res_t sensor_algo_res;
extern sensor_handler_t user_handler;

unsigned char SL_MCU_SLEEP_ALGO_FUNCTION(void);
void SL_Result_Analysis(void);

unsigned int  SL_SC7A20_PEDO_KCAL_WRIST_SLEEP_SWAY_ALGO(void)
{
#if SL_Sensor_Algo_Release_Enable==0x00
	unsigned char         sl_i;
#endif
	signed short          SC7A20_XYZ_Buf[3][14];
	unsigned char         SC7A20_FIFO_NUM;
	bool                  SC7A20_INT_STATUS;
	
	if((sl_init_status!=SL_SC7A20_VERSION_VALUE)&&(sl_init_status!=SL_SC7A20E_VERSION_VALUE))	return 0;//initial fail

    /*******read int status******/
	SC7A20_INT_STATUS=SL_INT_STATUS_READ();
	if((SC7A20_INT_STATUS==1)||(SL_STEP_TEMP!= SL_STEP))
	{
		SL_INT_TIME_CNT =0;
		SL_STEP_TEMP    =SL_STEP;
	}
	else
	{
		if(SL_INT_TIME_CNT<200)
		{
			SL_INT_TIME_CNT++;
		}
	}
	
	if(SL_INT_TIME_CNT<SL_INT_TIME_TH)//6s
	{
	    /*******get sc7a20 FIFO data******/
		SC7A20_FIFO_NUM= SL_SC7A20_Read_FIFO();
        /*******get pedo value************/
	    SL_STEP= SL_Watch_Kcal_Pedo_Algo(0);//不打开马达或音乐时调用该函数
	    //SL_STEP= SL_Watch_Kcal_Pedo_Algo(1);//打开马达或音乐时调用该函数

	    /*********get sleep active degree value*********/
		SL_SLEEP_ACTIVE      = SL_Sleep_Active_Degree(0);
		
	    /*******get accel orginal data and length*******/
	    SC7A20_FIFO_NUM      = SL_SC7A20_GET_FIFO_Buf(&SC7A20_XYZ_Buf[0][0],&SC7A20_XYZ_Buf[1][0],&SC7A20_XYZ_Buf[2][0],0);
    
		/* SL_Pedo_GetMotion_Status */
		SL_STEP_MOTION       = SL_Pedo_GetMotion_Status();
		/* SL_Pedo_Step_Get_Distance */
		SL_DISTANCE          = SL_Pedo_Step_Get_Distance();
		/* SL_Pedo_Step_Get_KCal */
		SL_KCAL              = SL_Pedo_Step_Get_KCal();
		/*****average step per minutes****/    
		SL_CADENCE_STEP      = SL_Pedo_Step_Get_Step_Per_Min();
		/*****average amp per step****/    
		SL_CADENCE_AMP       = SL_Pedo_Step_Get_Avg_Amp();
		/*****motion degree****/    
		SL_CADENCE_DEGREE    = SL_Pedo_Step_Get_Motion_Degree();
    
//	    if(SL_STEP>200)
//	    {
	        /**reset pedo value**/
//	        SL_Pedo_Kcal_ResetStepCount();
//	    }
    
	    /*******get wrist value******/
	    SL_WRIST= SL_Watch_Wrist_Algo();
    
	    /*******get overturn value******/
	    SL_CLOCK_STATUS=SL_Get_Clock_Status(1);//open overturn monitor
	    if(SL_CLOCK_STATUS==1)//overturn success
	    {
	        SL_Get_Clock_Status(0);//close overturn monitor
	    }

	    /*******get sway value******/
	    if(SL_SWAY_ENABLE==1)
	    {
	        /**this function will disable pedo function**/        
	        SL_SWAY_STATUS=SL_Get_Phone_Answer_Status(4,5);//get sway value
	    }
	}
	else
	{
	    //sc7a20 's data for heart rate algo
		//SC7A20_FIFO_NUM= SL_SC7A20_Read_FIFO();	
	    //SC7A20_FIFO_NUM= SL_SC7A20_GET_FIFO_Buf(&SC7A20_XYZ_Buf[0][0],&SC7A20_XYZ_Buf[1][0],&SC7A20_XYZ_Buf[2][0],0);	
		
		SL_Pedo_WorkMode_Reset();
		//SL_Turn_Wrist_WorkMode_Reset();please not use this function
		SC7A20_FIFO_NUM=0;
	}

	/*******get sleep status value******/
	SL_SLEEP_CNT++;
#if SL_Sensor_Algo_Release_Enable==0x00
	if(SL_SLEEP_CNT>39)//1min
#else
	if(SL_SLEEP_CNT>119)//1min	
#endif
	{
#if SL_Sensor_Algo_Release_Enable==0
		sl_sleep_sys_cnt++;
		if(sl_sleep_sys_cnt>59)//min
		{
			sl_sleep_sys_cnt=0;
			sl_sleep_sys_hour++;//hour
			if(sl_sleep_sys_hour==24)
				sl_sleep_sys_hour=0;
		}
#endif
		SL_SLEEP_CNT=0;
		SL_MCU_SLEEP_ALGO_FUNCTION();
	}

#if SL_Sensor_Algo_Release_Enable==0x00
	SL_DEBUG_DATA_LEN=SC7A20_FIFO_NUM;
	for(sl_i=0;sl_i<SL_DEBUG_DATA_LEN;sl_i++)
	{       
		SL_DEBUG_DATA[0][sl_i]=SC7A20_XYZ_Buf[0][sl_i];
		SL_DEBUG_DATA[1][sl_i]=SC7A20_XYZ_Buf[1][sl_i];
		SL_DEBUG_DATA[2][sl_i]=SC7A20_XYZ_Buf[2][sl_i];
		SL_DEBUG_DATA[3][sl_i]=SL_STEP;
		SL_DEBUG_DATA[4][sl_i]=SL_WRIST;
//        SL_DEBUG_DATA[4][sl_i]=SL_CLOCK_STATUS;
//		SL_DEBUG_DATA[5][sl_i]=SL_SLEEP_STATUS;
        SL_DEBUG_DATA[5][sl_i]=SL_SWAY_STATUS; 
		SL_DEBUG_DATA[6][sl_i]=SL_STEP_MOTION;  
		SL_DEBUG_DATA[7][sl_i]=SL_DISTANCE;
//		SL_DEBUG_DATA[7][sl_i]=hp_buf[sl_i];
		SL_DEBUG_DATA[8][sl_i]=SL_KCAL;
		SL_DEBUG_DATA[9][sl_i]=SL_CADENCE_STEP;
//        SL_DEBUG_DATA[8][sl_i]=SL_CADENCE_AMP;
//        SL_DEBUG_DATA[9][sl_i]=SL_CADENCE_DEGREE;	
	}
#endif

	SL_Result_Analysis();
    return SL_STEP;
}

#if SL_Sensor_Algo_Release_Enable==0x00
unsigned short sl_sleep_counter=0;
#endif
#define SL_SLEEP_DEEP_TH   6
#define SL_SLEEP_LIGHT_TH  3
/***Call this function regularly for 1 minute***/
unsigned char SL_MCU_SLEEP_ALGO_FUNCTION(void)
{

	/*******get sleep status value******/
	SL_SLEEP_STATUS= SL_Sleep_GetStatus(sl_sleep_sys_hour);
	SL_SLEEP_ACTIVE= SL_Sleep_Get_Active_Degree();
	SL_SLEEP_ADOM  = SL_Adom_GetStatus();
	
#if SL_Sensor_Algo_Release_Enable==0x00
	sl_sleep_counter++;
	USART_printf( USART1, "step=%d! sys_time=%d!\r\n",SL_STEP,sl_sleep_sys_hour);
	USART_printf( USART1, "T=%d,sleep_status:%d,sleep_adom:%d!\r\n",sl_sleep_counter,SL_SLEEP_STATUS,SL_SLEEP_ADOM);
	USART_printf( USART1, "SL_SLEEP_ACTIVE:%d!\r\n",SL_SLEEP_ACTIVE);
#endif
	
	if(SL_SLEEP_STATUS<SL_SLEEP_LIGHT_TH)
	{
		return 0;//0 1 2 3
	}
	else if(SL_SLEEP_STATUS<SL_SLEEP_DEEP_TH)
	{
		return 1;//4 5 6
	}
	else
	{
		return 2;//7
	}
}

void SL_Result_Analysis(void)
{
	sensor_evt_t sensor_evt = {0};
	
	sensor_algo_res.handup = SL_WRIST;
	if(SL_WRIST == 1)
	{
		sensor_evt.id = ALGO_HANDUP;
		user_handler(&sensor_evt);	
	}
	else if(SL_WRIST == 2)
	{
		sensor_evt.id = ALGO_HANDUP;
		user_handler(&sensor_evt);
	}
}

