# Copyright (c) 2020 Actions Semiconductor Co., Ltd
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library_sources_ifdef(CONFIG_LVGL lvgl_memory.c)
zephyr_library_sources_ifdef(CONFIG_LVGL_USE_RES_MANAGER lvgl_res_loader.c lvgl_img_loader.c)
