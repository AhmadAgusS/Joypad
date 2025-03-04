/*!
 * \file      driver_item.h
 * \brief     驱动配置项定义
 * \details
 * \author
 * \date
 * \copyright Actions
 */


#ifndef ZEPHYR_INCLUDE_DRIVERS_DRIVER_ITEM_H_
#define ZEPHYR_INCLUDE_DRIVERS_DRIVER_ITEM_H_

#define UART_DRV_ID 0X03
#define LED_DRV_ID 0X11
#define ONOFF_DRV_ID 0X0A
#define LRADC_DRV_ID 0X0B
#define GPIO_DRV_ID 0X0C
#define TAP_DRV_ID 0X0D
#define AUDIO_DRV_ID 0X18
#define CHARGE_DRV_ID 0X1E
#define CHARGEBOX_DRV_ID 0X1F
#define BATTERY_DRV_ID 0X20
#define NTC_DRV_ID 0x21
#define SYS_DRV_ID 0x4C


// 各个驱动具体的配置项ITEM
/* UART */
#define ITEM_UART_TX_PIN			((UART_DRV_ID<<16) | 1)
#define ITEM_UART_RX_PIN			((UART_DRV_ID<<16) | 2)
#define ITEM_UART_BAUDRATE			((UART_DRV_ID<<16) | 3)
#define ITEM_UART_PRINT_TIME_STAMP	((UART_DRV_ID<<16) | 4)
/* LED */
#define ITEM_LED_LED	((LED_DRV_ID<<16) | 1)
/* ONOFF KEY */
#define ITEM_ONOFF_USE_INNER_ONOFF_KEY	((ONOFF_DRV_ID<<16) | 1)
#define ITEM_ONOFF_CONTINUE_KEY_FUNCTION_AFTER_WAKE_UP	((ONOFF_DRV_ID<<16) | 2)
#define ITEM_ONOFF_KEY_VALUE	((ONOFF_DRV_ID<<16) | 3)
#define ITEM_ONOFF_TIME_PRESS_POWER_ON	((ONOFF_DRV_ID<<16) | 4)
#define ITEM_ONOFF_TIME_LONG_PRESS_RESET	((ONOFF_DRV_ID<<16) | 5)
#define ITEM_ONOFF_BOOT_HOLD_KEY_FUNC	((ONOFF_DRV_ID<<16) | 6)
#define ITEM_ONOFF_BOOT_HOLD_KEY_TIME_MS	((ONOFF_DRV_ID<<16) | 7)
#define ITEM_ONOFF_DEBOUNCE_TIME_MS	((ONOFF_DRV_ID<<16) | 8)
#define ITEM_ONOFF_REBOOT_AFTER_BOOT_HOLD_KEY_CLEAR_PAIRED_LIST	((ONOFF_DRV_ID<<16) | 9)
/* LRADC KEY */
#define ITEM_LRADC_KEY	((LRADC_DRV_ID<<16) | 1)
#define ITEM_LRADC_CTRL	((LRADC_DRV_ID<<16) | 2)
#define ITEM_LRADC_PULL_UP	((LRADC_DRV_ID<<16) | 3)
#define ITEM_LRADC_KEY_WAKE_UP	((LRADC_DRV_ID<<16) | 4)
#define ITEM_LRADC_VALUE_TEST	((LRADC_DRV_ID<<16) | 5)
#define ITEM_LRADC_DEBOUNCE_TIME_MS	((LRADC_DRV_ID<<16) | 6)
/* GPIO KEY */
#define ITEM_GPIO_KEY	((GPIO_DRV_ID<<16) | 1)
/* TAP KEY */
#define ITEM_TAP_KEY_CONTROL	((TAP_DRV_ID<<16) | 1)
/* AUDIO */
#define ITEM_AUDIO_OUT_MODE	((AUDIO_DRV_ID<<16) | 1)
#define ITEM_AUDIO_I2STX_SELECT_GPIO	((AUDIO_DRV_ID<<16) | 2)
#define ITEM_AUDIO_CHANNEL_SELECT_MODE	((AUDIO_DRV_ID<<16) | 3)
#define ITEM_AUDIO_CHANNEL_SELECT_GPIO	((AUDIO_DRV_ID<<16) | 4)
#define ITEM_AUDIO_CHANNEL_SELECT_LRADC	((AUDIO_DRV_ID<<16) | 5)
#define ITEM_AUDIO_TWS_ALONE_AUDIO_CHANNEL	((AUDIO_DRV_ID<<16) | 6)
#define ITEM_AUDIO_L_SPEAKER_OUT	((AUDIO_DRV_ID<<16) | 7)
#define ITEM_AUDIO_R_SPEAKER_OUT	((AUDIO_DRV_ID<<16) | 8)
#define ITEM_AUDIO_ADC_BIAS_SETTING	((AUDIO_DRV_ID<<16) | 9)
#define ITEM_AUDIO_DAC_BIAS_SETTING	((AUDIO_DRV_ID<<16) | 10)
#define ITEM_AUDIO_KEEP_DA_ENABLED_WHEN_PLAY_PAUSE	((AUDIO_DRV_ID<<16) | 11)
#define ITEM_AUDIO_DISABLE_PA_WHEN_RECONNECT	((AUDIO_DRV_ID<<16) | 12)
#define ITEM_AUDIO_EXTERN_PA_CONTROL	((AUDIO_DRV_ID<<16) | 13)
#define ITEM_AUDIO_LARGE_NOISE_OPTIMIZE_ENABLE	((AUDIO_DRV_ID<<16) | 14)
#define ITEM_AUDIO_ANTIPOP_PROCESS_DISABLE	((AUDIO_DRV_ID<<16) | 15)
#define ITEM_AUDIO_DMIC01_CHANNEL_ALIGNING	((AUDIO_DRV_ID<<16) | 16)
#define ITEM_AUDIO_DMIC23_CHANNEL_ALIGNING	((AUDIO_DRV_ID<<16) | 17)
#define ITEM_AUDIO_DMIC_SELECT_GPIO	((AUDIO_DRV_ID<<16) | 18)
#define ITEM_AUDIO_ENABLE_ANC	((AUDIO_DRV_ID<<16) | 19)
#define ITEM_AUDIO_ANCDMIC_SELECT_GPIO	((AUDIO_DRV_ID<<16) | 20)
#define ITEM_AUDIO_RECORD_ADC_SELECT	((AUDIO_DRV_ID<<16) | 21)
#define ITEM_AUDIO_ENABLE_VMIC	((AUDIO_DRV_ID<<16) | 22)
#define ITEM_AUDIO_HW_AEC_SELECT	((AUDIO_DRV_ID<<16) | 23)
#define ITEM_AUDIO_TM_ADC_SELECT	((AUDIO_DRV_ID<<16) | 24)
#define ITEM_AUDIO_MIC_CONFIG	((AUDIO_DRV_ID<<16) | 25)
#define ITEM_AUDIO_ADC_INPUT_SELECT	((AUDIO_DRV_ID<<16) | 26)
#define ITEM_AUDIO_PA_GAIN	((AUDIO_DRV_ID<<16) | 27)
#define ITEM_AUDIO_DUAL_MIC_EXCHANGE_ENABLE	((AUDIO_DRV_ID<<16) | 28)
#define ITEM_AUDIO_LARGE_CURRENT_PROTOTECT_ENABLE	((AUDIO_DRV_ID<<16) | 29)
#define ITEM_AUDIO_ANALOG_GAIN_SETTINGS	((AUDIO_DRV_ID<<16) | 30)

