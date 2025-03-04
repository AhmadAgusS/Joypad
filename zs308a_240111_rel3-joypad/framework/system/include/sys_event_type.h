/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file system event interface
 */

#ifndef _SYS_EVENT_TYPE_H_
#define _SYS_EVENT_TYPE_H_

/**
 * @defgroup sys_event_apis App system event Manager APIs
 * @ingroup system_apis
 * @{
 */

/** sys event type */
enum sys_event_e
{
    SYS_EVENT_NONE = 0,

    SYS_EVENT_POWER_ON,
    SYS_EVENT_POWER_OFF,
    SYS_EVENT_STANDBY,
    SYS_EVENT_WAKE_UP,

    SYS_EVENT_BATTERY_LOW,
	SYS_EVENT_BATTERY_LOW_EX,
    SYS_EVENT_BATTERY_TOO_LOW, 
    SYS_EVENT_REPEAT_BAT_LOW,

    SYS_EVENT_CHARGE_START,
    SYS_EVENT_CHARGE_FULL,
	SYS_EVENT_FRONT_CHARGE_POWON,

    SYS_EVENT_ENTER_PAIR_MODE,
    SYS_EVENT_CLEAR_PAIRED_LIST,
	SYS_EVENT_FACTORY_DEFAULT,

    SYS_EVENT_BT_WAIT_PAIR,
    SYS_EVENT_BT_CONNECTED,
    SYS_EVENT_2ND_CONNECTED,
    SYS_EVENT_BT_DISCONNECTED,
    SYS_EVENT_BT_UNLINKED,

    SYS_EVENT_TWS_START_PAIR,
    SYS_EVENT_TWS_CONNECTED,
    SYS_EVENT_TWS_DISCONNECTED,
	SYS_EVENT_TWS_PAIR_FAILED,

    SYS_EVENT_MIN_VOLUME,
    SYS_EVENT_MAX_VOLUME, 
    SYS_EVENT_PLAY_START,
    SYS_EVENT_PLAY_PAUSE,
	SYS_EVENT_PLAY_PREVIOUS,
	SYS_EVENT_PLAY_NEXT,

    SYS_EVENT_BT_START_CALL,
    SYS_EVENT_BT_CALL_OUTGOING,
    SYS_EVENT_BT_CALL_INCOMING,
    SYS_EVENT_BT_CALL_3WAYIN,
    SYS_EVENT_BT_CALL_REJECT, 
    SYS_EVENT_BT_CALL_ONGOING,
    SYS_EVENT_BT_CALL_END,
    SYS_EVENT_SWITCH_CALL_OUT,
    SYS_EVENT_MIC_MUTE_ON,
    SYS_EVENT_MIC_MUTE_OFF,

    SYS_EVENT_SIRI_START,
    SYS_EVENT_SIRI_STOP,

	SYS_EVENT_HID_PHOTO_SHOT,

	SYS_EVENT_ENTER_BTMUSIC,
    SYS_EVENT_ENTER_LINEIN,
    SYS_EVENT_ENTER_USOUND,
    SYS_EVENT_ENTER_SPDIF_IN,
	SYS_EVENT_ENTER_I2SRX_IN,
    SYS_EVENT_ENTER_SDMPLAYER,
    SYS_EVENT_ENTER_UMPLAYER,
    SYS_EVENT_ENTER_NORMPLAYER,
    SYS_EVENT_ENTER_SDPLAYBACK,
    SYS_EVENT_ENTER_UPLAYBACK,
    SYS_EVENT_ENTER_RECORD,
    SYS_EVENT_ENTER_MIC_IN,
	SYS_EVENT_ENTER_FM,

	SYS_EVENT_LINEIN_PLAY,
	SYS_EVENT_LINEIN_PAUSE,

    SYS_EVENT_SEL_VOICE_LANG_1,
    SYS_EVENT_SEL_VOICE_LANG_2,

    SYS_EVENT_ENTER_BQB_TEST_MODE,

    SYS_EVENT_CUSTOMED_1,
    SYS_EVENT_CUSTOMED_2,
    SYS_EVENT_CUSTOMED_3,
    SYS_EVENT_CUSTOMED_4,
    SYS_EVENT_CUSTOMED_5,
    SYS_EVENT_CUSTOMED_6,
    SYS_EVENT_CUSTOMED_7,
    SYS_EVENT_CUSTOMED_8,
    SYS_EVENT_CUSTOMED_9,

    SYS_EVENT_LOW_LATENCY_MODE,
    SYS_EVENT_NORMAL_LATENCY_MODE,
    SYS_EVENT_PRIVMA_TALK_START,
    SYS_EVENT_DC5V_CMD_COMPLETE,
    SYS_EVENT_NMA_COLLECTION,
    
    SYS_EVENT_BT_MUSIC_DAE_SWITCH,
    SYS_EVENT_DAE_DEFAULT,
    SYS_EVENT_DAE_CUSTOM1,
    SYS_EVENT_DAE_CUSTOM2,
    SYS_EVENT_DAE_CUSTOM3,
    SYS_EVENT_DAE_CUSTOM4,
    SYS_EVENT_DAE_CUSTOM5,
    SYS_EVENT_DAE_CUSTOM6,
    SYS_EVENT_DAE_CUSTOM7,
    SYS_EVENT_DAE_CUSTOM8,
    SYS_EVENT_DAE_CUSTOM9,

    SYS_EVENT_TRANSPARENCY_MODE_ENTER, 
    SYS_EVENT_TRANSPARENCY_MODE_EXIT,
    SYS_EVENT_RING_HANDSET, 

	/* application defined sys event */
	SYS_EVENT_APP_DEFINE_START           = 128,

	
};


/**
 * @} end defgroup sys_event_apis
 */
#endif

