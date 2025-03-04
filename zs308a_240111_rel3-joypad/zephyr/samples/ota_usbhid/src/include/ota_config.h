#ifndef __OTA_CONFIG_H__
#define __OTA_CONFIG_H__

#include <input_manager_type.h>

#define BOARD_LARK                          (4)
#define BOARD_CUCKOO                        (5)          
#define BOARD_FPGA                          (8)

#define BOARD_TYPE  BOARD_CUCKOO

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
#define IC_TYPE_CUCKOO  (1 << 15)
#define IC_TYPE_GL6189  (1 << 11)


/* 当前选用的 IC 型号
 */
#define CFG_IC_TYPE  IC_TYPE_CUCKOO


typedef struct  // OTA 设置
{
    cfg_uint8   Enable_Dongle_OTA_Erase_VRAM;   // Dongle OTA擦除用户区, CFG_TYPE_BOOL
    cfg_uint8   Enable_APP_OTA_Erase_VRAM;      // 发射机或APP OTA擦除用户区, CFG_TYPE_BOOL
    cfg_uint8   Enable_Single_OTA_Without_TWS;  // 未组队时允许单耳OTA, CFG_TYPE_BOOL
    cfg_uint8   Enable_Ver_Diff;                // 左右耳固件版本不同时，允许TWS OTA, CFG_TYPE_BOOL
    cfg_uint8   Enable_Ver_Low;                 // 关闭版本控制，版本号自动加1, CFG_TYPE_BOOL
    cfg_uint8	  Enable_Poweroff;                // OTA完成后关机, CFG_TYPE_BOOL
    cfg_uint8   Version_Number[12];             // 固件版本号, 例如 1.6.8, 2.6.3.4

} CFG_Struct_OTA_Settings;

/*-----------------------------------------------------------------------------
 * 注意：
 * 从 CFG_ID_BT_MUSIC_DAE_AL 0x60 开始预留给算法使用，
 * 而 CFG_ID_LEAUDIO_Call_MIC_DAE 0x5F，所以不能再添加新的配置项了，
 * 请使用各种 More_Settings 配置项来添加配置。
 *---------------------------------------------------------------------------*/

