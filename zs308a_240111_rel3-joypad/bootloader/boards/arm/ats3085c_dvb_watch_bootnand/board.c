/* * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief board init functions
 */

#include <kernel.h>
#include <init.h>
#include <device.h>
#include <soc.h>
#include "board.h"
#include <drivers/gpio.h>
#include <board_cfg.h>

#define CONFIG_GPIO_HIGHZ  (0x1000)
static const struct acts_pin_config board_pin_reset_defautl[] = {

};
static void bootloader_pin_reset(void)
{
	acts_pinctl_reg_setup_pins(board_pin_reset_defautl, ARRAY_SIZE(board_pin_reset_defautl));
}
void boot_to_application(void)
{
	bootloader_pin_reset();
	boot_to_app(0, BOOT_FLASH_ID_NAND);
}

void boot_to_ota_app(void)
{
	//boot_to_part(5, 0, BOOT_FLASH_ID_NAND); // 5=temp part
}

static const struct acts_pin_config board_pin_config[] = {
    /*UART0 */
#if IS_ENABLED(CONFIG_UART_0)
    /* uart0 tx */
    PIN_MFP_SET(GPIO_28,  UART0_MFP_CFG),
    /* uart0 rx */
    PIN_MFP_SET(GPIO_29,  UART0_MFP_CFG),
#endif

    /*UART1 */
#if IS_ENABLED(CONFIG_UART_1)
    /* uart1 tx */
    PIN_MFP_SET(GPIO_16,  UART1_MFP_CFG),
    /* uart1 rx */
    PIN_MFP_SET(GPIO_17,  UART1_MFP_CFG),
#endif

#if IS_ENABLED(CONFIG_SPI_FLASH_0)
    /* SPI0 CS */
    PIN_MFP_SET(GPIO_0,   SPINOR_MFP_CFG),
    /* SPI0 MISO */
    PIN_MFP_SET(GPIO_1,   SPINOR_MFP_CFG),
    /* SPI0 CLK */
    PIN_MFP_SET(GPIO_2,   SPINOR_MFP_CFG),
    /* SPI0 MOSI */
    PIN_MFP_SET(GPIO_3,   SPINOR_MFP_CFG),
    /* SPI0 IO2 */
    PIN_MFP_SET(GPIO_6,   SPINOR_MFP_PU_CFG),
    /* SPI0 IO3 */
    PIN_MFP_SET(GPIO_7,   SPINOR_MFP_PU_CFG),
#endif

#if IS_ENABLED(CONFIG_ACTS_BATTERY_NTC)
    PIN_MFP_SET(GPIO_20,  BATNTC_MFP_CFG),
#endif

#if IS_ENABLED(CONFIG_I2CMT_0)
    /* I2C CLK*/
    PIN_MFP_SET(GPIO_49, I2CMT_MFP_CFG(MFP0_I2CMT)),
    /* I2C DATA*/
    PIN_MFP_SET(GPIO_50, I2CMT_MFP_CFG(MFP0_I2CMT)),
#endif

#if IS_ENABLED(CONFIG_I2CMT_1)
    /* I2C CLK*/
    PIN_MFP_SET(GPIO_51, I2CMT_MFP_CFG(MFP1_I2CMT)),
    /* I2C DATA*/
    PIN_MFP_SET(GPIO_52, I2CMT_MFP_CFG(MFP1_I2CMT)),
#endif

#if IS_ENABLED(CONFIG_I2C_0)
    /* I2C CLK*/
    PIN_MFP_SET(GPIO_57, I2C_MFP_CFG(MFP0_I2C)),
    /* I2C DATA*/
    PIN_MFP_SET(GPIO_58, I2C_MFP_CFG(MFP0_I2C)),
#endif

#if IS_ENABLED(CONFIG_I2C_1)
    /* I2C CLK*/
    PIN_MFP_SET(GPIO_59, I2C_MFP_CFG(MFP1_I2C)),
    /* I2C DATA*/
    PIN_MFP_SET(GPIO_60, I2C_MFP_CFG(MFP1_I2C)),
#endif

#if IS_ENABLED(CONFIG_CEC)
    PIN_MFP_SET(GPIO_12,  CEC_MFP_CFG),
#endif

#if IS_ENABLED(CONFIG_PANEL)
    /* lcd hsync */
    PIN_MFP_SET(GPIO_30, (GPIO_CTL_MFP(LCD_MFP_SEL)|GPIO_CTL_PULLUP)),
    /* lcd_wr_dclk_scl*/
    PIN_MFP_SET(GPIO_34, LCD_MFP_SEL),
    /* lcd_d0*/
    PIN_MFP_SET(GPIO_14, LCD_MFP_SEL),
    /* lcd_d1*/
    PIN_MFP_SET(GPIO_15, LCD_MFP_SEL),
    /* lcd_d2*/
    PIN_MFP_SET(GPIO_16, LCD_MFP_SEL),
    /* lcd_d3*/
    PIN_MFP_SET(GPIO_17, LCD_MFP_SEL),
#endif

#if IS_ENABLED(CONFIG_AUDIO_I2SRX_0)
    /*I2SRX0 mclk*/
    PIN_MFP_SET(GPIO_53,   I2SRX_MFP_CFG),
    /*I2SRX0 bclk*/
    PIN_MFP_SET(GPIO_54,   I2SRX_MFP_CFG),
    /*I2SRX0 lrclk*/
    PIN_MFP_SET(GPIO_55,   I2SRX_MFP_CFG),
    /*I2SRX0 d0*/
    PIN_MFP_SET(GPIO_56,   I2SRX_MFP_CFG),
#endif

#if IS_ENABLED(CONFIG_SPI_1)
    /*SPI SS*/
    PIN_MFP_SET(GPIO_24,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_25,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_26,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_27,  SPI_MFP_CFG(MFP_SPI1)),
#endif

#if IS_ENABLED(CONFIG_SPI_2)
    /*SPI SS*/
    PIN_MFP_SET(GPIO_30,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_31,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_32,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_33,  SPI_MFP_CFG(MFP_SPI1)),
#endif

#if IS_ENABLED(CONFIG_SPI_3)
    /*SPI SS*/
    PIN_MFP_SET(GPIO_20,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_21,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_22,  SPI_MFP_CFG(MFP_SPI1)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_23,  SPI_MFP_CFG(MFP_SPI1)),
#endif

#if IS_ENABLED(CONFIG_SPIMT_0)
    /* SPI SS*/
    PIN_MFP_SET(GPIO_49,  SPIMT_MFP_CFG(MFP0_SPIMT)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_50,  SPIMT_MFP_CFG(MFP0_SPIMT)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_51,  SPIMT_MFP_CFG(MFP0_SPIMT)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_52,  SPIMT_MFP_CFG(MFP0_SPIMT)),
    /* SPI SS1*/
    PIN_MFP_SET(GPIO_61,  SPIMT_MFP_CFG(MFP0_SPIMT)),
#endif

#if IS_ENABLED(CONFIG_SPIMT_1)
    /* SPI SS*/
    PIN_MFP_SET(GPIO_53,  SPIMT_MFP_CFG(MFP1_SPIMT)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_54,  SPIMT_MFP_CFG(MFP1_SPIMT)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_55,  SPIMT_MFP_CFG(MFP1_SPIMT)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_56,  SPIMT_MFP_CFG(MFP1_SPIMT)),
#endif
};

