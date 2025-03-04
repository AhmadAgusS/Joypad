#include "uart_test.h"

static const struct device *uart_dev;
// static struct uart_config uart_cfg;

static int test_uart_main(const struct device *arg)
{
    uart_dev = device_get_binding(UART_PORT);
    // printk("%s %s: GPIO53:TX 0x%x\n", __FUNCTION__, uart_dev->name, sys_read32((0x40068000 + 0xD8)));
    // printk("%s %s: GPIO54:RX 0x%x\n", __FUNCTION__, uart_dev->name, sys_read32((0x40068000 + 0xDC)));
    
    if (!uart_dev) {
        SYS_LOG_ERR("Get %s failed\n", UART_PORT);
        return 1;
    } else {
        return 0;
    }    
}

static int uart_start_test_write(const struct shell *shell,
        size_t argc, char **argv)
{
    // printk("%s %s: GPIO53:TX 0x%x\n", __FUNCTION__, uart_dev->name, sys_read32((0x40068000 + 0xD8)));
    // printk("%s %s: GPIO54:RX 0x%x\n", __FUNCTION__, uart_dev->name, sys_read32((0x40068000 + 0xDC)));
    u8_t buf_tx[TEST_UART_BUF_LEN] = { 0 };

    for (int i = 0; i < TEST_UART_BUF_LEN; i++) {
        buf_tx[i] = i;
    }
    
    for (int i = 0; i < TEST_UART_BUF_LEN; i++) {
        uart_poll_out(uart_dev, buf_tx[i]);
    }

    printk("%s Write data successed.\n", __FUNCTION__);
    return 0;
}

static int uart_start_test_read(const struct shell *shell,
        size_t argc, char **argv)
{
    // printk("%s %s: GPIO53:TX 0x%x\n", __FUNCTION__, uart_dev->name, sys_read32((0x40068000 + 0xD8)));
    // printk("%s %s: GPIO54:RX 0x%x\n", __FUNCTION__, uart_dev->name, sys_read32((0x40068000 + 0xDC)));
    u8_t buf_rx[TEST_UART_BUF_LEN] = { 0 };

    for (int i = 0; i < TEST_UART_BUF_LEN; i++) {
        buf_rx[i] = i;
    }
    int ret = 0;
    int i = 0;
    while (i < TEST_UART_BUF_LEN) {
        ret = uart_poll_in(uart_dev, &(buf_rx[i]));
        i++;
    }
    
    for(i = 0; i < TEST_UART_BUF_LEN; i++) {
        printk("%02x ", buf_rx[i]);
        
        if(i != 0 && i % 16 == 0)
            printk("\n");
    }
    printk("%s Read data successed.\n", __FUNCTION__);
    return 0;
}


SHELL_STATIC_SUBCMD_SET_CREATE(test_uart_cmds,
        SHELL_CMD(write, NULL, "start test TX", uart_start_test_write),
        SHELL_CMD(read, NULL, "start test RX", uart_start_test_read),
        SHELL_SUBCMD_SET_END     /* Array terminated. */
        );

SHELL_CMD_REGISTER(test_uart, &test_uart_cmds, "UART commands", NULL);
SYS_INIT(test_uart_main, APPLICATION, 10);
