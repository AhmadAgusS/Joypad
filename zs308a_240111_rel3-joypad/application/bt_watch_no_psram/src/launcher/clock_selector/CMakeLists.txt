# SPDX-License-Identifier: Apache-2.0

if(CONFIG_CLOCK_SELECTOR_USE_VIEW_SLIDING)
	set(app_src clock_selector.c clock_selector_subview.c)
else()
	set(app_src clock_selector.c clock_selector_view.c)
endif()

target_sources(app PRIVATE ${app_src})