#if IS_ENABLED(CONFIG_MMC_0)
static const struct acts_pin_config board_mmc0_config[] = {
    /* MMC0 CMD*/
    PIN_MFP_SET(GPIO_11,   SDC0_MFP_CFG_VAL),
    /* MMC0 CLK*/
    PIN_MFP_SET(GPIO_10,  (GPIO_CTL_MFP(SDC0_MFP_SEL)|GPIO_CTL_PADDRV_LEVEL(3))),
    /* MMC0 DATA0 */
    PIN_MFP_SET(GPIO_12,   SDC0_MFP_CFG_VAL),
    /* MMC0 DATA1 */
    PIN_MFP_SET(GPIO_13,	SDC0_MFP_CFG_VAL),
    /* MMC0 DATA2 */
    PIN_MFP_SET(GPIO_8,    SDC0_MFP_CFG_VAL),
    /* MMC0 DATA2 */
    PIN_MFP_SET(GPIO_9,    SDC0_MFP_CFG_VAL),
};
#endif

#if IS_ENABLED(CONFIG_AUDIO_SPDIFTX_0)
static const struct acts_pin_config board_spdiftx0_config[] = {
    PIN_MFP_SET(GPIO_9, SPDIFTX_MFP_CFG),
};
#endif

#if IS_ENABLED(CONFIG_AUDIO_SPDIFRX_0)
static const struct acts_pin_config board_spdifrx0_config[] = {
    PIN_MFP_SET(GPIO_13,  SPDIFRX_MFP_CFG)
};
#endif

