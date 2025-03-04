/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file register address define for Actions SoC
 */

#ifndef	_ACTIONS_SOC_REGS_H_
#define	_ACTIONS_SOC_REGS_H_
#include "brom_interface.h"

#define RMU_REG_BASE				0x40000000
#define CMUD_REG_BASE				0x40001000
#define CMUA_REG_BASE				0x40000100
#define PMU_REG_BASE				0x40004000
#define GPIO_REG_BASE				0x40068000
#define DMA_REG_BASE				0x4001c000
#define DMA_LINE0_REG_BASE			0x4001cc00
#define DMA_LINE1_REG_BASE			0x4001cc20
#define UART0_REG_BASE				0x40038000
#define UART1_REG_BASE				0x4003C000
#define UART2_REG_BASE				0x40040000
#define UART3_REG_BASE				0x40044000
#define MEM_REG_BASE                0x40010000
#define SE_REG_BASE                 0x40020000
#define I2C0_REG_BASE				0x40048000
#define I2C1_REG_BASE				0x4004c000
#define SPI0_REG_BASE				0x40028000
#define SPI1_REG_BASE				0x4002c000
#define SPI2_REG_BASE				0x40030000
#define SPI3_REG_BASE				0x40034000
#define I2CMT0_REG_BASE				0x40088000
#define I2CMT1_REG_BASE				0x4008c000
#define WIC_BASE                    0x40090000
#define DE_REG_BASE                 0x4006C000
#define LCDC_REG_BASE               0x40064000
#define SPIMT0_REG_BASE				0x40080000
#define SPIMT1_REG_BASE				0x40084000
#define BTC_REG_BASE                0x01100000
#define SD0_REG_BASE				0x40054000
#define SD1_REG_BASE				0x40058000
#define PWM_REG_BASE                0x40060000
#define PWM_CLK0_BASE               0x40001054





#define HOSC_CTL					(CMUA_REG_BASE + 0x00)
#define HOSCLDO_CTL					(CMUA_REG_BASE + 0x04)
#define CK64M_CTL					(CMUA_REG_BASE + 0x08)
#define LOSC_CTL					(CMUA_REG_BASE + 0x10)
#define RC4M_CTL					(CMUA_REG_BASE + 0x14)
#define AVDDLDO_CTL					(CMUA_REG_BASE + 0x1c)
#define COREPLL_CTL					(CMUA_REG_BASE + 0x20)

#define RC32K_CTL					(CMUA_REG_BASE + 0x64)
#define RC32K_CAL					(CMUA_REG_BASE + 0x68)
#define RC32K_COUNT					(CMUA_REG_BASE + 0x6C)




/*fix build err, dsp pll not exist*/
#define DSPPLL_CTL					(CMUA_REG_BASE + 0x24)
#define DISPLAYPLL_CTL				(CMUA_REG_BASE + 0x30)


#define AUDIOLDO_CTL				(CMUA_REG_BASE + 0x1C)
#define AUDIO_PLL0_CTL				(CMUA_REG_BASE + 0x28)
#define AUDIO_PLL1_CTL				(CMUA_REG_BASE + 0x2C)

#define HOSCOK_CTL					(CMUA_REG_BASE + 0x70)

#define CMU_SYSCLK					(CMUD_REG_BASE + 0x00)
#define CMU_DEVCLKEN0				(CMUD_REG_BASE + 0x04)
#define CMU_DEVCLKEN1				(CMUD_REG_BASE + 0x08)
#define CMU_SD0CLK                  (CMUD_REG_BASE + 0x10)
#define CMU_SD1CLK                  (CMUD_REG_BASE + 0x14)
#define CMU_DECLK					(CMUD_REG_BASE + 0x30)
#define CMU_LCDCLK					(CMUD_REG_BASE + 0x34)
#define CMU_SECCLK                  (CMUD_REG_BASE + 0x3C)
#define CMU_LRADCCLK				(CMUD_REG_BASE + 0x78)
#define CMU_ANCCLK                  (CMUD_REG_BASE + 0x8C)
#define CMU_DSPCLK                  (CMUD_REG_BASE + 0x90)
#define CMU_ANCDSPCLK               (CMUD_REG_BASE + 0x94)
#define CMU_DACCLK                  (CMUD_REG_BASE + 0x98)
#define CMU_ADCCLK                  (CMUD_REG_BASE + 0x9C)
#define CMU_I2STXCLK                (CMUD_REG_BASE + 0xA0)
#define CMU_I2SRXCLK                (CMUD_REG_BASE + 0xA4)
#define CMU_SPDIFTXCLK              (CMUD_REG_BASE + 0xA8)
#define CMU_SPDIFRXCLK              (CMUD_REG_BASE + 0xAC)

