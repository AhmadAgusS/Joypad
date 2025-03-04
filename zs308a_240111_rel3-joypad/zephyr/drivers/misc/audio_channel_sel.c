/*
 * Copyright (c) 2017 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief I2C master driver for Actions SoC
 */

#include <errno.h>
#include <sys/__assert.h>
#include <stdbool.h>
#include <kernel.h>
#include <device.h>
#include <init.h>
#include <drivers/misc/audio_channel_sel.h>
#include <soc.h>
#include <drivers/adc.h>
#include <soc_pmu.h>
#include <drivers/gpio.h>

//#include <irq.h>
//#include <board.h>

#include <soc_pmu.h>

#define LRADC_CTRL_NONE 0xff

int audio_acts_chan_sel(const struct device *dev, struct audio_chansel_data *data)
{

	u32_t lradc_ctrl;

	lradc_ctrl = data->CFG_Type_Channel_Select_LRADC.LRADC_Ctrl;

	if(lradc_ctrl != LRADC_CTRL_NONE) {

		u32_t ctrl_no = lradc_ctrl & 0xff;
		u32_t gpio_no = (lradc_ctrl & 0xff00) >> 8;
		u32_t mfp_sel = (lradc_ctrl & 0xff0000) >> 16;

		struct adc_channel_cfg channel_cfg = {0};

		channel_cfg.channel_id = ctrl_no;

		struct device *adc;

		adc = device_get_binding("PMUDAC_0");

		if (adc_channel_setup(adc, &channel_cfg)) {
			LOG_ERR("setup channel_id %d error", channel_cfg.channel_id);
			return -EFAULT;
		}

		if(gpio_no < GPIO_MAX_PIN_NUM)
			sys_write32(mfp_sel, GPIO_REG_BASE + (1 + gpio_no)*4);

		struct adc_sequence sequence;
		uint8_t adc_buf[4];

		sequence.channels = BIT(ctrl_no);
		sequence.buffer = &adc_buf[0];
		sequence.buffer_size = sizeof(adc_buf);

		int ret;

		ret = adc_read(adc, &sequence);

		ret = ((uint16_t)adc_buf[1] << 8) | adc_buf[0];

		return ret;

	} else {

		u32_t pin = data->CFG_Type_Channel_Select_GPIO.GPIO_Pin;
		u32_t pull = data->CFG_Type_Channel_Select_GPIO.Pull_Up_Down;
		u32_t active = data->CFG_Type_Channel_Select_GPIO.Active_Level;
		u32_t val;

		if(pull)
			sys_write32(0x8a0, GPIO_REG_BASE + (1 + pin)*4);
		else
			sys_write32(0x2a0, GPIO_REG_BASE + (1 + pin)*4);

		val = !!(sys_read32(GPIO_REG_IDAT(GPIO_REG_BASE, pin)) & GPIO_BIT(pin));

		return val;
	}
	return 0;
}

int audio_acts_chan_sel_init(const struct device *dev)
{

	return 0;
}

const struct audio_chan_sel_driver_api audio_acts_chan_sel_driver_api = {
	.chan_sel = audio_acts_chan_sel,
};

struct audio_chansel_data audio_acts_chansel_data;

DEVICE_DEFINE(audio_channel_sel, "audio_channel_sel",		\
		    &audio_acts_chan_sel_init,	NULL, 		\
		    &audio_acts_chansel_data, NULL,	\
		    POST_KERNEL, CONFIG_AUDIO_CHAN_SEL_INIT_PRIORITY,	\
		    &audio_acts_chan_sel_driver_api);




