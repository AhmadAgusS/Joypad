
#ifndef __CONFIG_H__
#define __CONFIG_H__

/*!
 * \file
 * \brief     配置文件
 * \details
 * \author
 * \date
 * \copyright Actions
 */


#define BOARD_LARK                          (4)

#define CFG_SUPPORT_AAP_SETTINGS     1
#define CFG_OPTIMIZE_BT_MUSIC_STUCK  0

#define BOARD_TYPE  BOARD_LARK

/*-----------------------------------------------------------------------------
 * 配置文件中必须使用以下定义的 cfg_xxx 数据类型
 *---------------------------------------------------------------------------*/
typedef signed char   cfg_int8;
typedef signed short  cfg_int16;
typedef signed int  cfg_int32;

typedef unsigned char   cfg_uint8;
typedef unsigned short  cfg_uint16;
typedef unsigned int  cfg_uint32;

/*---------------------------------------------------------------------------*/


/* 常用数值定义
 */
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif 

#define ENABLE   TRUE
#define DISABLE  FALSE
#define OK       TRUE
#define FAIL     FALSE
#define NONE     0


/* IC 型号定义
 */
#define IC_TYPE_LARK    (1 << 14)


/* 当前选用的 IC 型号
 */
#if (BOARD_TYPE == BOARD_LARK)
    #define CFG_IC_TYPE  IC_TYPE_LARK
#endif

/* 最大配置数定义
 */
#define CFG_MAX_USER_VERSION_LEN    32
#define CFG_MAX_CASE_NAME_LEN       20
#define CFG_MAX_RESERVED_SIZE       255
#define CFG_MAX_GPIO_PINS           79
#define CFG_MAX_LRADC_KEYS          9
#define CFG_MAX_LRADC_COMBO_KEYS    3
#define CFG_MAX_GPIO_KEYS           4
#define CFG_MAX_KEY_FUNC_MAPS       40
#define CFG_MAX_COMBO_KEY_MAPS      8
#define CFG_MAX_LEDS                4
#define CFG_MAX_LED_DISPLAY_MODELS  15
#define CFG_MAX_VOICES              24
#define CFG_MAX_NUMERIC_VOICES      10
#define CFG_MAX_TONES               10
#define CFG_MAX_VOICE_NAME_LEN      9
#define CFG_MAX_VOICE_FMT_LEN       5
#define CFG_MAX_TONE_NAME_LEN       9
#define CFG_MAX_TONE_FMT_LEN        5
#define CFG_MAX_EVENT_NOTIFY        40
#define CFG_MAX_BATTERY_LEVEL       10
#define CFG_MAX_BT_DEV_NAME_LEN     30
#define CFG_MAX_BT_SUFFIX_LEN       10
#define CFG_MAX_BT_PIN_CODE_LEN     6
#define CFG_MAX_BT_SUPPORT_DEVICES  3
#define CFG_MAX_BT_MUSIC_VOLUME     16
#define CFG_MAX_BT_CALL_VOLUME      15
#define CFG_MAX_LINEIN_VOLUME       16
#define CFG_MAX_VOICE_VOLUME        16
#define CFG_MAX_CAP_TEMP_COMP       20
#define CFG_MAX_PEQ_BANDS           14
#define CFG_MAX_UUID_STR_LEN        38
#define CFG_MAX_ADC_NUM             4

#define CFG_GPIO_MFP_PIN_DEF(_gpio_no, _mfp_sel)  \
    (_gpio_no << 0) |  \
    (_mfp_sel << 8)

/*-----------------------------------------------------------------------------
 * 配置枚举类型定义
 * 类型必须以 CFG_XXX 命名
 *---------------------------------------------------------------------------*/


/* 配置分类定义
 */




/*-----------------------------------------------------------------------------
 * 配置数据类定义
 * 类型必须以 CFG_XXX 命名
 * 类成员必须赋值
 *---------------------------------------------------------------------------*/




#endif  // __CONFIG_H__


