#ifndef __HID_REPORT_DESC_H__
#define __HID_REPORT_DESC_H__

/* keyboard */
#define REPORT_ID_KBD		0x01
/* Media Volume Ctrl */
#define REPORT_ID_MEDIA		0x02

static const u8_t hid_report_desc[] = {
	 0x05, 0x01, /* USAGE_PAGE (Generic Desktop) */
	 0x09, 0x06, /* USAGE (Keyboard) */
	 0xa1, 0x01, /* COLLECTION (Application) */
	 0x85, REPORT_ID_KBD, /* Report ID (1) */
	 0x05, 0x07, /* USAGE_PAGE (Keyboard/Keypad) */
	 0x19, 0xe0, /* USAGE_MINIMUM (Keyboard LeftControl) */
	 0x29, 0xe7, /* USAGE_MAXIMUM (Keyboard Right GUI) */
	 0x15, 0x00, /* LOGICAL_MINIMUM (0) */
	 0x25, 0x01, /* LOGICAL_MAXIMUM (1) */
	 0x95, 0x08, /* REPORT_COUNT (8) */
	 0x75, 0x01, /* REPORT_SIZE (1) */
	 0x81, 0x02, /* INPUT (Data,Var,Abs) */
	 0x95, 0x01, /* REPORT_COUNT (1) */
	 0x75, 0x08, /* REPORT_SIZE (8) */
	 0x81, 0x03, /* INPUT (Cnst,Var,Abs) */
	 0x95, 0x06, /* REPORT_COUNT (6) */
	 0x75, 0x08, /* REPORT_SIZE (8) */
	 0x15, 0x00, /* LOGICAL_MINIMUM (0) */
	 0x25, 0xFF, /* LOGICAL_MAXIMUM (255) */
	 0x05, 0x07, /* USAGE_PAGE (Keyboard/Keypad) */
	 0x19, 0x00, /* USAGE_MINIMUM (Reserved (no event indicated)) */
	 0x29, 0x65, /* USAGE_MAXIMUM (Keyboard Application) */
	 0x81, 0x00, /* INPUT (Data,Ary,Abs) */
	 0x25, 0x01, /* LOGICAL_MAXIMUM (1) */
	 0x95, 0x05, /* REPORT_COUNT (5) */
	 0x75, 0x01, /* REPORT_SIZE (1) */
	 0x05, 0x08, /* USAGE_PAGE (LEDs) */
	 0x19, 0x01, /* USAGE_MINIMUM (Num Lock) */
	 0x29, 0x05, /* USAGE_MAXIMUM (Kana) */
	 0x91, 0x02, /* OUTPUT (Data,Var,Abs) */
	 0x95, 0x01, /* REPORT_COUNT (1) */
	 0x75, 0x03, /* REPORT_SIZE (3) */
	 0x91, 0x03, /* OUTPUT (Cnst,Var,Abs) */
	 0xc0,	     /* END_COLLECTION */


	/*
	 * Report ID: 2
	 * USB Audio Volume Control
	 */
	0x05, 0x0c, /* USAGE_PAGE (Consumer) */
	0x09, 0x01, /* USAGE (Consumer Control) */
	0xa1, 0x01, /* COLLECTION (Application) */
	0x85, REPORT_ID_MEDIA, /* Report ID*/
	0x15, 0x00, /* Logical Minimum (0x00) */
	0x25, 0x01, /* Logical Maximum (0x01) */
	0x09, 0xe9, /* USAGE (Volume Up) */
	0x09, 0xea, /* USAGE (Volume Down) */
	0x09, 0xe2, /* USAGE (Mute) */
	0x09, 0xcd, /* USAGE (Play/Pause) */
	0x09, 0xb5, /* USAGE (Scan Next Track) */
	0x09, 0xb6, /* USAGE (Scan Previous Track) */
	0x09, 0xb3, /* USAGE (Fast Forward) */
	0x09, 0xb7, /* USAGE (Stop) */
	0x75, 0x01, /* Report Size (0x01) */
	0x95, 0x08, /* Report Count (0x08) */
	0x81, 0x42, /* Input() */
	0xc0,	/* END_COLLECTION */
};

/*
 * according to hid report descriptor
 */
#define HID_SIZE_KEYBOARD	9
#define Num_Lock_code		0x53
#define Caps_Lock_code		0x39

#define HID_SIEZE_MEDIA		2
#define PLAYING_NEXTSONG    	0x10
#define PLAYING_PREVSONG    	0x20
#define PLAYING_VOLUME_INC  	0x01
#define PLAYING_VOLUME_DEC  	0x02
#define PLAYING_AND_PAUSE   	0x08
#endif /* __HID_REPORT_DESC_H__ */
