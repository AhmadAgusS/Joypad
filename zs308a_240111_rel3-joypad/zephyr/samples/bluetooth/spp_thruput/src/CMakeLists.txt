# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(include)

add_subdirectory_ifdef(CONFIG_BT_MAIN_APP libsrc)
add_subdirectory_ifdef(CONFIG_BT_MAIN_APP main)
add_subdirectory_ifdef(CONFIG_BT_MAIN_APP utils)
add_subdirectory_ifdef(CONFIG_BT_BREDR bt_spp)