#define CMU_SPI0CLK                  (CMUD_REG_BASE + 0x20)
#define CMU_SPI1CLK                  (CMUD_REG_BASE + 0x24)
#define CMU_SPI2CLK                  (CMUD_REG_BASE + 0x28)
#define CMU_SPI3CLK                  (CMUD_REG_BASE + 0x2C)


#define CMU_MEMCLKEN0				(CMUD_REG_BASE+0x00B0)
#define CMU_MEMCLKEN1				(CMUD_REG_BASE+0x00B4)
#define CMU_MEMCLKEN2				(CMUD_REG_BASE+0x00B8)

#define CMU_MEMCLKSRC0				(CMUD_REG_BASE+0x00C0)
#define CMU_MEMCLKSRC1				(CMUD_REG_BASE+0x00C4)
#define CMU_MEMCLKSRC2				(CMUD_REG_BASE+0x00C8)

#define CMU_S1CLKCTL                (CMUD_REG_BASE+0x00D0)
#define CMU_S1BTCLKCTL              (CMUD_REG_BASE+0x00D4)
//#define CMU_S2HCLKCTL               (CMUD_REG_BASE+0x00D8)
#define CMU_S2SCLKCTL               (CMUD_REG_BASE+0x00DC)
#define CMU_S3CLKCTL                (CMUD_REG_BASE+0x00E0)
#define CMU_GPIOCLKCTL              (CMUD_REG_BASE+0x00E8)
#define CMU_PMUWKUPCLKCTL			(CMUD_REG_BASE+0x00EC)

#define CMU_TIMER0CLK               (CMUD_REG_BASE+0x0040)
#define CMU_TIMER1CLK               (CMUD_REG_BASE+0x0044)
#define CMU_TIMER2CLK               (CMUD_REG_BASE+0x0048)
#define CMU_TIMER3CLK               (CMUD_REG_BASE+0x004c)
#define CMU_TIMER4CLK               (CMUD_REG_BASE+0x0050)		/* For tws used */

#define CMU_SPIMT0CLK               (CMUD_REG_BASE + 0x100)
#define CMU_SPIMT1CLK               (CMUD_REG_BASE + 0x104)
#define CMU_I2CMT0CLK               (CMUD_REG_BASE + 0x110)
#define CMU_I2CMT1CLK               (CMUD_REG_BASE + 0x114)

#define RMU_MRCR0					(RMU_REG_BASE + 0x00)
#define RMU_MRCR1					(RMU_REG_BASE + 0x04)
#define DSP_VCT_ADDR                (RMU_REG_BASE+0x00000080)
#define DSP_STATUS_EXT_CTL          (RMU_REG_BASE+0x00000084)
#define ANC_VCT_ADDR               (RMU_REG_BASE+0x00000090)
#define ANC_STATUS_EXT_CTL         (RMU_REG_BASE+0x00000094)
#define	CHIPVERSION                 (RMU_REG_BASE+0x000000A0)

#define INTC_BASE					0x40003000
#define INT_TO_DSP					(INTC_BASE+0x00000000)
#define INFO_TO_DSP					(INTC_BASE+0x00000004)
#define INT_TO_ANC_DSP				(INTC_BASE+0x00000008)
#define INFO_TO_ANC_DSP				(INTC_BASE+0x0000000C)
#define INT_TO_BT_CPU				(INTC_BASE+0x00000010)
#define PENDING_FROM_DSP			(INTC_BASE+0x00000014)
#define PENDING_FROM_ANC_DSP		(INTC_BASE+0x00000018)
#define PENDING_FROM_BT_CPU         (INTC_BASE+0x0000001C)

