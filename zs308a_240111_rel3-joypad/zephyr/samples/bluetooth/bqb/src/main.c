
#include <zephyr.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <drivers/bluetooth/bt_drv.h>

static void bt_manager_set_bt_drv_param(void)
{
	btdrv_init_param_t param;

	memset(&param, 0, sizeof(param));
	param.set_hosc_cap = 1;
	param.hosc_capacity = 0x64;			/* Wait todo: get for config or nvram */
	param.set_max_rf_power = 1;
	param.bt_max_rf_tx_power = 38;		/* 8db */
	//param.set_ble_rf_power = 1;
	//param.ble_rf_tx_power = 34;

	btdrv_set_init_param(&param);
}

int main(void)
{
    int ret;

	/* init bt param */
	bt_manager_set_bt_drv_param();

	/* BQB mode [0:BR BQB Test, 1:BLE BQB Test, 2:BR/BLE dual BQB Test] */
	extern int bqb_init(int bqb_mode);
	ret = bqb_init(2);
	if (ret) {
		printk("[BQB] enter bqb mode failed (err %d)\n", ret);
	} else {
		printk("[BQB] enter bqb mode!\n");
	}

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}
