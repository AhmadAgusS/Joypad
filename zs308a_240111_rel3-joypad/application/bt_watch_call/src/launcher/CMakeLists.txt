# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(
	.
)

FILE(GLOB applist_src applist/*.c)
FILE(GLOB health_src health/*.c)
FILE(GLOB longlist_src longlist/*.c)
FILE(GLOB power_src power/*.c)
FILE(GLOB stopwatch_src stopwatch/*.c)

target_sources(app PRIVATE
	launcher_app.c
	clock_view.c
	heart_view.c
	main_view.c
	message_view.c
	sport_view.c
	music_view.c
	aod_clock_view.c
	alipay_main_view.c
	wxpay_main_view.c
	${applist_src}
	${health_src}
	${longlist_src}
	${power_src}
	${stopwatch_src}
)

if(CONFIG_LCMUSIC_APP OR CONFIG_BT_MUSIC_APP)
	target_sources(app PRIVATE music_view.c)
endif()

if(CONFIG_ALARM_MANAGER)
	target_sources(app PRIVATE alarm_set_view.c)
endif()

add_subdirectory(clock_selector)
add_subdirectory(alipay)
