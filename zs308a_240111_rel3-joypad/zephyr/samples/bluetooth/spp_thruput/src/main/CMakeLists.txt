# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

target_sources(app PRIVATE system_app_main.c)
target_sources(app PRIVATE rmc_timer.c)
