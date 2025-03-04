# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_ACTS_BATTERY_SUPPLY_INTERNAL         battery_internal)
add_subdirectory_ifdef(CONFIG_ACTS_BATTERY_SUPPLY_EXTERNAL         battery_external)
