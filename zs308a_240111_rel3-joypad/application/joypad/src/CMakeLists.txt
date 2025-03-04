# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(
	.
	include
)

add_subdirectory(main)
add_subdirectory(utils)
add_subdirectory(bt_ble_vnd_gatt)
add_subdirectory(bt_hid)
add_subdirectory(bt_spp)
add_subdirectory(tool)
add_subdirectory(libsrc)
add_subdirectory(usb_hid)