#define PENDING_FROM_SENSOR_CPU     INTC_BASE
#define INT_TO_SENSOR_CPU			INTC_BASE
#define INT_TO_SENSOR_CPU			INTC_BASE

#define MEMORY_CTL					(MEM_REG_BASE)
#define DSP_PAGE_ADDR0				(MEM_REG_BASE+0x00000080)
#define DSP_PAGE_ADDR1				(MEM_REG_BASE+0x00000084)
#define DSP_PAGE_ADDR2              (MEM_REG_BASE+0x00000088)
#define DSP_PAGE_ADDR3              (MEM_REG_BASE+0x0000008c)

#define WIO0_CTL					(GPIO_REG_BASE + 0x300)
#define WIO1_CTL					(GPIO_REG_BASE + 0x304)
#define WIO2_CTL					(GPIO_REG_BASE + 0x308)
#define WIO3_CTL					(GPIO_REG_BASE + 0x30c)

// brom api address
//fpga
#define SPINOR_API_ADDR				(0x00005d10) //(0x00005814)


//#define SPINOR_API_ADDR				(p_brom_api->p_spinor_api)


// Interaction RAM
#define INTER_RAM_ADDR				(0x01068000)
#define INTER_RAM_SIZE				(0x00008000)

//--------------SPICACHE_Control_Register-------------------------------------------//
//--------------Register Address---------------------------------------//
#define     SPICACHE_Control_Register_BASE                                    0x40014000
#define     SPICACHE_CTL                                                      (SPICACHE_Control_Register_BASE+0x0000)
#define     SPICACHE_INVALIDATE                                               (SPICACHE_Control_Register_BASE+0x0004)
#define     SPICACHE_UNCACHE_ADDR_START                                       (SPICACHE_Control_Register_BASE+0x0008)
#define     SPICACHE_UNCACHE_ADDR_END                                         (SPICACHE_Control_Register_BASE+0x000C)
#define     SPICACHE_TOTAL_MISS_COUNT                                         (SPICACHE_Control_Register_BASE+0x0010)
#define     SPICACHE_TOTAL_HIT_COUNT                                          (SPICACHE_Control_Register_BASE+0x0014)
#define     SPICACHE_PROFILE_INDEX_START                                      (SPICACHE_Control_Register_BASE+0x0018)
#define     SPICACHE_PROFILE_INDEX_END                                        (SPICACHE_Control_Register_BASE+0x001C)
#define     SPICACHE_RANGE_INDEX_MISS_COUNT                                   (SPICACHE_Control_Register_BASE+0x0020)
#define     SPICACHE_RANGE_INDEX_HIT_COUNT                                    (SPICACHE_Control_Register_BASE+0x0024)
#define     SPICACHE_PROFILE_ADDR_START                                       (SPICACHE_Control_Register_BASE+0x0028)
#define     SPICACHE_PROFILE_ADDR_END                                         (SPICACHE_Control_Register_BASE+0x002C)
#define     SPICACHE_RANGE_ADDR_MISS_COUNT                                    (SPICACHE_Control_Register_BASE+0x0030)
#define     SPICACHE_RANGE_ADDR_HIT_COUNT                                     (SPICACHE_Control_Register_BASE+0x0034)