/* CHARGE */
#define ITEM_CHARGE_SELECT_CHARGE_MODE	((CHARGE_DRV_ID<<16) | 1)
#define ITEM_CHARGE_CURRENT	((CHARGE_DRV_ID<<16) | 2)
#define ITEM_CHARGE_VOLTAGE	((CHARGE_DRV_ID<<16) | 3)
#define ITEM_CHARGE_STOP_MODE	((CHARGE_DRV_ID<<16) | 4)
#define ITEM_CHARGE_STOP_VOLTAGE	((CHARGE_DRV_ID<<16) | 5)
#define ITEM_CHARGE_STOP_CURRENT	((CHARGE_DRV_ID<<16) | 6)
#define ITEM_CHARGE_PRECHARGE_STOP_VOLTAGE ((CHARGE_DRV_ID<<16) | 7)
#define ITEM_CHARGE_PRECHARGE_CURRENT ((CHARGE_DRV_ID<<16) | 8)
#define ITEM_CHARGE_PRECHARGE_CURRENT_MIN_LIMIT ((CHARGE_DRV_ID<<16) | 9)
#define ITEM_CHARGE_FAST_CHARGE_ENABLE ((CHARGE_DRV_ID<<16) | 10)
#define ITEM_CHARGE_FAST_CHARGE_CURRENT ((CHARGE_DRV_ID<<16) | 11)
#define ITEM_CHARGE_FAST_CHARGE_VOLTAGE_THRESHOLD ((CHARGE_DRV_ID<<16) | 12)
#define ITEM_CHARGE_ENABLE_BATTERY_RECHARGE ((CHARGE_DRV_ID<<16) | 13)
#define ITEM_CHARGE_BATTERY_RECHARGE_THRESHOLD ((CHARGE_DRV_ID<<16) | 14)
#define ITEM_CHARGE_BATTERY_CHARGE_TOTAL_TIME_LIMIT ((CHARGE_DRV_ID<<16) | 15)
#define ITEM_CHARGE_BATTERY_CHECK_PERIOD_SEC	((CHARGE_DRV_ID<<16) | 16)
#define ITEM_CHARGE_CHECK_PERIOD_SEC	((CHARGE_DRV_ID<<16) | 17)
#define ITEM_CHARGE_FULL_CONTINUE_SEC	((CHARGE_DRV_ID<<16) | 18)
#define ITEM_CHARGE_FRONT_CHARGE_FULL_POWER_OFF_WAIT_SEC	((CHARGE_DRV_ID<<16) | 19)
#define ITEM_CHARGE_DC5V_DETECT_DEBOUNCE_TIME_MS	((CHARGE_DRV_ID<<16) | 20)

