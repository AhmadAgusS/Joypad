subdir-ccflags-y += -I${ZEPHYR_BASE}/samples/bt_box/src/include
subdir-ccflags-y += -I${ZEPHYR_BASE}/samples/bt_box/src/tool/
subdir-ccflags-y += -I${ZEPHYR_BASE}/samples/bt_box/src/btmusic/
subdir-ccflags-y += -I${ZEPHYR_BASE}/samples/bt_box/src/btcall/
subdir-ccflags-y += -I${ZEPHYR_BASE}/samples/bt_box/src/ota/

ifdef CONFIG_OTA_RECOVERY_APP
obj-y += ota_recovery/
else
obj-y += main/

obj-$(CONFIG_BT_MUSIC_APP) += btmusic/
obj-$(CONFIG_BT_CALL_APP) += btcall/
obj-$(CONFIG_OTA_APP) += ota/
obj-$(CONFIG_TOOL) += tool/
obj-$(CONFIG_ACTIONS_ATT) +=att/
obj-$(CONFIG_CHARGER_APP) += charger/

obj-$(CONFIG_NSM_APP) += bt_nsm/
endif
