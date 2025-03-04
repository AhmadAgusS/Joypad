# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_OTA_FOREGROUND app PRIVATE ota_app_foreground.c)
target_sources_ifdef(CONFIG_OTA_BACKGROUND app PRIVATE ota_app_background.c ota_msgbox_view.c)
target_sources_ifdef(CONFIG_UI_SERVICE app PRIVATE ota_app_view.c)
