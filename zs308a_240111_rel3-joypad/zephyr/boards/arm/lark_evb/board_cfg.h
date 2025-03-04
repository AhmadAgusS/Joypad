/*
 * Copyright (c) 2015 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BOARD_CFG_H
#define __BOARD_CFG_H
#include <drivers/cfg_drv/dev_config.h>
#include <soc.h>

/*
 *  The device module enables the definition, If 1, the corresponding module   is opened, the GPIO configuration is enabled,
 *		If 0, the corresponding module is closed, and the GPIO configuration is turned off
 */
#define CONFIG_GPIO_A							1
#define CONFIG_GPIO_A_NAME      			"GPIOA"
#define CONFIG_GPIO_B							1
#define CONFIG_GPIO_B_NAME      			"GPIOB"
#define CONFIG_GPIO_C							1
#define CONFIG_GPIO_C_NAME      			"GPIOC"
#define CONFIG_WIO      						1
#define CONFIG_WIO_NAME         			"WIO"
#define CONFIG_EXTEND_GPIO						0
#define CONFIG_EXTEND_GPIO_NAME             "GPIOD"

#define CONFIG_GPIO_PIN2NAME(x)          (((x) < 32) ? CONFIG_GPIO_A_NAME : (((x) < 64) ? CONFIG_GPIO_B_NAME : CONFIG_GPIO_C_NAME))


/*
spi nor flash cfg
*/
#define CONFIG_SPI_FLASH_0						1
#define CONFIG_SPI_FLASH_NAME      		"spi_flash"


#define CONFIG_ACTS_BATTERY						1
#define CONFIG_ACTS_BATTERY_DEV_NAME         "batadc"
/*
 *  Device module interrupt priority definition
 */
#define CONFIG_BTC_IRQ_PRI                		0

#define CONFIG_TWS_IRQ_PRI                		0

#define CONFIG_DMA_IRQ_PRI                      0


#define CONFIG_GPIO_IRQ_PRI                     0

#define CONFIG_WDT_0_IRQ_PRI                    0

#define CONFIG_DSP_IRQ_PRI                		0

#define CONFIG_SPI_FLASH_CHIP_SIZE      0x200000
#define CONFIG_SPI_FLASH_BUS_WIDTH      4
#define CONFIG_SPI_FLASH_DELAY_CHAIN    11
#define CONFIG_SPI_FLASH_MFP       gpio0_spinor0_cs_node,gpio1_spinor0_miso_node,gpio2_spinor0_clk_node,gpio3_spinor0_mosi_node,gpio6_spinor0_io2_node,gpio7_spinor0_io3_node
#define CONFIG_SPI_FLASH_FREQ_MHZ    	96

/*
mmc board cfg
*/
#define CONFIG_MMC_0                1
#define CONFIG_MMC_0_NAME           "MMC_0"
#define CONFIG_MMC_0_IRQ_PRI        0
#define CONFIG_MMC_0_BUS_WIDTH      4
#define CONFIG_MMC_0_CLKSEL         1     /*0 or 1, config by pinctrls*/
#define CONFIG_MMC_0_DATA_REG_WIDTH 4
#define CONFIG_MMC_0_MFP            gpio11_sdc0_cmd_node,gpio10_sdc0_clk_node,gpio12_sdc0_d0_node,gpio13_sdc0_d1_node,gpio8_sdc0_d2_node,gpio9_sdc0_d3_node
#define CONFIG_MMC_0_USE_DMA        1
#define CONFIG_MMC_0_DMA_CHAN       0xff
#define CONFIG_MMC_0_DMA_ID         5
#define CONFIG_MMC_0_USE_GPIO_IRQ   0
#define CONFIG_MMC_0_GPIO_IRQ_DEV   CONFIG_GPIO_A_NAME  /*CONFIG_GPIO_A_NAME&CONFIG_GPIO_B_NAME&CONFIG_GPIO_C_NAME*/
#define CONFIG_MMC_0_GPIO_IRQ_NUM   10    /*GPIOA10*/
#define CONFIG_MMC_0_GPIO_IRQ_FLAG  0    /*0=GPIO_ACTIVE_HIGH OR 1=GPIO_ACTIVE_LOW*/
#define CONFIG_MMC_0_ENABLE_SDIO_IRQ 0   /* If 1 to enable SD0 SDIO IRQ */

#define CONFIG_MMC_1                1
#define CONFIG_MMC_1_NAME           "MMC_1"
#define CONFIG_MMC_1_IRQ_PRI        0
#define CONFIG_MMC_1_BUS_WIDTH      4
#define CONFIG_MMC_1_CLKSEL         0
#define CONFIG_MMC_1_DATA_REG_WIDTH 1
#define CONFIG_MMC_1_MFP
#define CONFIG_MMC_1_USE_DMA        0
#define CONFIG_MMC_1_DMA_CHAN       0xff
#define CONFIG_MMC_1_DMA_ID         6
#define CONFIG_MMC_1_USE_GPIO_IRQ   0
#define CONFIG_MMC_1_ENABLE_SDIO_IRQ 0   /* If 1 to enable SD1 SDIO IRQ */

