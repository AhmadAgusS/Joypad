#include "i2c_test.h"
#include <drivers/i2c.h>
#define LOG_LEVEL CONFIG_I2C_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(i2c_test_master);

static const struct device *i2c_master_dev;
static u8_t read_buf[64] = {0};
static int read_fault_cnt;
const static uint8_t expected_rcv_data[64] = {
        0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
        0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
        0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
        0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0
    };

static int64_t cycle_start_time; // Get the start time of the cycle

static void i2c_test_read_sync(void)
{
    int i = 0;
    u8_t test_buf[64] = {0};

    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);

    // i2c_configure(i2c_master_dev, 0x12);   // 0x12: I2C_SPEED_STANDARD - 100khz
    i2c_configure(i2c_master_dev, 0x1A);   // 0x1A: I2C_SPEED_ULTRA - 1.5Mhz

    memset(test_buf, 0x0, sizeof(test_buf));
    
    int ret = i2c_read(i2c_master_dev, test_buf, sizeof(test_buf), CONFIG_TEST_I2C_SLAVE_ADDR);
    
    printk("%s, i2c_read ret=%d, data received:\n", __func__, ret);
    for (i = 0; i < sizeof(test_buf); i++) {
        if(i % 16 == 0 && i != 0)
            printk("\n");
        printk("%02x ", test_buf[i]);
    }

    return;
}

static void i2c_test_write_sync(void)
{
    u8_t test_buf[64] = {0};
    printk("%s, buf=%p\n", __func__, test_buf);
    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);

    // i2c_configure(i2c_master_dev, 0x12);   // 0x12: I2C_SPEED_STANDARD - 100khz
    i2c_configure(i2c_master_dev, 0x1A);   // 0x1A: I2C_SPEED_ULTRA - 100khz

    for (int i = 0; i < sizeof(test_buf); i++) {
        test_buf[i] = i;
    }
    
    int ret = i2c_write(i2c_master_dev, test_buf, sizeof(test_buf), CONFIG_TEST_I2C_SLAVE_ADDR);
    printk("%s, ret=%d\n", __func__, ret);
    
    return;
}


int i2c_write_async_func_t(void *cb_data, struct i2c_msg *msgs, uint8_t num_msgs, bool is_err)
{    
    printk("write_async success, time spent:%lld ms\n", k_uptime_get() - cycle_start_time);
    return 0;
}

int i2c_read_async_func_t(void *cb_data, struct i2c_msg *msgs, uint8_t num_msgs, bool is_err)
{
    printk("%s L%d: len=%d:\n", __FUNCTION__, __LINE__, msgs->len);
    for (int i = 0; i < msgs->len; i++) {
        if (i % 16 == 0 && i != 0) {
            printk("\n");
        }
        printk("%02x ", msgs->buf[i]);
        
    }    
    printk("\nread_async success, time spent:%lld ms\n", k_uptime_get() - cycle_start_time);
    return 0;
}

int i2c_read_async_cmp_func_t(void *cb_data, struct i2c_msg *msgs, uint8_t num_msgs, bool is_err)
{
    if (memcmp(msgs->buf, expected_rcv_data, 64) != 0) {
        read_fault_cnt += 1;
    }

    return 0;
}

static void i2c_test_write_async(void)
{
    u8_t test_buf[64] = {0};
    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    
    // i2c_configure(i2c_master_dev, 0x12);   // 0x12: I2C_SPEED_STANDARD - 100khz
    // i2c_configure(i2c_master_dev, 0x1A);   // 0x1A: I2C_SPEED_ULTRA - 100khz
    i2c_configure(i2c_master_dev, I2C_MODE_MASTER | I2C_SPEED_SET(I2C_SPEED_FAST));   // 0x1A: I2C_SPEED_ULTRA - 400khz

    // it's better to declare test_buf as global array; otherwise,
    // test_buf will be released and will cause data corruption if user doesn't enable CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
    for (int i = 0; i < sizeof(test_buf); i++) {
        test_buf[i] = i;
    }

    cycle_start_time = k_uptime_get();
    int ret = i2c_write_async(i2c_master_dev, test_buf, sizeof(test_buf), CONFIG_TEST_I2C_SLAVE_ADDR, i2c_write_async_func_t, NULL);
    LOG_DBG("%s: %s i2c_write_async ret= %d\n", i2c_master_dev->name, __func__, ret);
    return;
}