#define     SPI1_CACHE_REGISTER_BASE                                          0x40018000
#define     SPI1_CACHE_CTL                                                    (SPI1_CACHE_REGISTER_BASE+0x0000)
#define     SPI1_CACHE_OPERATE                                                (SPI1_CACHE_REGISTER_BASE+0x0004)
#define     SPI1_CACHE_CPU_UNCACHE_ADDR_START                                 (SPI1_CACHE_REGISTER_BASE+0x0008)
#define     SPI1_CACHE_CPU_UNCACHE_ADDR_END                                   (SPI1_CACHE_REGISTER_BASE+0x000C)
#define     SPI1_CACHE_M4F_MISS_COUNT                                         (SPI1_CACHE_REGISTER_BASE+0x0010)
#define     SPI1_CACHE_M4F_HIT_COUNT                                          (SPI1_CACHE_REGISTER_BASE+0x0014)
#define     SPI1_CACHE_M4F_WRITEBACK_COUNT                                    (SPI1_CACHE_REGISTER_BASE+0x0018)
#define     SPI1_CACHE_DMA_MISS_COUNT                                         (SPI1_CACHE_REGISTER_BASE+0x0028)
#define     SPI1_CACHE_DMA_HIT_COUNT                                          (SPI1_CACHE_REGISTER_BASE+0x002c)
#define     SPI1_CACHE_DMA_WRITEBACK_COUNT                                    (SPI1_CACHE_REGISTER_BASE+0x0030)
#define     SPI1_CACHE_DMA_UNCACHE_ADDR_START                                 (SPI1_CACHE_REGISTER_BASE+0x0034)
#define     SPI1_CACHE_DMA_UNCACHE_ADDR_END                                   (SPI1_CACHE_REGISTER_BASE+0x0038)
#define     SPI1_CACHE_PROFILE_ADDR_START                                     (SPI1_CACHE_REGISTER_BASE+0x003c)
#define     SPI1_CACHE_PROFILE_ADDR_END                                       (SPI1_CACHE_REGISTER_BASE+0x0040)
#define     SPI1_CACHE_DSP_MISS_COUNT                                         (SPI1_CACHE_REGISTER_BASE+0x0044)
#define     SPI1_CACHE_DSP_HIT_COUNT                                          (SPI1_CACHE_REGISTER_BASE+0x0048)
#define     SPI1_CACHE_DSP_WRITEBACK_COUNT                                    (SPI1_CACHE_REGISTER_BASE+0x004c)
#define     SPI1_CACHE_DE_READ_COUNT                                          (SPI1_CACHE_REGISTER_BASE+0x0050)
#define     SPI1_CACHE_DE_WRITE_COUNT                                         (SPI1_CACHE_REGISTER_BASE+0x0054)


//--------------PMUVDD-------------------------------------------//
//--------------Register Address---------------------------------------//
#define     PMUVDD_BASE                                                       0x40004000
#define     VOUT_CTL0                                                         (PMUVDD_BASE+0x00)
#define     VOUT_CTL1_S1                                                      (PMUVDD_BASE+0x04)
#define     VOUT_CTL1_S2                                                      (PMUVDD_BASE+0x08)
#define     VOUT_CTL1_S3                                                      (PMUVDD_BASE+0X0C)
#define     PMU_DET                                                           (PMUVDD_BASE+0x10)
#define     PMU_DEBUG                                                         (PMUVDD_BASE+0X14)
#define     DCDC_VC18_CTL                                                     (PMUVDD_BASE+0X20)
#define     DCDC_VD12_CTL                                                     (PMUVDD_BASE+0X24)
#define     DCDC_VDD_CTL                                                      (PMUVDD_BASE+0X28)
#define     PWRGATE_DIG                                                       (PMUVDD_BASE+0X30)
#define     PWRGATE_DIG_ACK                                                   (PMUVDD_BASE+0X34)
#define     PWRGATE_RAM                                                       (PMUVDD_BASE+0X38)
#define     PWRGATE_RAM_ACK                                                   (PMUVDD_BASE+0X3C)
#define     PMU_INTMASK                                                       (PMUVDD_BASE+0X40)


//--------------PMUSVCC-------------------------------------------//
//--------------Register Address---------------------------------------//
#define     PMUSVCC_BASE                                                      0x40004000
#define     CHG_CTL_SVCC                                                      (PMUSVCC_BASE+0X100)
#define     BDG_CTL_SVCC                                                      (PMUSVCC_BASE+0x104)
#define     SYSTEM_SET_SVCC                                                   (PMUSVCC_BASE+0x108)
#define     POWER_CTL_SVCC                                                    (PMUSVCC_BASE+0x10C)
#define     WKEN_CTL_SVCC                                                     (PMUSVCC_BASE+0x110)
#define     WAKE_PD_SVCC                                                      (PMUSVCC_BASE+0x114)
#define     PWM_CTL_SVCC                                                      (PMUSVCC_BASE+0x118)
#define     PWM_DUTY_SVCC                                                     (PMUSVCC_BASE+0x11c)