#define CONFIG_MMC_ACTS_ERROR_DETAIL    1 /* If 1 to print detail information when error occured */
#define CONFIG_MMC_WAIT_DAT1_BUSY       1 /* If 1 to wait SD/MMC card data 1 pin busy */
#define CONFIG_MMC_YIELD_WAIT_DMA_DONE  1 /* If  1 to yield task to wait DMA done */
#define CONFIG_MMC_SD0_FIFO_WIDTH_8BITS 0 /* If 1 to enable SD0 FIFO width 8 bits transfer */
#define CONFIG_MMC_STATE_FIFO           0 /* If 1 to enable using FIFO state for CPU read/write operations */

/*
sd board cfg
*/
#define CONFIG_SD                   1        /*0=disable 1=enable*/
#define CONFIG_SD_NAME              "sd"
#define CONFIG_SD_MMC_DEV           CONFIG_MMC_0_NAME /*CONFIG_MMC_0_NAME or CONFIG_MMC_1_NAME*/
#define CONFIG_SD_MMC_MFP           CONFIG_MMC_0_MFP
#define CONFIG_SD_USE_GPIO_DET      0
#define CONFIG_SD_GPIO_DET_NUM      21    /*GPIOB21*/
#define CONFIG_SD_GPIO_DET_LEVEL    0    /* The GPIO level(0: low voltage 1:high voltage) to indicates the SD card has been detected */
#define CONFIG_SD_USE_GPIO_POWER    0
#define CONFIG_SD_GPIO_POWER_NUM    21   /*GPIOB21*/
#define CONFIG_SD_GPIO_POWER_LEVEL  0    /* The GPIO level(0: low voltage 1:high voltage) to indicates the SD/eMMC card power on */
#define CONFIG_SD_CARD_POWER_RESET_MS    80 /* wait in milliseconds SD card to power off */
#define CONFIG_SD_CARD_HOTPLUG_DEBOUNCE_MS  100 /* SD card hot plug debounce */

#define CONFIG_MMC_SDCARD_RETRY_TIMES    2 /* mmc initialization retry times */
#define CONFIG_MMC_SDCARD_ERR_RETRY_NUM  2 /* mmc read/write retry if error happened */

#define CONFIG_MMC_SDCARD_LOW_POWER      0 /* If 1 to enable SD card low power */
#if (CONFIG_MMC_SDCARD_LOW_POWER == 1)
#define CONFIG_MMC_SDCARD_LOW_POWER_SLEEP   1 /* If 1 to check the mmc device can enter sleep */
#endif

#define CONFIG_MMC_SDCARD_SHOW_PERF      0 /* If 1 to enable sd card performance statistics */


/*
uart board cfg
*/
#define CONFIG_UART_0           1
#define CONFIG_UART_0_NAME      "UART_0"
#define CONFIG_UART_0_IRQ_PRI   0
#if 0
#define CONFIG_UART_0_SPEED     2000000
#else
//#define CONFIG_UART_0_SPEED     3000000
#define CONFIG_UART_0_SPEED     115200
#endif
#define CONFIG_UART_0_MFP       gpio28_uart0_tx_node,gpio29_uart0_rx_node
#define CONFIG_UART_0_USE_TX_DMA   1
#define CONFIG_UART_0_TX_DMA_CHAN  0x2
#define CONFIG_UART_0_TX_DMA_ID    1
#define CONFIG_UART_0_USE_RX_DMA   1
#define CONFIG_UART_0_RX_DMA_CHAN  0xff
#define CONFIG_UART_0_RX_DMA_ID    1


#define CONFIG_UART_1           0
//#define CONFIG_UART_1           1
//#define CONFIG_UART_1_NAME      "UART_1"
//#define CONFIG_UART_1_IRQ_PRI   0
//#define CONFIG_UART_1_SPEED     115200
//#define CONFIG_UART_1_MFP       gpio16_uart1_tx_node,gpio17_uart1_rx_node
//#define CONFIG_UART_1_USE_TX_DMA   0
//#define CONFIG_UART_1_TX_DMA_CHAN  0xff
//#define CONFIG_UART_1_TX_DMA_ID    2


#define CONFIG_UART_2           0

#define CONFIG_UART_3           0


//#define CONFIG_UART_4           0


