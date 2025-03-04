# SPDX-License-Identifier: Apache-2.0
zephyr_include_directories(.)

target_sources_ifdef(CONFIG_OTA_FOREGROUND app PRIVATE ota_app_foreground.c)