#define     PMUADC_BASE                                                      0x40004000
#define 	PMUADC_CTL 	  													(PMUADC_BASE+0x200)


//--------------EFUSE-------------------------------------------//
#define     EFUSE_BASE                                                        0x40008000
#define     EFUSE_CTL0                                                        (EFUSE_BASE+0x00)
#define     EFUSE_CTL1                                                        (EFUSE_BASE+0x04)
#define     EFUSE_CTL2                                                        (EFUSE_BASE+0x08)
#define     EFUSE_DATA0                                                       (EFUSE_BASE+0x0c)
#define     EFUSE_DATA1                                                       (EFUSE_BASE+0x10)
#define     EFUSE_DATA2                                                       (EFUSE_BASE+0x14)
#define     EFUSE_DATA3                                                       (EFUSE_BASE+0x18)
#define     EFUSE_DATA4                                                       (EFUSE_BASE+0x1c)
#define     EFUSE_DATA5                                                       (EFUSE_BASE+0x20)
#define     EFUSE_DATA6                                                       (EFUSE_BASE+0x24)
#define     EFUSE_DATA7                                                       (EFUSE_BASE+0x28)
#define     EFUSE_DATA8                                                       (EFUSE_BASE+0x2c)
#define     EFUSE_DATA9                                                       (EFUSE_BASE+0x30)
#define     EFUSE_DATA10                                                      (EFUSE_BASE+0x34)
#define     EFUSE_DATA11                                                      (EFUSE_BASE+0x38)
#define     EFUSE_PGDATA0                                                     (EFUSE_BASE+0x3c)
#define     EFUSE_PGDATA1                                                     (EFUSE_BASE+0x40)
#define     EFUSE_PGDATA2                                                     (EFUSE_BASE+0x44)
#define     EFUSE_PGDATA3                                                     (EFUSE_BASE+0x48)


#define     InterruptController_BASE                                          0xe000e000
#define     NVIC_ISER0                                                        (InterruptController_BASE+0x00000100)
#define     NVIC_ISER1                                                        (InterruptController_BASE+0x00000104)
#define     NVIC_ICER0                                                        (InterruptController_BASE+0x00000180)
#define     NVIC_ICER1                                                        (InterruptController_BASE+0x00000184)
#define     NVIC_ISPR0                                                        (InterruptController_BASE+0x00000200)
#define     NVIC_ISPR1                                                        (InterruptController_BASE+0x00000204)
#define     NVIC_ICPR0                                                        (InterruptController_BASE+0x00000280)
#define     NVIC_ICPR1                                                        (InterruptController_BASE+0x00000284)
#define     NVIC_IABR0                                                        (InterruptController_BASE+0x00000300)
#define     NVIC_IABR1                                                        (InterruptController_BASE+0x00000304)
#define     NVIC_IPR0                                                         (InterruptController_BASE+0x00000400)
#define     NVIC_IPR1                                                         (InterruptController_BASE+0x00000404)
#define     NVIC_IPR2                                                         (InterruptController_BASE+0x00000408)
#define     NVIC_IPR3                                                         (InterruptController_BASE+0x0000040c)
#define     NVIC_IPR4                                                         (InterruptController_BASE+0x00000410)
#define     NVIC_IPR5                                                         (InterruptController_BASE+0x00000414)
#define     NVIC_IPR6                                                         (InterruptController_BASE+0x00000418)
#define     NVIC_IPR7                                                         (InterruptController_BASE+0x0000041c)
#define     NVIC_IPR8                                                         (InterruptController_BASE+0x00000420)
#define     NVIC_IPR9                                                         (InterruptController_BASE+0x00000424)


#define     TIMER_REGISTER_BASE                                               0x4000C100

/* For sys tick used */
#define     T0_CTL                                                            (TIMER_REGISTER_BASE+0x00)
#define     T0_VAL                                                            (TIMER_REGISTER_BASE+0x04)
#define     T0_CNT                                                            (TIMER_REGISTER_BASE+0x08)

/* For hrtimer used */
#define     T1_CTL                                                            (TIMER_REGISTER_BASE+0x20)
#define     T1_VAL                                                            (TIMER_REGISTER_BASE+0x24)
#define     T1_CNT                                                            (TIMER_REGISTER_BASE+0x28)