/*
pwm board cfg
*/
#define CONFIG_PWM           1
#define CONFIG_PWM_NAME      "PWM"
#define CONFIG_PWM_CHANS     9
#define CONFIG_PWM_CYCLE     8000
#define CONFIG_PWM_MFP       gpio05_pwm_chan1_node
#define CONFIG_PWM_USE_DMA   1
#define CONFIG_PWM_DMA_CHAN  0xff
#define CONFIG_PWM_DMA_ID    21



/*
I2C board cfg
*/
#define CONFIG_I2C_0           0
#define CONFIG_I2C_0_NAME      "I2C_0"
#define CONFIG_I2C_0_IRQ_PRI   0
#define CONFIG_I2C_0_CLK_FREQ  100000
#define CONFIG_I2C_0_MFP       gpio28_i2c0_clk_node,gpio29_i2c0_data_node
#define CONFIG_I2C_0_USE_DMA   1
#define CONFIG_I2C_0_DMA_CHAN  0xff
#define CONFIG_I2C_0_DMA_ID    19
#define CONFIG_I2C_0_MAX_ASYNC_ITEMS 10


#define CONFIG_I2C_1           0
#define CONFIG_I2C_1_NAME      "I2C_1"
#define CONFIG_I2C_1_IRQ_PRI   0
#define CONFIG_I2C_1_CLK_FREQ  100000
#define CONFIG_I2C_1_MFP       gpio18_i2c1_clk_node,gpio19_i2c1_data_node
#define CONFIG_I2C_1_USE_DMA   0
#define CONFIG_I2C_1_DMA_CHAN  0xff
#define CONFIG_I2C_1_DMA_ID    20
#define CONFIG_I2C_1_MAX_ASYNC_ITEMS 3

/*
SPI board cfg
*/
#define CONFIG_SPI_1           0
#define CONFIG_SPI_1_NAME      "SPI_1"
#define CONFIG_SPI_1_IRQ_PRI   0
#define CONFIG_SPI_1_MFP       gpio24_spi1_ss_node,gpio25_spi1_clk_node,gpio26_spi1_miso_node,gpio27_spi1_mosi_node
#define CONFIG_SPI_1_USE_DMA   1
#define CONFIG_SPI_1_TXDMA_CHAN  0xff
#define CONFIG_SPI_1_RXDMA_CHAN  0xff
#define CONFIG_SPI_1_DMA_ID    8


#define CONFIG_SPI_2           0
#define CONFIG_SPI_2_NAME      "SPI_2"
#define CONFIG_SPI_2_IRQ_PRI   0
#define CONFIG_SPI_2_MFP       gpio30_spi2_ss_node,gpio31_spi2_clk_node,gpio32_spi2_miso_node,gpio33_spi2_mosi_node
#define CONFIG_SPI_2_USE_DMA   1
#define CONFIG_SPI_2_TXDMA_CHAN  0xff
#define CONFIG_SPI_2_RXDMA_CHAN  0xff
#define CONFIG_SPI_2_DMA_ID    9

#define CONFIG_SPI_3           0
#define CONFIG_SPI_3_NAME      "SPI_3"
#define CONFIG_SPI_3_IRQ_PRI   0
#define CONFIG_SPI_3_MFP       gpio20_spi3_ss_node,gpio21_spi3_clk_node,gpio22_spi3_miso_node,gpio23_spi3_mosi_node
#define CONFIG_SPI_3_USE_DMA   1
#define CONFIG_SPI_3_TXDMA_CHAN  0xff
#define CONFIG_SPI_3_RXDMA_CHAN  0xff
#define CONFIG_SPI_3_DMA_ID    10

/*
I2CMT cfg
*/
#define CONFIG_I2CMT_0           0
#define CONFIG_I2CMT_0_NAME      "I2CMT_0"
#define CONFIG_I2CMT_0_IRQ_PRI   0
#define CONFIG_I2CMT_0_CLK_FREQ  100000
#define CONFIG_I2CMT_0_USE_DMA   0
#define CONFIG_I2CMT_0_DMA_CHAN  0xff
#define CONFIG_I2CMT_0_MFP       gpio57_i2cmt0_clk_node,gpio58_i2cmt0_data_node

#define CONFIG_I2CMT_1           0
#define CONFIG_I2CMT_1_NAME      "I2CMT_1"
#define CONFIG_I2CMT_1_IRQ_PRI   0
#define CONFIG_I2CMT_1_CLK_FREQ  100000
#define CONFIG_I2CMT_1_USE_DMA   0
#define CONFIG_I2CMT_1_DMA_CHAN  0xff
#define CONFIG_I2CMT_1_MFP       gpio59_i2cmt1_clk_node,gpio60_i2cmt1_data_node

