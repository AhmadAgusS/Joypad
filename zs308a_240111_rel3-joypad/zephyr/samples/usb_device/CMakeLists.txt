# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(usb_device)

add_subdirectory(src)

target_sources(app PRIVATE src/system_app_main.c)
