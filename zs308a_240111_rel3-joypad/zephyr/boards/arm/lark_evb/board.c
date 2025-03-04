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

static const struct acts_pin_config board_pin_config[] = {
	/* uart0 */
	//{28, 5 | GPIO_CTL_SMIT | GPIO_CTL_PADDRV_LEVEL(4) },	/* UART0_TX */
	//{29, 5 | GPIO_CTL_SMIT | GPIO_CTL_PADDRV_LEVEL(4) },	/* UART0_RX */
};

static int board_early_init(const struct device *arg)
{
	ARG_UNUSED(arg);
	acts_pinmux_setup_pins(board_pin_config, ARRAY_SIZE(board_pin_config));
	sys_write32((sys_read32(RMU_MRCR1) | 1 << 3), RMU_MRCR1);
	sys_write32((sys_read32(CMU_DEVCLKEN1) | 1 << 3), CMU_DEVCLKEN1);
	sys_write32((sys_read32(0x4005C144) | 1 << 23), 0x4005C144);
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
	{AUDIO_ANALOG_MIC0, ADC_CH_INPUT0P, ADC_CH_DISABLE, ADC_CH_DISABLE, ADC_CH_DISABLE},
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

SYS_INIT(board_early_init, PRE_KERNEL_1, 5);

SYS_INIT(board_later_init, POST_KERNEL, 5);