/*
SPIMT cfg
*/
#define CONFIG_SPIMT_0           0
#define CONFIG_SPIMT_0_NAME      "SPIMT_0"
#define CONFIG_SPIMT_0_IRQ_PRI   0
#define CONFIG_SPIMT_0_DMA_CHAN  0xff
#define CONFIG_SPIMT_0_MFP       gpio49_spimt0_ss_node,gpio50_spimt0_clk_node,gpio51_spimt0_miso_node,gpio52_spimt0_mosi_node

#define CONFIG_SPIMT_1           0
#define CONFIG_SPIMT_1_NAME      "SPIMT_1"
#define CONFIG_SPIMT_1_IRQ_PRI   0
#define CONFIG_SPIMT_1_DMA_CHAN  0xff
#define CONFIG_SPIMT_1_MFP       gpio53_spimt1_ss_node,gpio54_spimt1_clk_node,gpio55_spimt1_miso_node,gpio56_spimt1_mosi_node

/*
tp board cfg
*/
#define CONFIG_TP_RESET_GPIO  1
#define CONFIG_TP_RESET_GPIO_NAME   CONFIG_GPIO_A_NAME
#define CONFIG_TP_RESET_GPIO_NUM    12
#define CONFIG_TP_RESET_GPIO_FLAG   GPIO_ACTIVE_LOW

/*
audio board cfg
*/
/********************************** DAC CONFIGURATION **********************************/
/* AUDIO DAC device compatible name */
#define CONFIG_AUDIO_DAC_0_NAME                 "DAC_0"
/* AUDIO DAC interrupt priority */
#define CONFIG_AUDIO_DAC_0_IRQ_PRI              (0)
/* The DMA channel for DAC FIFO0  */
#define CONFIG_AUDIO_DAC_0_FIFO0_DMA_CHAN       (0xff)
/* The DMA slot ID for DAC FIFO0 */
#define CONFIG_AUDIO_DAC_0_FIFO0_DMA_ID         (0xb)
/* The DMA channel for DAC FIFO1  */
#define CONFIG_AUDIO_DAC_0_FIFO1_DMA_CHAN       (0xff)
/* The DMA slot ID for DAC FIFO1 */
#define CONFIG_AUDIO_DAC_0_FIFO1_DMA_ID         (0xc)

/**
 * The DAC working mode in dedicated PCB layout.
 * - 0: single-end(non-direct drive) mode
 * - 1: single-end(direct drive VRO) mode
 * - 2: differential mode
 */
#define CONFIG_AUDIO_DAC_0_LAYOUT               (2)

/**
 * The GL6189 DAC PA gain setting as below:
 * - 0: 0dB
 * - 1: -1.5dB
 * - 2: -3dB
 * - 3: -4.5dB
 * The LARK DAC PA gain setting as below:
 * PA gain    old plan    diff6db = 1    new plan
 *   7            0db	        6db	             --
 *   6	   -1.5db	     -7.5db	      --
 *   5	     -3db	   	-9db	     0db
 *   4	   -4.3db	    -10.3db	    -1.36db
 *   3	     -6db	      -12db	    -3db
 *   2	   -7.3db	    -13.3db	    -4.5db
 *   1	     -9db	      -15db	    -6db
 *   0	     mute	   	--		    mute
 */
#define CONFIG_AUDIO_DAC_0_PA_VOL               (0)

/* Enable DAC left and right channels mix function. */
#define CONFIG_AUDIO_DAC_0_LR_MIX               (0)

/* Enable DAC SDM(noise detect mute) function. */
#define CONFIG_AUDIO_DAC_0_NOISE_DETECT_MUTE    (0)

/* SDM mute counter configuration. */
#define CONFIG_AUDIO_DAC_0_SDM_CNT              (0x1000)

/* SDM noise dectection threshold */
#define CONFIG_AUDIO_DAC_0_SDM_THRES            (0xFFF)

/* Enable DAC automute function when continuously output 512x(configurable) samples 0 data. */
#define CONFIG_AUDIO_DAC_0_AUTOMUTE             (0)

/* Enable ADC loopback to DAC function. */
#define CONFIG_AUDIO_DAC_0_LOOPBACK             (0)

/* If 1 to mute the DAC left channel. */
#define CONFIG_AUDIO_DAC_0_LEFT_MUTE            (0)

/* If 1 to mute the DAC right channel. */
#define CONFIG_AUDIO_DAC_0_RIGHT_MUTE           (0)

/* Auto mute counter configuration. */
#define CONFIG_AUDIO_DAC_0_AM_CNT               (0x1000)

/* Auto noise dectection threshold */
#define CONFIG_AUDIO_DAC_0_AM_THRES             (0)

/* If 1 to enable DAC automute IRQ function. */
#define CONFIG_AUDIO_DAC_0_AM_IRQ               (0)

