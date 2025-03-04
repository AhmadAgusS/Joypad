# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(
	.
	view_glue
)

file(GLOB_RECURSE SOURCES *.c)

#FILE(GLOB view_glue_src view_glue/*.c)

target_sources(app PRIVATE ${SOURCES}
#	launcher_app.c
#	${view_glue_src}
)
