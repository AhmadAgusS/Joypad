#
# Copyright (c) 2020 Actions Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Actions
#

zephyr_sources(sw_math.c sw_blend.c sw_rotate.c)
zephyr_sources_ifdef(CONFIG_UI_DMA_MEMSETCPY ui_memsetcpy.c)

add_subdirectory_ifdef(CONFIG_DISPLAY_COMPOSER composer)