/* The threshold to generate half empty IRQ signal. */
#define CONFIG_AUDIO_DAC_0_PCMBUF_HE_THRES      (0x30)

/* The threshold to generate half full IRQ signal. */
#define CONFIG_AUDIO_DAC_0_PCMBUF_HF_THRES      (0x40)

/********************************** I2STX CONFIGURATION **********************************/
/* AUDIO I2STX device compatible name */
#define CONFIG_AUDIO_I2STX_0_NAME               "I2STX_0"
/* AUDIO I2STX interrupt priority */
#define CONFIG_AUDIO_I2STX_0_IRQ_PRI            (0)
/* The DMA channel for I2STX FIFO0  */
#define CONFIG_AUDIO_I2STX_0_FIFO0_DMA_CHAN     (0xff)
/* The DMA slot ID for I2STX FIFO0 */
#define CONFIG_AUDIO_I2STX_0_FIFO0_DMA_ID       (0xe)

/**
 * I2STX channel number selection.
 *   - 2: 2 channels
 *   - 4: 4 channels(TDM)
 *   - 8: 8 channels(TDM)
 */
#define CONFIG_AUDIO_I2STX_0_CHANNEL_NUM        (2)

/**
 * I2STX transfer format selection.
 *   - 0: I2S format
 *   - 1: left-justified format
 *   - 2: right-justified format
 *   - 3: TDM format
 */
#define CONFIG_AUDIO_I2STX_0_FORMAT             (0)

/**
 * I2STX BCLK data width.
 *   - 0: 32bits
 *   - 1: 16bits
 */
#define CONFIG_AUDIO_I2STX_0_BCLK_WIDTH         (0)

/* Enable the SRD(sample rate detect) function. */
#define CONFIG_AUDIO_I2STX_0_SRD_EN             (0)

/**
 * I2STX master or slaver mode selection.
 *   - 0: master
 *   - 1: slaver
 */
#define CONFIG_AUDIO_I2STX_0_MODE               (0)

/* Enable in slave mode MCLK to use internal clock. */
#define CONFIG_AUDIO_I2STX_0_SLAVE_INTERNAL_CLK (0)

/**
 * I2STX LRCLK process selection.
 *   - 0: 50% duty
 *   - 1: 1 BCLK
 */
#define CONFIG_AUDIO_I2STX_0_LRCLK_PROC         (0)

/**
 * I2STX MCLK reverse selection.
 *   - 0: normal
 *   - 1: reverse
 */
#define CONFIG_AUDIO_I2STX_0_MCLK_REVERSE       (0)

/* Enable I2STX channel BCLK/LRCLK alway existed which used in master mode. */
#define CONFIG_AUDIO_I2STX_0_ALWAYS_OPEN        (0)

/**
 * I2STX transfer TDM format selection.
 *   - 0: I2S format
 *   - 1: left-justified format
 */
#define CONFIG_AUDIO_I2STX_0_TDM_FORMAT         (0)

/**
 *  I2STX TDM frame start position selection.
 *   - 0: the rising edge of LRCLK with a pulse.
 *   - 1: the rising edge of LRCLK with a 50% duty cycle.
 *   - 2: the falling edge of LRCLK with a 50% duty cycle.
 */
#define CONFIG_AUDIO_I2STX_0_TDM_FRAME          (0)

/**
 * I2STX data output delay selection.
 *   - 0: 2 mclk cycles after the bclk rising edge.
 *   - 1: 3 mclk cycles after the bclk rising edge.
 *   - 2: 4 mclk cycles after the bclk rising edge.
 *   - 3: 5 mclk cycles after the bclk rising edge.
 */
#define CONFIG_AUDIO_I2STX_0_TX_DELAY           (0)

/* Pins select for I2S-TX MCLK/BCLK/LRCLK/DATAs signals. */
#define CONFIG_AUDIO_I2STX_0_MFP gpio39_i2stx0_d0_node,gpio16_i2stx0_mclk_node,gpio17_i2stx0_bclk_node,gpio18_i2stx0_lrclk_node

/********************************** SPDIFTX CONFIGURATION **********************************/
/* AUDIO SPDIFTX device compatible name */
#define CONFIG_AUDIO_SPDIFTX_0_NAME             "SPDIFTX_0"

/* Enable the clock of SPDIFTX source from I2STX div2 clock. */
#define CONFIG_AUDIO_SPDIFTX_0_CLK_I2STX_DIV2       (0)

/* Pins select for SPDIF-RX Data signals. */
#define CONFIG_AUDIO_SPDIFTX_0_MFP gpio9_spdiftx0_d0_node

