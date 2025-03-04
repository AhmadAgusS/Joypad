# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(${ZEPHYR_BASE}/drivers)
zephyr_sources(
  soc.c
  soc_pm.c
  soc_reset.c
  soc_clock.c
  soc_pinmux.c
  soc_pmu.c
  spicache.c
  soc_boot.c
  soc_se_trng.c
  soc_memctrl.c
  soc_ppi.c
  soc_pstore.c
  soc_psram.c
  soc_image_head.c
)
zephyr_sources_ifdef(CONFIG_SYS_POWER_MANAGEMENT power.c)
zephyr_sources_ifdef(CONFIG_PM_POLICY_APP policy_sleep.c)
zephyr_sources_ifdef(CONFIG_SOC_SPICACHE_PROFILE_STAT cacheprofile_stat.c)
zephyr_sources_ifdef(CONFIG_SOC_EP_MODE soc_sleep_ep.c)
zephyr_sources_ifndef(CONFIG_SOC_EP_MODE soc_sleep.c)

zephyr_library_import(libbt_test ${CMAKE_CURRENT_LIST_DIR}/libbt_test.a)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/socatp/CMakeLists.txt)
    add_subdirectory(socatp)
else()
    zephyr_library_import(libsocatp ${CMAKE_CURRENT_LIST_DIR}/libsocatp.a)
endif()