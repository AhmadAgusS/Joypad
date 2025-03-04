#include "i2c_test.h"

#define LOG_LEVEL CONFIG_I2C_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(i2c_test_slave);

static const struct device *i2c_slave_dev;
int i2c_slave_write_requested_cb(struct i2c_slave_config *config)
{
	LOG_INF("%s: go into write_requested", i2c_slave_dev->name);

	return 0;
}

int i2c_slave_read_requested_cb(struct i2c_slave_config *config, uint8_t *val)
{
	LOG_INF("%s: go into read_requested", i2c_slave_dev->name);
	*val = 0x86;
	return 0;
}

int i2c_slave_write_received_cb(struct i2c_slave_config *config, uint8_t val)
{
	LOG_INF("%s: write received: val:0x%x", i2c_slave_dev->name, val);
	return 0;
}

int i2c_slave_read_processed_cb(struct i2c_slave_config *config, uint8_t *val)
{
	*val = 0xae;
    LOG_INF("%s: go into read_processed: val=[0x%02x]", i2c_slave_dev->name, *val);
	return 0;
}

int i2c_slave_stop_cb(struct i2c_slave_config *config)
{
	LOG_INF("%s: go into stop", i2c_slave_dev->name);
	return 0;
}

struct i2c_slave_callbacks slave_callbacks = {
    .write_requested = i2c_slave_write_requested_cb,
    .read_requested = i2c_slave_read_requested_cb,
    .write_received = i2c_slave_write_received_cb,
    .read_processed = i2c_slave_read_processed_cb,
    .stop = i2c_slave_stop_cb,
};

struct i2c_slave_config slave_config = {
    .address = CONFIG_TEST_I2C_SLAVE_ADDR,
    .callbacks = &slave_callbacks,
};

static int test_i2c_main(const struct device *arg)
{
    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    // acts_pinmux_set(I2C_SDA, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    i2c_slave_dev = device_get_binding(CONFIG_TEST_SLAVE_AT_I2C_DEV);
    if (!i2c_slave_dev) {
        LOG_ERR("Fail to find device %s\n", CONFIG_TEST_SLAVE_AT_I2C_DEV);
    } else {
        LOG_INF("Successfully bound dev: %s as slave", i2c_slave_dev->name);
    }
    return 1;    
}


static int i2c_start_test(const struct shell *shell,
        size_t argc, char **argv)
{
    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    // acts_pinmux_set(I2C_SDA, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    int ret = i2c_slave_register(i2c_slave_dev, &slave_config);
    LOG_INF("%s, i2c_slave_register ret = %d", __func__, ret);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(test_i2c_cmds,
        SHELL_CMD(start, NULL, "start test", i2c_start_test),
        SHELL_SUBCMD_SET_END     /* Array terminated. */
        );

SHELL_CMD_REGISTER(test_i2c_slave, &test_i2c_cmds, "I2C Slave commands", NULL);
SYS_INIT(test_i2c_main, APPLICATION, 10);
