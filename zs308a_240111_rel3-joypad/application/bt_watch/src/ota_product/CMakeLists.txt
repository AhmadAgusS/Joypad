# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_OTA_PRODUCT_SUPPORT app PRIVATE ota_product_app.c)
target_sources_ifdef(CONFIG_OTA_PRODUCT_SUPPORT app PRIVATE ota_product_key.c)