static void i2c_test_read_async(void)
{
    
    void *cb_data = NULL;
    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);

    // i2c_configure(i2c_master_dev, 0x12);   // 0x12: I2C_SPEED_STANDARD - 100khz
    // i2c_configure(i2c_master_dev, 0x1A);   // 0x1A: I2C_SPEED_ULTRA - 100khz
    i2c_configure(i2c_master_dev, I2C_MODE_MASTER | I2C_SPEED_SET(I2C_SPEED_FAST));   // 0x1A: I2C_SPEED_ULTRA - 400khz

    cycle_start_time = k_uptime_get();
    int ret = i2c_read_async(i2c_master_dev, read_buf, sizeof(read_buf), CONFIG_TEST_I2C_SLAVE_ADDR, i2c_read_async_func_t, cb_data);
    LOG_DBG("%s: %s i2c_read_async ret= %d\n", i2c_master_dev->name, __func__, ret);
    return;
}

static int test_i2c_main(const struct device *arg)
{
    // acts_pinmux_set(I2C_SCL, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    // acts_pinmux_set(I2C_SDA, 0x4 | 1 << 5 | ((4 / 2) << 12) | 1 << 11);
    i2c_master_dev = device_get_binding(CONFIG_TEST_MASTER_AT_I2C_DEV);
     
    if (!i2c_master_dev) {
        LOG_ERR("Fail to bind dev:%s as i2c master", CONFIG_TEST_MASTER_AT_I2C_DEV);
    } else {
        LOG_INF("Successfully bound dev:%s as i2c master", i2c_master_dev->name);
    }
    return 0;
}

#include <sys/printk.h>
#include <sys/util.h>

#define CYCLE_TIME_MS  4   // Total cycle time: 4ms
#define READ_PHASE_MS  2   // First 2ms for reading
#define WRITE_PHASE_MS 2   // Next 2ms for writing

static void i2c_start_async_rw_loop(void)
{
    // setup data in buffer
    u8_t test_buf[64] = {0};
    int round = 1;
    
    int ret = 0;

    i2c_configure(i2c_master_dev, I2C_MODE_MASTER | I2C_SPEED_SET(I2C_SPEED_FAST));   // 0x1A: I2C_SPEED_ULTRA - 400khz


    // it's better to declare test_buf as global array; otherwise,
    // test_buf will be released and will cause data corruption if user doesn't enable CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
    for (int i = 0; i < sizeof(test_buf); i++) {
        test_buf[i] = i;
    }

    while (1) {
        cycle_start_time = k_uptime_get(); // Get the start time of the cycle
        
        // ======== Read Phase (2ms) ========
        ret = i2c_read_async(i2c_master_dev, read_buf, sizeof(read_buf), CONFIG_TEST_I2C_SLAVE_ADDR, i2c_read_async_cmp_func_t, NULL);
        
        while (k_uptime_get() - cycle_start_time < READ_PHASE_MS) {
            k_sleep(K_USEC(10));  // sleep 10us to eliminate burden of system
        }

        // ======== Write Phase (second 2ms) ========
        ret = i2c_write_async(i2c_master_dev, test_buf, sizeof(test_buf), CONFIG_TEST_I2C_SLAVE_ADDR, NULL, NULL);        // printk("[WRITE] ret = %d\n", ret);
        
        while (k_uptime_get() - cycle_start_time < CYCLE_TIME_MS) {
            k_sleep(K_USEC(10));  // sleep 10us to eliminate burden of system
        }
        
        if (round % 1000 == 0) {
            printk("round: #%d / err(read_async):%d\n", round, read_fault_cnt);
            read_fault_cnt = 0;
        }
        round++;
    }
}

static int i2c_start_async_rw_loop_test(const struct shell *shell,
        size_t argc, char **argv)
{
    i2c_start_async_rw_loop();
    return 0;
}

static int i2c_start_read_sync_test(const struct shell *shell,
        size_t argc, char **argv)
{
    i2c_test_read_sync();
    return 0;
}

static int i2c_start_write_sync_test(const struct shell *shell,
        size_t argc, char **argv)
{
    i2c_test_write_sync();
    return 0;
}

static int i2c_start_write_async_test(const struct shell *shell,
        size_t argc, char **argv)
{
    i2c_test_write_async();
    return 0;
}

static int i2c_start_read_async_test(const struct shell *shell,
        size_t argc, char **argv)
{
    i2c_test_read_async();
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(test_i2c_cmds,
        SHELL_CMD(start_read_sync, NULL, "start test", i2c_start_read_sync_test),
        SHELL_CMD(start_write_sync, NULL, "start test", i2c_start_write_sync_test),
        SHELL_CMD(start_read_async, NULL, "start test", i2c_start_read_async_test),
        SHELL_CMD(start_write_async, NULL, "start test", i2c_start_write_async_test),
        SHELL_CMD(start_async_read_write_loop, NULL, "start async read write loop test until you stop it", i2c_start_async_rw_loop_test),
        SHELL_SUBCMD_SET_END     /* Array terminated. */
        );

SHELL_CMD_REGISTER(test_i2c_master, &test_i2c_cmds, "I2C Master commands", NULL);
SYS_INIT(test_i2c_main, APPLICATION, 10);