#if IS_ENABLED(CONFIG_AUDIO_I2STX_0)
static const struct acts_pin_config board_i2stx0_config[] = {
    /*I2STX0 mclk*/
    PIN_MFP_SET(GPIO_49, I2STX_MFP_CFG),
    /*I2STX0 bclk*/
    PIN_MFP_SET(GPIO_50, I2STX_MFP_CFG),
    /*I2STX0 lrclk*/
    PIN_MFP_SET(GPIO_51, I2STX_MFP_CFG),
    /*I2STX0 d0*/
    PIN_MFP_SET(GPIO_52, I2STX_MFP_CFG),
};
#endif

#if IS_ENABLED(CONFIG_SPINAND_0)
static const struct acts_pin_config board_spinand_spi0_config[] = {
    /* SPI0 CS */
    PIN_MFP_SET(GPIO_0,   SPINOR_MFP_CFG),
    /* SPI0 MISO */
    PIN_MFP_SET(GPIO_1,   SPINOR_MFP_CFG),
    /* SPI0 CLK */
    PIN_MFP_SET(GPIO_2,   SPINOR_MFP_CFG),
    /* SPI0 MOSI */
    PIN_MFP_SET(GPIO_3,   SPINOR_MFP_CFG),
    /* SPI0 IO2 */
    PIN_MFP_SET(GPIO_6,   SPINOR_MFP_PU_CFG),
    /* SPI0 IO3 */
    PIN_MFP_SET(GPIO_7,   SPINOR_MFP_PU_CFG),

};

static const struct acts_pin_config board_spinand_spi0_gpiohighz_config[] = {
	/*SPI0 CS*/
	PIN_MFP_SET(GPIO_0,  CONFIG_GPIO_HIGHZ),
	/*SPI0 MISO*/
	PIN_MFP_SET(GPIO_1,  CONFIG_GPIO_HIGHZ),
	/*SPI0 CLK*/
	PIN_MFP_SET(GPIO_2,  CONFIG_GPIO_HIGHZ),
	/*SPI0 MOSI*/
	PIN_MFP_SET(GPIO_3,  CONFIG_GPIO_HIGHZ),
	/*SPI0 IO2*/
	PIN_MFP_SET(GPIO_6,  CONFIG_GPIO_HIGHZ),
	/*SPI0 IO3*/
	PIN_MFP_SET(GPIO_7,  CONFIG_GPIO_HIGHZ),
};
#endif

#if IS_ENABLED(CONFIG_PWM)
/* Look at CONFIG_PWM_PIN_CHAN_MAP select the available pwm gpio */
static const struct pwm_acts_pin_config board_pwm_config[] = {
    /* GPIO5 used as pwm channel 1*/
    PWM_PIN_MFP_SET(GPIO_5, 1, PWM_MFP_CFG),
    /* GPIO21 used as pwm channel 7*/
    PWM_PIN_MFP_SET(GPIO_21, 7, PWM_MFP_CFG),
};
#endif

#if IS_ENABLED(CONFIG_ADCKEY)

#define CONFIG_ADCKEY_GPIO

