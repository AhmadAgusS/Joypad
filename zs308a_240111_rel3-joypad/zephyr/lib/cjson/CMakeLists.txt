# SPDX-License-Identifier: Apache-2.0

if(CONFIG_CJSON_LIB)

zephyr_include_directories(.)

zephyr_sources(
	cJSON.c
)

zephyr_sources_ifdef(CONFIG_MINIMAL_LIBC
	libc_sscanf.c
	libm_fabs.c
)

endif()