/* For system cycle used */
#define     T2_CTL                                                            (TIMER_REGISTER_BASE+0x40)
#define     T2_VAL                                                            (TIMER_REGISTER_BASE+0x44)
#define     T2_CNT                                                            (TIMER_REGISTER_BASE+0x48)


/* For tws used */
#define     T4_CTL                                                            (TIMER_REGISTER_BASE+0x80)
#define     T4_VAL                                                            (TIMER_REGISTER_BASE+0x84)
#define     T4_CNT                                                            (TIMER_REGISTER_BASE+0x88)

#define     RTC_REG_BASE      												  0x4000C000
#define     WD_CTL        													  (RTC_REG_BASE+0x20)
#define     HCL_CTL                                                           (RTC_REG_BASE+0x1D0)
#define     RTC_REMAIN0                                                       (RTC_REG_BASE+0x30)
#define     RTC_REMAIN1                                                       (RTC_REG_BASE+0x34)
#define     RTC_REMAIN2                                                       (RTC_REG_BASE+0x38)
#define     RTC_REMAIN3                                                       (RTC_REG_BASE+0x3C)
#define     RTC_REMAIN4                                                       (RTC_REG_BASE+0x40)
#define     RTC_REMAIN5                                                       (RTC_REG_BASE+0x44)

/* DAC control register */
#define     AUDIO_DAC_REG_BASE                                                0x4005C000

/* ADC control register  */
#define     AUDIO_ADC_REG_BASE                                                0x4005C100
#define     ADC_REF_LDO_CTL                                                   (AUDIO_ADC_REG_BASE + 0x48)

/* I2STX control register */
#define     AUDIO_I2STX0_REG_BASE                                             0x4005C400

/* I2SRX control register */
#define     AUDIO_I2SRX0_REG_BASE                                             0x4005C500

/* SPDIFTX control register */
#define     AUDIO_SPDIFTX_REG_BASE                                            0x4005C600

/* SPDIFRX control register */
#define     AUDIO_SPDIFRX_REG_BASE                                            0x4005C700

/*spi1*/
#define     SPI1_REGISTER_BASE                                                0x4002C000
#define     SPI1_CTL                                                          (SPI1_REGISTER_BASE+0x0000)
#define     SPI1_DDR_MODE_CTL                                                 (SPI1_REGISTER_BASE+0x0018)


/*anc*/
#define	    ANC_DS_CONTROL_REGISTER_BASE			0x4005C200

#define	    ANC_MIX_CTL								(ANC_DS_CONTROL_REGISTER_BASE+0x00)
#define	    ANC_DS_CTL								(ANC_DS_CONTROL_REGISTER_BASE+0x04)
#define	    ANC_DS_FF_CTL							(ANC_DS_CONTROL_REGISTER_BASE+0x08)
#define	    ANC_DS_FB_CTL							(ANC_DS_CONTROL_REGISTER_BASE+0x0c)
#define	    ALL_REG_ACCESS_SEL							(ANC_DS_CONTROL_REGISTER_BASE+0x54)

#define     SPI1_CLKGATING                                                    (SPI1_REGISTER_BASE+0x0038)

#define     CTK0_BASE                                                         0x4007C000
#define     CTK_CTL                                                           (CTK0_BASE+0x00)

/* uart */
#define UART0_CTL			(UART0_REG_BASE+0x00)
#define UART0_RXDAT			(UART0_REG_BASE+0x04)
#define UART0_TXDAT			(UART0_REG_BASE+0x08)
#define UART0_STA			(UART0_REG_BASE+0x0c)
#define UART0_BR			(UART0_REG_BASE+0x10)

#define UART1_CTL			(UART1_REG_BASE+0x00)
#define UART1_RXDAT			(UART1_REG_BASE+0x04)
#define UART1_TXDAT			(UART1_REG_BASE+0x08)
#define UART1_STA			(UART1_REG_BASE+0x0c)
#define UART1_BR			(UART1_REG_BASE+0x10)

#endif /* _ACTIONS_SOC_REGS_H_	*/