/********************************** ADC CONFIGURATION **********************************/
/* AUDIO ADC device compatible name */
#define CONFIG_AUDIO_ADC_0_NAME                 "ADC_0"
/* AUDIO ADC interrupt priority */
#define CONFIG_AUDIO_ADC_0_IRQ_PRI              (0)
/* The DMA channel for ADC FIFO0  */
#define CONFIG_AUDIO_ADC_0_FIFO0_DMA_CHAN       (0xff)
/* The DMA slot ID for ADC FIFO0 */
#define CONFIG_AUDIO_ADC_0_FIFO0_DMA_ID         (0xb)
/* The DMA channel for ADC FIFO1  */
#define CONFIG_AUDIO_ADC_0_FIFO1_DMA_CHAN       (0xff)
/* The DMA slot ID for ADC FIFO1 */
#define CONFIG_AUDIO_ADC_0_FIFO1_DMA_ID         (0xc)

/**
 * ADC0 channel HPF auto-set time selection.
 *   - 0: 1.3ms in 48kfs
 *   - 1: 5ms in 48kfs
 *   - 2: 10ms in 48kfs
 *   - 3: 20ms in 48kfs
 */
#define CONFIG_AUDIO_ADC_0_CH0_HPF_TIME         (1)

/* ADC channel0 frequency which range from 0 ~ 111111b. */
#define CONFIG_AUDIO_ADC_0_CH0_FREQUENCY        (0)

/* If 1 to enable ADC channel0 HPF high frequency range. */
#define CONFIG_AUDIO_ADC_0_CH0_HPF_FC_HIGH      (0)

/**
 * ADC channel1 HPF auto-set time selection.
 *   - 0: 1.3ms in 48kfs
 *   - 1: 5ms in 48kfs
 *   - 2: 10ms in 48kfs
 *   - 3: 20ms in 48kfs
 */
#define CONFIG_AUDIO_ADC_0_CH1_HPF_TIME         (1)

/* ADC channel1 frequency which range from 0 ~ 111111b. */
#define CONFIG_AUDIO_ADC_0_CH1_FREQUENCY        (0)

/* If 1 to enable ADC channel1 HPF high frequency range. */
#define CONFIG_AUDIO_ADC_0_CH1_HPF_FC_HIGH      (0)

/**
 * ADC channel2 HPF auto-set time selection.
 *   - 0: 1.3ms in 48kfs
 *   - 1: 5ms in 48kfs
 *   - 2: 10ms in 48kfs
 *   - 3: 20ms in 48kfs
 */
#define CONFIG_AUDIO_ADC_0_CH2_HPF_TIME         (1)

/* ADC channel2 frequency which range from 0 ~ 111111b. */
#define CONFIG_AUDIO_ADC_0_CH2_FREQUENCY        (0)

/* If 1 to enable ADC channel2 HPF high frequency range. */
#define CONFIG_AUDIO_ADC_0_CH2_HPF_FC_HIGH      (0)

/**
 * ADC channel3 HPF auto-set time selection.
 *   - 0: 1.3ms in 48kfs
 *   - 1: 5ms in 48kfs
 *   - 2: 10ms in 48kfs
 *   - 3: 20ms in 48kfs
 */
#define CONFIG_AUDIO_ADC_0_CH3_HPF_TIME         (1)

/* ADC channel3 frequency which range from 0 ~ 111111b. */
#define CONFIG_AUDIO_ADC_0_CH3_FREQUENCY        (0)

/* If 1 to enable ADC channel3 HPF high frequency range. */
#define CONFIG_AUDIO_ADC_0_CH3_HPF_FC_HIGH      (0)

/**
 * Audio LDO output voltage selection.
 *   - 0: 1.6v
 *   - 1: 1.7v
 *   - 2: 1.8v
 *   - 3: 1.9v
 */
#define CONFIG_AUDIO_ADC_0_LDO_VOLTAGE          (1)

/*
 * Audio VMIC control MIC power as <vmic-ctl0, vmic-ctl1, vmic-ctl2>.
 *   - 0x: disable VMIC OP
 *   - 2: bypass VMIC OP
 *   - 3: enable VMIC OP
 */
#define CONFIG_AUDIO_ADC_0_VMIC_CTL_ARRAY       {3, 3, 3}

/**
 * Audio VMIC control the MIC voltage as <vmic-vol0, vmic-vol1>.
 *   - 0: 0.8 AVCC
 *   - 1: 0.85 AVCC
 *   - 2: 0.9 AVCC
 *   - 3: 0.95 AVCC
 */
#define CONFIG_AUDIO_ADC_0_VMIC_VOLTAGE_ARRAY   {2, 2, 2}

/* Enable ADC fast capacitor charge function. */
#define CONFIG_AUDIO_ADC_0_FAST_CAP_CHARGE      (0)