/* CHARGER BOX */
#define ITEM_CHARGEBOX_ENABLE_CHARGER_BOX ((CHARGEBOX_DRV_ID<<16) | 1)
#define ITEM_CHARGEBOX_DC5V_PULL_DOWN_CURRENT ((CHARGEBOX_DRV_ID<<16) | 2)
#define ITEM_CHARGEBOX_DC5V_PULL_DOWN_HOLD_MS ((CHARGEBOX_DRV_ID<<16) | 3)
#define ITEM_CHARGEBOX_STANDBY_DELAY_MS ((CHARGEBOX_DRV_ID<<16) | 4)
#define ITEM_CHARGEBOX_STANDBY_VOLTAGE ((CHARGEBOX_DRV_ID<<16) | 5)
#define ITEM_CHARGEBOX_WAKE_DELAY_MS ((CHARGEBOX_DRV_ID<<16) | 6)
#define ITEM_CHARGEBOX_BOX_STANDBY_CURRENT ((CHARGEBOX_DRV_ID<<16) | 7)
#define ITEM_CHARGEBOX_DC5V_UART_COMM_SETTINGS ((CHARGEBOX_DRV_ID<<16) | 8)
#define ITEM_CHARGEBOX_DC5V_IO_COMM_SETTINGS ((CHARGEBOX_DRV_ID<<16) | 9)
/* BATTERY */
#define ITEM_BATTERY_LEVEL ((BATTERY_DRV_ID<<16) | 1)
#define ITEM_BATTERY_TOO_LOW_VOLTAGE ((BATTERY_DRV_ID<<16) | 2)
#define ITEM_BATTERY_LOW_VOLTAGE ((BATTERY_DRV_ID<<16) | 3)
#define ITEM_BATTERY_LOW_VOLTAGE_EX ((BATTERY_DRV_ID<<16) | 4)
#define ITEM_BATTERY_LOW_PROMPT_INTERVAL_SEC ((BATTERY_DRV_ID<<16) | 5)
/* NTC */
#define ITEM_NTC_SETTING ((NTC_DRV_ID<<16) | 1)
#define ITEM_NTC_RANGES ((NTC_DRV_ID<<16) | 2)

/* OTHER - SYS MORE CONFIG */
#define ITEM_SYS_SETTINGS_SUPPORT_FEATURES ((SYS_DRV_ID<<16) | 1)

/*!
 * \brief 读取驱动配置数据
 * \n
 * \param item_key : 各驱动定义的item ID, 见driver_config.h
 * \param data : 保存配置数据
 * \param size : data 大小
 * \return
 *     成功: 数据长度
 * \n  失败: 0
 */
int cfg_get_by_key(uint32_t item_key, void *data, int size);


#endif  // ZEPHYR_INCLUDE_DRIVERS_DRIVER_ITEM_H_

