/*
 * Copyright (c) 2015 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <board_cfg.h>
#include <drivers/audio/audio_common.h>

#define CONFIG_SD_USE_GPIO_DET      0
#define CONFIG_SD_GPIO_DET_NUM      64    /*GPIO64*/
#define CONFIG_SD_GPIO_DET_LEVEL    0    /* The GPIO level(0: low voltage 1:high voltage) to indicates the SD card has been detected */
#define CONFIG_SD_USE_GPIO_POWER    1
#define CONFIG_SD_GPIO_POWER_NUM    64   /*GPIO64*/
#define CONFIG_SD_GPIO_POWER_LEVEL  0    /* The GPIO level(0: low voltage 1:high voltage) to indicates the SD/eMMC card power on */

#define CONFIG_PANEL_TE_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_B_NAME, 3, GPIO_ACTIVE_HIGH, 1)
#define CONFIG_PANEL_RESET_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_B_NAME, 1, GPIO_ACTIVE_LOW, 1)
#define CONFIG_PANEL_POWER_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_A_NAME, 5, GPIO_ACTIVE_HIGH, 1)

#define CONFIG_TPKEY_ISR_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_B_NAME, 0, GPIO_ACTIVE_LOW, 1)
#define CONFIG_TPKEY_RESET_GPIO		GPIO_CFG_MAKE(CONFIG_EXTEND_GPIO_NAME, 9, GPIO_ACTIVE_LOW, 1)
#define CONFIG_TPKEY_POWER_GPIO		GPIO_CFG_MAKE(CONFIG_GPIO_A_NAME, 19, GPIO_ACTIVE_HIGH, 1)

#if IS_ENABLED(CONFIG_GPIOKEY)
	/* The GPIO MFP for GPIO KEY */
#define CONFIG_GPIOKEY_MFP    PIN_MFP_SET(GPIO_21,   GPIOKEY_MFP_CFG)
#endif

#define CONFIG_SPINAND_USE_GPIO_POWER       1
#define CONFIG_SPINAND_POWER_GPIO           64    /* GPIO64 */
#define CONFIG_SPINAND_GPIO_POWER_LEVEL     1     /* The GPIO level(0: low voltage 1:high voltage) to indicates the spinand power on */


#define BOARD_BATTERY_CAP_MAPPINGS      \
	{0, 3200},  \
	{5, 3300},  \
	{10, 3400}, \
	{20, 3550}, \
	{30, 3650}, \
	{40, 3750}, \
	{50, 3800}, \
	{60, 3850}, \
	{70, 3900}, \
	{80, 3950}, \
	{90, 4000}, \
	{100, 4050},

#define BOARD_ADCKEY_KEY_MAPPINGS	\
	{KEY_ADFU,          0x05}, \
	{KEY_PREVIOUSSONG,	0x376},	\
	{KEY_MENU,		    0x532},	\
	{KEY_TBD,	        0x69C},	\
	{KEY_VOLUMEDOWN,    0x86D}, \
	{KEY_VOLUMEUP,	    0x9EE}, \
	{KEY_NEXTSONG,	    0xB87},

/* @brief The macro to define an audio device */
#define AUDIO_LINE_IN0 (AUDIO_DEV_TYPE_LINEIN | 0)
#define AUDIO_LINE_IN1 (AUDIO_DEV_TYPE_LINEIN | 1)
#define AUDIO_LINE_IN2 (AUDIO_DEV_TYPE_LINEIN | 2)
#define AUDIO_ANALOG_MIC0 (AUDIO_DEV_TYPE_AMIC | 0)
#define AUDIO_ANALOG_MIC1 (AUDIO_DEV_TYPE_AMIC | 1)
#define AUDIO_ANALOG_MIC2 (AUDIO_DEV_TYPE_AMIC | 2)
#define AUDIO_ANALOG_FM0 (AUDIO_DEV_TYPE_FM | 0)
#define AUDIO_DIGITAL_MIC0 (AUDIO_DEV_TYPE_DMIC | 0)
#define AUDIO_DIGITAL_MIC1 (AUDIO_DEV_TYPE_DMIC | 1)

/* config for battery NTC */
#define BATNTC_MFP_SEL          28
#define BATNTC_MFP_CFG (GPIO_CTL_MFP(BATNTC_MFP_SEL) | GPIO_CTL_SMIT | GPIO_CTL_PADDRV_LEVEL(3))

struct board_pinmux_info{
	const struct acts_pin_config *pins_config;
	int pins_num;
};

struct pwm_acts_pin_config{
	unsigned int pin_num;
	unsigned int pin_chan;
	unsigned int mode;
};

struct board_pwm_pinmux_info{
	const struct pwm_acts_pin_config *pins_config;
	int pins_num;
};


extern void board_get_mmc0_pinmux_info(struct board_pinmux_info *pinmux_info);
extern void board_get_spdiftx0_pinmux_info(struct board_pinmux_info *pinmux_info);
extern void board_get_spdifrx0_pinmux_info(struct board_pinmux_info *pinmux_info);
extern void board_get_i2stx0_pinmux_info(struct board_pinmux_info *pinmux_info);
extern void board_get_pwm_pinmux_info(struct board_pwm_pinmux_info *pinmux_info);
extern void board_get_spinand_pinmux_info(struct board_pinmux_info *pinmux_info);
extern void board_get_spinand_gpiohighz_info(struct board_pinmux_info *pinmux_info);

/* @brief Get the mapping relationship between the hardware inputs and audio devices */
int board_audio_device_mapping(audio_input_map_t *input_map);
int board_extern_pa_ctl(u8_t pa_class, bool is_on);

#endif /* __INC_BOARD_H */