#define CFG_ID_USER_VERSION             0x01
#define CFG_ID_PLATFORM_CASE            0x02
#define CFG_ID_CONSOLE_UART             0x03
#define CFG_ID_SYSTEM_SETTINGS          0x04
#define CFG_ID_OTA_SETTINGS             0x05
#define CFG_ID_FACTORY_SETTINGS         0x06
#define CFG_ID_ONOFF_KEY                0x07
#define CFG_ID_LRADC_KEYS               0x08
#define CFG_ID_GPIO_KEYS                0x09
#define CFG_ID_TAP_KEY                  0x0A
#define CFG_ID_KEY_THRESHOLD            0x0B
#define CFG_ID_KEY_FUNC_MAPS            0x0C
#define CFG_ID_COMBO_KEY_FUNC_MAPS      0x0D
#define CFG_ID_CUSTOMED_KEY_SEQUENCE    0x0E
#define CFG_ID_LED_DRIVES               0x0F
#define CFG_ID_LED_DISPLAY_MODELS       0x10
#define CFG_ID_BT_MUSIC_VOLUME_TABLE    0x11
#define CFG_ID_BT_CALL_VOLUME_TABLE     0x12
#define CFG_ID_LINEIN_VOLUME_TABLE      0x13
#define CFG_ID_VOICE_VOLUME_TABLE       0x14
#define CFG_ID_VOLUME_SETTINGS          0x15
#define CFG_ID_AUDIO_SETTINGS           0x16
#define CFG_ID_TONE_LIST                0x17
#define CFG_ID_KEY_TONE                 0x18
#define CFG_ID_VOICE_LIST               0x19
#define CFG_ID_NUMERIC_VOICE_LIST       0x1A
#define CFG_ID_EVENT_NOTIFY             0x1B
#define CFG_ID_BATTERY_CHARGE           0x1C
#define CFG_ID_CHARGER_BOX              0x1D
#define CFG_ID_BATTERY_LEVEL            0x1E
#define CFG_ID_BATTERY_LOW              0x1F
#define CFG_ID_NTC_SETTINGS             0x20
#define CFG_ID_BT_DEVICE                0x21
#define CFG_ID_BT_RF_PARAM_TABLE        0x22
#define CFG_ID_BT_MANAGER               0x23
#define CFG_ID_BT_PAIR                  0x24
#define CFG_ID_TWS_PAIR                 0x25
#define CFG_ID_TWS_ADVANCED_PAIR        0x26
#define CFG_ID_TWS_SYNC                 0x27
#define CFG_ID_BT_AUTO_RECONNECT        0x28
#define CFG_ID_BT_HID_SETTINGS          0x29
#define CFG_ID_LOW_LATENCY_SETTINGS     0x2A
#define CFG_ID_BTMUSIC_MULTI_DAE_SETTINGS  0x2B
#define CFG_ID_BT_MUSIC_VOLUME_SYNC     0x2C
#define CFG_ID_BT_MUSIC_STOP_HOLD       0x2D
#define CFG_ID_BT_TWO_DEVICE_PLAY       0x2E
#define CFG_ID_BT_CALL_VOLUME_SYNC      0x2F
#define CFG_ID_INCOMING_CALL_PROMPT     0x30
#define CFG_ID_CAP_TEMP_COMP            0x31
#define CFG_ID_LINEIN_DETECT            0x32
#define CFG_ID_BT_MUSIC_DAE             0x33
#define CFG_ID_BT_CALL_OUT_DAE          0x34
#define CFG_ID_BT_CALL_MIC_DAE          0x35
#define CFG_ID_LINEIN_OUT_DAE           0x36
#define CFG_ID_BT_CALL_QUALITY          0x37
#define CFG_ID_VOICE_PLAYER_PARAM       0x38
#define CFG_ID_VOICE_USER_SETTINGS      0x39
#define CFG_ID_TONE_PLAYER_PARAM        0x3A
#define CFG_ID_TONE_USER_SETTINGS       0x3B
#define CFG_ID_LINEIN_PLAYER_PARAM      0x3C
#define CFG_ID_LINEIN_USER_SETTINGS     0x3D
#define CFG_ID_BTMUSIC_PLAYER_PARAM     0x3E
#define CFG_ID_BTMUSIC_USER_SETTINGS    0x3F
#define CFG_ID_BTSPEECH_PLAYER_PARAM    0x40
#define CFG_ID_BTSPEECH_USER_SETTINGS   0x41
#define CFG_ID_IGSPEECH_PLAYER_PARAM    0x42
#define CFG_ID_IGSPEECH_USER_SETTINGS   0x43
#define CFG_ID_BLE_MANAGER              0x44
#define CFG_ID_BLE_ADVERTISING_MODE_1   0x45
#define CFG_ID_BLE_ADVERTISING_MODE_2   0x46
#define CFG_ID_BLE_CONNECTION_PARAM     0x47
#define CFG_ID_LE_AUDIO_MANAGER         0x48
#define CFG_ID_LE_AUDIO_ADVERTISING_MODE  0x49
#define CFG_ID_BLE_PASS_THROUGH         0x4A
#define CFG_ID_BT_LINK_QUALITY          0x4B
#define CFG_ID_BT_SCAN_PARAMS           0x4C
#define CFG_ID_APP_MUSIC                0x4D
#define CFG_ID_CARD_SETTINGS            0x4E
#define CFG_ID_USB_SETTINGS             0x4F
#define CFG_ID_USR_RESERVED_DATA        0x50
#define CFG_ID_SYS_RESERVED_DATA        0x51
#define CFG_ID_BT_DEBUG                 0x52
#define CFG_ID_SYSTEM_MORE_SETTINGS     0x53
#define CFG_ID_UPGRADE_MORE_SETTINGS    0x54
#define CFG_ID_DISPLAY_MORE_SETTINGS    0x55
#define CFG_ID_KEY_MORE_SETTINGS        0x56
#define CFG_ID_VOICE_MORE_SETTINGS      0x57
#define CFG_ID_AUDIO_MORE_SETTINGS      0x58
#define CFG_ID_VOLUME_GAIN_MORE_SETTINGS  0x59
#define CFG_ID_BT_MUSIC_MORE_SETTINGS   0x5A
#define CFG_ID_BT_CALL_MORE_SETTINGS    0x5B
#define CFG_ID_TR_BT_DEVICE             0x5C
#define CFG_ID_BLE_MANAGER_MORE_SETTINGS  0x5D
#define CFG_ID_LEAUDIO_CALL_OUT_DAE     0x5E
#define CFG_ID_LEAUDIO_CALL_MIC_DAE     0x5F

#endif