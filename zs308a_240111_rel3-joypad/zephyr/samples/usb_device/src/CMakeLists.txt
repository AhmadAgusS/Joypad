# SPDX-License-Identifier: Apache-2.0

# USB Device
if(CONFIG_USB_DEVICE)
	zephyr_include_directories(usb_cdc_acm usb_hid)
	add_subdirectory_ifdef(CONFIG_USB_CDC_ACM usb_cdc_acm)
	add_subdirectory_ifdef(CONFIG_USB_DEVICE_HID usb_hid)
endif()