#ifdef CONFIG_ADCKEY_GPIO
#define CONFIG_ADCKEY_GPIO_NUM (GPIO_21)
#else
#define CONFIG_ADCKEY_WIO_NUM  (WIO_0)
#define CONFIG_ADCKEY_WIO_MFP  (3)
#endif

static void board_adckey_pinmux_init(void)
{
#ifdef CONFIG_ADCKEY_GPIO
    acts_pinmux_set(CONFIG_ADCKEY_GPIO_NUM, ADCKEY_MFP_CFG);
#else
    sys_write32(CONFIG_ADCKEY_WIO_MFP, WIO0_CTL + (CONFIG_ADCKEY_WIO_NUM * 4));
#endif
}
#endif
static int board_early_init(const struct device *arg)
{
    ARG_UNUSED(arg);
    acts_pinmux_setup_pins(board_pin_config, ARRAY_SIZE(board_pin_config));

#if IS_ENABLED(CONFIG_MMC_0)
    acts_pinmux_setup_pins(board_mmc0_config, ARRAY_SIZE(board_mmc0_config));
#endif
#if IS_ENABLED(CONFIG_ADCKEY)
    board_adckey_pinmux_init();
#endif
    clk_ahb_set(1);
    return 0;
}

static int board_later_init(const struct device *arg)
{
    ARG_UNUSED(arg);
    printk("%s %d: \n", __func__, __LINE__);
    return 0;
}

/* UART registers struct */
struct acts_uart_reg {
    volatile uint32_t ctrl;

    volatile uint32_t rxdat;

    volatile uint32_t txdat;

    volatile uint32_t stat;

    volatile uint32_t br;
} ;

void uart_poll_out_ch(int c)
{
    struct acts_uart_reg *uart = (struct acts_uart_reg*)UART0_REG_BASE;
    /* Wait for transmitter to be ready */
    while (uart->stat &  BIT(6));
    /* send a character */
    uart->txdat = (uint32_t)c;

}
/*for early printk*/
int arch_printk_char_out(int c)
{
    if ('\n' == c)
        uart_poll_out_ch('\r');
    uart_poll_out_ch(c);
    return 0;
}

void board_get_mmc0_pinmux_info(struct board_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_MMC_0)
    pinmux_info->pins_config = board_mmc0_config;
    pinmux_info->pins_num = ARRAY_SIZE(board_mmc0_config);
#endif
}


void board_get_spdiftx0_pinmux_info(struct board_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_AUDIO_SPDIFTX_0)
    pinmux_info->pins_config = board_spdiftx0_config;
    pinmux_info->pins_num = ARRAY_SIZE(board_spdiftx0_config);
#endif
}

void board_get_spdifrx0_pinmux_info(struct board_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_AUDIO_SPDIFRX_0)
    pinmux_info->pins_config = board_spdifrx0_config;
    pinmux_info->pins_num = ARRAY_SIZE(board_spdifrx0_config);
#endif
}

void board_get_i2stx0_pinmux_info(struct board_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_AUDIO_SPDIFRX_0)
    pinmux_info->pins_config = board_i2stx0_config;
    pinmux_info->pins_num = ARRAY_SIZE(board_i2stx0_config);
#endif
}

void board_get_pwm_pinmux_info(struct board_pwm_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_PWM)
    pinmux_info->pins_config = board_pwm_config;
    pinmux_info->pins_num = ARRAY_SIZE(board_pwm_config);
#endif
}

void board_get_spinand_pinmux_info(struct board_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_SPINAND_0)
	pinmux_info->pins_config = board_spinand_spi0_config;
	pinmux_info->pins_num = ARRAY_SIZE(board_spinand_spi0_config);
#endif
}

void board_get_spinand_gpiohighz_info(struct board_pinmux_info *pinmux_info)
{
#if IS_ENABLED(CONFIG_SPINAND_0)
	pinmux_info->pins_config = board_spinand_spi0_gpiohighz_config;
	pinmux_info->pins_num = ARRAY_SIZE(board_spinand_spi0_gpiohighz_config);
#endif
}

SYS_INIT(board_early_init, PRE_KERNEL_1, 5);

SYS_INIT(board_later_init, POST_KERNEL, 5);
