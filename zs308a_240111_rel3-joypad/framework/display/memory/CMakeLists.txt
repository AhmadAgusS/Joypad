# Copyright (c) 2020 Actions Semiconductor Co., Ltd
#
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_library_sources_ifdef(CONFIG_UI_MEMORY_MANAGER ui_mem.c ui_mem_fb.c ui_mem_gui.c ui_mem_res.c)
zephyr_library_sources_ifdef(CONFIG_RES_MANAGER res_mempool.c)
zephyr_library_sources_ifdef(CONFIG_BITMAP_FONT font_mempool.c)