/********************************** I2SRX CONFIGURATION **********************************/
/* AUDIO I2SRX device compatible name */
#define CONFIG_AUDIO_I2SRX_0_NAME               "I2SRX_0"
/* AUDIO I2SRX interrupt priority */
#define CONFIG_AUDIO_I2SRX_0_IRQ_PRI            (0)
/* The DMA channel for I2SRX FIFO0  */
#define CONFIG_AUDIO_I2SRX_0_FIFO0_DMA_CHAN     (0xff)
/* The DMA slot ID for I2SRX FIFO0 */
#define CONFIG_AUDIO_I2SRX_0_FIFO0_DMA_ID       (0xe)

/**
 * I2SRX channel number selection.
 *   - 2: 2 channels
 *   - 4: 4 channels(TDM)
 *   - 8: 8 channels(TDM)
 */
#define CONFIG_AUDIO_I2SRX_0_CHANNEL_NUM        (2)

/**
 * I2SRX transfer format selection.
 *   - 0: I2S format
 *   - 1: left-justified format
 *   - 2: right-justified format
 *   - 3: TDM format
 */
#define CONFIG_AUDIO_I2SRX_0_FORMAT             (0)

/**
 * I2SRX BCLK data width.
 *   - 0: 32bits
 *   - 1: 16bits
 */
#define CONFIG_AUDIO_I2SRX_0_BCLK_WIDTH         (0)

/* Enable the SRD(sample rate detect) function. */
#define CONFIG_AUDIO_I2SRX_0_SRD_EN             (0)

/**
 * I2SRX master or slaver mode selection.
 *   - 0: master
 *   - 1: slaver
 */
#define CONFIG_AUDIO_I2SRX_0_MODE               (0)

/* Enable in slave mode MCLK to use internal clock. */
#define CONFIG_AUDIO_I2SRX_0_SLAVE_INTERNAL_CLK    (0)

/**
 * I2SRX LRCLK process selection.
 *   - 0: 50% duty
 *   - 1: 1 BCLK
 */
#define CONFIG_AUDIO_I2SRX_0_LRCLK_PROC         (0)

/**
 * I2SRX MCLK reverse selection.
 *   - 0: normal
 *   - 1: reverse
 */
#define CONFIG_AUDIO_I2SRX_0_MCLK_REVERSE       (0)

/**
 * I2SRX transfer TDM format selection.
 *   - 0: I2S format
 *   - 1: left-justified format
 */
#define CONFIG_AUDIO_I2SRX_0_TDM_FORMAT         (0)

/**
 *  I2SRX TDM frame start position selection.
 *   - 0: the rising edge of LRCLK with a pulse.
 *   - 1: the rising edge of LRCLK with a 50% duty cycle.
 *   - 2: the falling edge of LRCLK with a 50% duty cycle.
 */
#define CONFIG_AUDIO_I2SRX_0_TDM_FRAME          (0)

/* If 1 to enable the I2SRX clock source from I2STX. */
#define CONFIG_AUDIO_I2SRX_0_CLK_FROM_I2STX     (0)

/* Pins select for I2S-RX MCLK/BCLK/LRCLK/DATAs signals. */
#define CONFIG_AUDIO_I2SRX_0_MFP gpio43_i2srx0_d0_node,gpio19_i2srx0_mclk_node,gpio20_i2srx0_bclk_node,gpio21_i2srx0_lrclk_node

/********************************** SPDIFRX CONFIGURATION **********************************/
/* AUDIO SPDIFRX device compatible name */
#define CONFIG_AUDIO_SPDIFRX_0_NAME          "SPDIFRX_0"
/* AUDIO SPDIFRX interrupt priority */
#define CONFIG_AUDIO_SPDIFRX_0_IRQ_PRI           (0)
/* The DMA channel for SPDIFRX FIFO0  */
#define CONFIG_AUDIO_SPDIFRX_0_FIFO0_DMA_CHAN    (0xff)
/* The DMA slot ID for SPDIFRX FIFO0 */
#define CONFIG_AUDIO_SPDIFRX_0_FIFO0_DMA_ID      (0x10)

/* Specify minimal CORE_PLL clock for spdifrx. */
#define CONFIG_AUDIO_SPDIFRX_0_MIN_COREPLL_CLOCK (50000000)

/* Pins select for SPDIF-RX Data signals. */
#define CONFIG_AUDIO_SPDIFRX_0_MFP gpio13_spdifrx0_d0_node

/*
 * panel cfg
 */
/*
 * b'[15:8]: major type, 1-CPU, 4-SPI
 * b'[7-0]: minor type,
 *          For CPU, 0-Intel_8080, 1-Moto_6800
 *          For SPI, 0-3wire_type1, 1-3wire_type2, 2-4wire_type1, 3-4wire_type2, 4-quad, 5-quad_sync
 */
