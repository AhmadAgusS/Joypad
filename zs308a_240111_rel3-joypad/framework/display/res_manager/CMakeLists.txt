# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SIMPLE_RES_MANAGER)
  zephyr_library_sources(simple_res_manager.c)
else()
  zephyr_library_sources(res_manager_api.c)
endif()
