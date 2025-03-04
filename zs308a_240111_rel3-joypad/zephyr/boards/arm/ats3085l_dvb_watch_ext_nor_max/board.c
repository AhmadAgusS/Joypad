/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
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

#if IS_ENABLED(CONFIG_SPINAND_3)
    /*SPI3 IO2*/
    PIN_MFP_SET(GPIO_8,   SPINAND_MFP_PU_CFG),
    /*SPI3 IO3*/
    PIN_MFP_SET(GPIO_9,   SPINAND_MFP_PU_CFG),
    /*SPI3 SS*/
    PIN_MFP_SET(GPIO_10,  SPINAND_MFP_CFG),
    /*SPI3 CLK*/
    PIN_MFP_SET(GPIO_11,  SPINAND_MFP_CFG),
    /*SPI3 IO1*/
    PIN_MFP_SET(GPIO_12,  SPINAND_MFP_CFG),
    /*SPI3 IO0*/
    PIN_MFP_SET(GPIO_13,  SPINAND_MFP_CFG),
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
    PIN_MFP_SET(GPIO_30,  SPI_MFP_CFG(MFP_SPI2)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_31,  SPI_MFP_CFG(MFP_SPI2)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_32,  SPI_MFP_CFG(MFP_SPI2)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_33,  SPI_MFP_CFG(MFP_SPI2)),
#endif

#if IS_ENABLED(CONFIG_SPI_3)
    /*SPI SS*/
    PIN_MFP_SET(GPIO_20,  SPI_MFP_CFG(MFP_SPI3)),
    /* SPI CLK*/
    PIN_MFP_SET(GPIO_21,  SPI_MFP_CFG(MFP_SPI3)),
    /* SPI MISO*/
    PIN_MFP_SET(GPIO_22,  SPI_MFP_CFG(MFP_SPI3)),
    /* SPI MOSI*/
    PIN_MFP_SET(GPIO_23,  SPI_MFP_CFG(MFP_SPI3)),
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
#ifdef CONFIG_RTT_CONSOLE
	jtag_set();
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

static const audio_input_map_t board_audio_input_map[] =  {
    {AUDIO_LINE_IN0, ADC_CH_INPUT0P, ADC_CH_DISABLE, ADC_CH_INPUT0N, ADC_CH_DISABLE},
    {AUDIO_LINE_IN1, ADC_CH_INPUT0NP_DIFF, ADC_CH_INPUT1NP_DIFF, ADC_CH_DISABLE, ADC_CH_DISABLE},
    {AUDIO_LINE_IN2, ADC_CH_DISABLE, ADC_CH_INPUT1P, ADC_CH_DISABLE, ADC_CH_INPUT1N},
    {AUDIO_ANALOG_MIC0, ADC_CH_INPUT0NP_DIFF, ADC_CH_DISABLE, ADC_CH_DISABLE, ADC_CH_DISABLE},
    {AUDIO_ANALOG_MIC1, ADC_CH_INPUT0NP_DIFF, ADC_CH_DISABLE, ADC_CH_DISABLE, ADC_CH_DISABLE},
    {AUDIO_ANALOG_FM0, ADC_CH_INPUT0P, ADC_CH_DISABLE, ADC_CH_INPUT0N, ADC_CH_DISABLE},
    {AUDIO_DIGITAL_MIC0, ADC_CH_DMIC, ADC_CH_DMIC, ADC_CH_DISABLE, ADC_CH_DISABLE},
};

int board_audio_device_mapping(audio_input_map_t *input_map)
{
    int i;

    if (!input_map)
        return -EINVAL;

    for (i = 0; i < ARRAY_SIZE(board_audio_input_map); i++) {
        if (input_map->audio_dev == board_audio_input_map[i].audio_dev) {
            input_map->ch0_input = board_audio_input_map[i].ch0_input;
            input_map->ch1_input = board_audio_input_map[i].ch1_input;
            input_map->ch2_input = board_audio_input_map[i].ch2_input;
            input_map->ch3_input = board_audio_input_map[i].ch3_input;
            break;
        }
    }

    if (i == ARRAY_SIZE(board_audio_input_map)) {
        printk("can not find out audio dev 0x%x\n", input_map->audio_dev);
        return -ENOENT;
    }

    return 0;
}

#ifdef CONFIG_BOARD_EXTERNAL_PA_ENABLE

#define EXTERN_PA_CTL1_PIN  25
#define EXTERN_PA_CTL_SLEEP_OFF_TIME_MS 160
#define EXTERN_PA_CTL_SLEEP_ON_TIME_MS 20
//#define EXTERN_PA_EIO_CTRL_PIN (0)
//#define EXTERN_PA_EIO_CTRL_PIN_NAME CONFIG_EXTEND_GPIO_NAME

int board_extern_pa_ctl(u8_t pa_class, bool is_on)
{
#ifdef EXTERN_PA_CTL1_PIN
    const struct device *pa_gpio_dev;
    pa_gpio_dev = device_get_binding(CONFIG_GPIO_PIN2NAME(EXTERN_PA_CTL1_PIN));
    if (!pa_gpio_dev)
        return -EINVAL;
#endif

#ifdef EXTERN_PA_EIO_CTRL_PIN
    const struct device *egpio_dev;
    egpio_dev = device_get_binding(EXTERN_PA_EIO_CTRL_PIN_NAME);
    if (!egpio_dev)
        return -EINVAL;
#endif

    if (!is_on) {
        /* external PA power off */
        printk("audio external PA power off\n");

#ifdef EXTERN_PA_CTL1_PIN
        gpio_pin_configure(pa_gpio_dev, EXTERN_PA_CTL1_PIN % 32, GPIO_OUTPUT);
        gpio_pin_set(pa_gpio_dev, EXTERN_PA_CTL1_PIN % 32, 0);
        /* XA7191D PA spec depicts that shall disable PA for above 150ms when switch work mode */
        k_sleep(K_MSEC(EXTERN_PA_CTL_SLEEP_OFF_TIME_MS));
#endif

#ifdef EXTERN_PA_EIO_CTRL_PIN
        gpio_pin_configure(egpio_dev, EXTERN_PA_EIO_CTRL_PIN % 32, GPIO_OUTPUT);
        gpio_pin_set(egpio_dev, EXTERN_PA_EIO_CTRL_PIN % 32, 0);
#endif
    } else {
        /* external PA power off */
        printk("audio external PA power on\n");

#ifdef EXTERN_PA_CTL1_PIN
        gpio_pin_configure(pa_gpio_dev, EXTERN_PA_CTL1_PIN % 32, GPIO_OUTPUT);
        gpio_pin_set(pa_gpio_dev, EXTERN_PA_CTL1_PIN % 32, 1);
        k_sleep(K_MSEC(EXTERN_PA_CTL_SLEEP_ON_TIME_MS));
#endif

#ifdef EXTERN_PA_EIO_CTRL_PIN
        gpio_pin_configure(egpio_dev, EXTERN_PA_EIO_CTRL_PIN % 32, GPIO_OUTPUT);
        gpio_pin_set(egpio_dev, EXTERN_PA_EIO_CTRL_PIN % 32, 1);
#endif
    }

    return 0;
}
#endif

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

SYS_INIT(board_early_init, PRE_KERNEL_1, 5);

SYS_INIT(board_later_init, POST_KERNEL, 5);