#define CONFIG_PANEL_PORT_TYPE		((4 << 8) | (4))
#define CONFIG_PANEL_PORT_CS		(0)
#define CONFIG_PANEL_PORT_SPI_DELAY_CHAIN_NS	(8)
#define CONFIG_PANEL_PORT_SPI_DUAL_DATA_LANE	(1)

#define CONFIG_PANEL_TIMING_HACTIVE	(360)
#define CONFIG_PANEL_TIMING_VACTIVE	(360)
#define CONFIG_PANEL_TIMING_TE_ACTIVE	(1)

#define CONFIG_PANEL_MFP			gpio30_lcd_ce0_hsync_node,gpio34_lcd_wr_dclk_scl_node,gpio14_lcd_d0_node,gpio15_lcd_d1_node,gpio16_lcd_d2_node,gpio17_lcd_d3_node
#define CONFIG_PANEL_TE_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_B_NAME, 3, GPIO_ACTIVE_HIGH, 1)
#define CONFIG_PANEL_RESET_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_B_NAME, 1, GPIO_ACTIVE_HIGH, 1)
#define CONFIG_PANEL_SLEEP_SW		(1)

#define CONFIG_PANEL_TE_SCANLINE	(340)

/*
 * tp cfg
 */
#define CONFIG_TPKEY_I2C_NAME		CONFIG_I2C_1_NAME
#define CONFIG_TPKEY_ISR_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_A_NAME, 26, GPIO_ACTIVE_LOW, 1)
#define CONFIG_TPKEY_RESET_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_A_NAME, 25, GPIO_ACTIVE_LOW, 1)
//#define CONFIG_TPKEY_POWER_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_A_NAME, 31, GPIO_ACTIVE_HIGH, 1)
//#define CONFIG_TPKEY_LOWPOWER		(1)

/*
PMU cfg
*/
/* PMU interrupt priority */
#define CONFIG_PMU_IRQ_PRI                       (0)

/* If 1 to enable the ON-OFF key short press detection function. */
#define CONFIG_PMU_ONOFF_SHORT_DETECT            (0)

/* If 1 to indicates that ON-OFF key and REMOTE key use the same WIO */
#define CONFIG_PMU_ONOFF_REMOTE_SAME_WIO         (0)

/*
PMUADC cfg
*/

/** PMUADC battery channel over sampling counter
 *   - 0: disable over sampling
 *   - 1: 8 times
 *   - 2: 32 times
 *   - 3: 128 times
 */
#define CONFIG_PMUADC_BAT_AVG_CNT                (1)

/** PMUADC LRADC1 channel over sampling counter
 *   - 0: disable over sampling
 *   - 1: 8 times
 *   - 2: 32 times
 *   - 3: 128 times
 */
#define CONFIG_PMUADC_LRADC1_AVG                 (1)

/**
 * PMU ADC LRADC clock source selection.
 *   - 0: RC32K
 *   - 1: reserved
 *   - 2: RC4M/16
 *   - 3: RC4M
 *   - 4: HOSC/8
 *   - 5: HOSC/128
 */
#define CONFIG_PMUADC_CLOCK_SOURCE               (2)

/**
 * PMU ADC LRADC clock source divisor selection.
 *   - 0: /1
 *   - 1: /2
 *   - 2: /4
 *   - 3: /8
 */
#define CONFIG_PMUADC_CLOCK_DIV                  (0)

/**
 * PMU ADC previous buffer current BIAS selection.
 *   - 0: 0.25uA
 *   - 1: 0.5uA
 *   - 2: 0.75uA
 *   - 3: 1uA
 */
#define CONFIG_PMUADC_IBIAS_BUF_SEL              (1)

/**
 * PMU ADC core current BIAS selection.
 *   - 0: 0.25uA
 *   - 1: 0.5uA
 *   - 2: 0.75uA
 *   - 3: 1uA
 */
#define CONFIG_PMUADC_IBIAS_ADC_SEL              (1)

/* The timeout of sync counter8hz */
#define CONFIG_PMU_COUNTER8HZ_SYNC_TIMEOUT_US    (200000)

/* If 1 to enable backup time when power off */
#define CONFIG_PM_BACKUP_TIME_FUNCTION_EN        (1)

#define CONFIG_PM_BACKUP_TIME_NVRAM_ITEM_NAME    "PM_BAK_TIME"
/*
RTC cfg
*/

/*
RTC cfg
*/
/* RTC interrupt priority */
#define CONFIG_RTC_IRQ_PRI                       (0)

/**
 *  The RTC clock source selection.
 * - 0: RTC_CLKSRC_HOSC_4HZ
 * - 1: RTC_CLKSRC_HCL_LOSC_100HZ
 */
#define CONFIG_RTC_CLK_SOURCE                    (1)

#endif /* __BOARD_CFG_H */
