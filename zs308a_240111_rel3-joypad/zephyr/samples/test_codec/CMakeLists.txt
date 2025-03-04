# SPDX-License-Identifier: Apache-2.0

#list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/framework)


cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_codec)

target_sources(app PRIVATE src/system_app_main.c src/test_codec.c src/system_audio_policy.c src/card_reader.c)
