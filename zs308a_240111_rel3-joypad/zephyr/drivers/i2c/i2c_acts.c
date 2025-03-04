/*
 * Copyright (c) 2017 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief I2C master driver for Actions SoC
 */

//#define DT_DRV_COMPAT actions_acts_i2c

#include <errno.h>
#include <sys/__assert.h>
#include <stdbool.h>
#include <kernel.h>
#include <device.h>
#include <init.h>
#include <drivers/i2c.h>
#include <drivers/dma.h> 	// use dma
#include <soc.h>
#include <board_cfg.h>
#include <sys/ring_buffer.h>

#define LOG_LEVEL CONFIG_I2C_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(i2c_acts_i2c);

//#define I2C_TIMEOUT_MS					Z_TIMEOUT_MS(50)
#define I2C_WAIT_COMPLETE_MS            (2)
#define I2C_WAIT_TX_FIFO_EMPTY          (1000)
#define I2C_WAIT_ASYNC_TIMEOUT_US       (6000000)

/* I2Cx_CTL */
#define I2C_CTL_GRAS					(0x1 << 0)
#define	I2C_CTL_GRAS_ACK				(0)
#define	I2C_CTL_GRAS_NACK				I2C_CTL_GRAS
#define I2C_CTL_RB						(0x1 << 1)
#define I2C_CTL_GBCC_MASK				(0x3 << 2)
#define I2C_CTL_GBCC(x)					(((x) & 0x3) << 2)
#define	I2C_CTL_GBCC_NONE				I2C_CTL_GBCC(0)
#define	I2C_CTL_GBCC_START				I2C_CTL_GBCC(1)
#define	I2C_CTL_GBCC_STOP				I2C_CTL_GBCC(2)
#define	I2C_CTL_GBCC_RESTART			I2C_CTL_GBCC(3)
#define I2C_CTL_EN						(0x1 << 5)
#define I2C_CTL_IRQE					(0x1 << 6)
#define I2C_CTL_BUSSEL					(1 << 7)
#define I2C_CTL_DRQRE					(1 << 8)
#define I2C_CTL_DRQTE					(1 << 9)
#define I2C_CTL_RX_IRQ_THREHOLD_4BYTES	(0x1 << 10)
#define I2C_CTL_ADM_IRQ_EN				(0x1 << 16)
#define I2C_CTL_NACK_IRQ_EN				(0x1 << 17)
#define I2C_CTL_STD_IRQ_EN				(0x1 << 19)
#define I2C_CTL_FIFO_IRQ_EN				(0x1 << 20)

/* I2Cx_CLKDIV */
#define I2C_CLKDIV_DIV_MASK				(0xff << 0)
#define I2C_CLKDIV_DIV(x)				(((x) & 0xff) << 0)

/* I2Cx_STAT */
#define I2C_STAT_RACK					(0x1 << 0)
#define I2C_STAT_BEB					(0x1 << 1)
#define I2C_STAT_IRQP					(0x1 << 2)
#define I2C_STAT_STPD					(0x1 << 4)
#define I2C_STAT_STAD					(0x1 << 5)
#define I2C_STAT_BBB					(0x1 << 6)
#define I2C_STAT_TCB					(0x1 << 7)
#define I2C_STAT_LBST					(0x1 << 8)
#define I2C_STAT_SAMB					(0x1 << 9)
#define I2C_STAT_SRGC					(0x1 << 10)

/* I2Cx_CMD */
#define I2C_CMD_SBE						(0x1 << 0)
#define I2C_CMD_AS_MASK					(0x7 << 1)
#define I2C_CMD_AS(x)					(((x) & 0x7) << 1)
#define I2C_CMD_RBE						(0x1 << 4)
#define I2C_CMD_SAS_MASK				(0x7 << 5)
#define I2C_CMD_SAS(x)					(((x) & 0x7) << 5)
#define I2C_CMD_DE						(0x1 << 8)
#define I2C_CMD_NS						(0x1 << 9)
#define I2C_CMD_SE						(0x1 << 10)
#define I2C_CMD_MSS						(0x1 << 11)
#define I2C_CMD_WRS						(0x1 << 12)
#define I2C_CMD_EXEC					(0x1 << 15)

/* I2Cx_FIFOCTL */
#define I2C_FIFOCTL_NIB					(0x1 << 0)
#define I2C_FIFOCTL_RFR					(0x1 << 1)
#define I2C_FIFOCTL_TFR					(0x1 << 2)

/* I2Cx_FIFOSTAT */
#define I2C_FIFOSTAT_CECB				(0x1 << 0)
#define I2C_FIFOSTAT_RNB				(0x1 << 1)
#define I2C_FIFOSTAT_RFE				(0x1 << 2)
#define I2C_FIFOSTAT_RFF				(0x1 << 3)
#define I2C_FIFOSTAT_TFE				(0x1 << 4)
#define I2C_FIFOSTAT_TFF				(0x1 << 5)
#define I2C_FIFOSTAT_WRS				(0x1 << 6)
#define I2C_FIFOSTAT_RFD_MASK			(0xf << 8)
#define I2C_FIFOSTAT_RFD_SHIFT			(8)
#define I2C_FIFOSTAT_TFD_MASK			(0xf << 12)
#define I2C_FIFOSTAT_TFD_SHIFT			(12)

/* extract fifo level from fifostat */
#define I2C_RX_FIFO_LEVEL(x)			(((x) >> 8) & 0xff)
#define I2C_TX_FIFO_LEVEL(x)			(((x) >> 12) & 0xff)

#define MAX_CNT_ONCE 255

enum i2c_state {
	STATE_INVALID,
	STATE_READ_DATA,
	STATE_WRITE_DATA,
	STATE_TRANSFER_OVER,
	STATE_TRANSFER_ERROR,
};

/* I2C controller */
struct i2c_acts_controller {
	volatile uint32_t ctl;
	volatile uint32_t clkdiv;
	volatile uint32_t stat;
	volatile uint32_t addr;
	volatile uint32_t txdat;
	volatile uint32_t rxdat;
	volatile uint32_t cmd;
	volatile uint32_t fifoctl;
	volatile uint32_t fifostat;
	volatile uint32_t datcnt;
	volatile uint32_t rcnt;
};

struct acts_i2c_config {
	struct i2c_acts_controller *base;
	void (*irq_config_func)(void);
	const char *dma_dev_name;
	uint8_t clock_id;
	uint8_t reset_id;
	uint8_t irq_id;
	uint8_t dma_id;
	uint8_t txdma_chan;
	uint8_t rxdma_chan;
	uint8_t use_dma; //not 0, use dma tranfser
	uint8_t use_cmd; //not 0, use cmd tranfser
	uint32_t clk_freq;
};

/* Device run time data */
struct acts_i2c_data {
	struct k_mutex mutex;
	struct k_sem complete_sem;
	struct i2c_msg *cur_msg;
	uint32_t msg_buf_ptr;
	uint32_t xfer_len;
	enum i2c_state state;
	uint32_t clk_freq;

	// dma usage
	// sturct i2c_context ctx;		// not defined yet
	const struct device *dma_dev;
	struct k_sem dma_sync;
	uint8_t txdma_chan;
	uint8_t rxdma_chan;


#ifdef CONFIG_I2C_SLAVE
	bool master_active;
	uint32_t status;
	struct i2c_slave_config *slave_cfg;
	bool slave_attached;
#endif

#ifdef CONFIG_DEVICE_POWER_MANAGEMENT
	uint32_t device_power_state;
#endif
#ifdef CONFIG_I2C_ASYNC
	struct ring_buf cbuf;
	struct i2c_msg_async cur_msg_async;
#ifdef CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
	uint8_t *rx_sync_buf;	// seems redundant
#endif
	uint8_t on_irq_async_msg : 1; /* If 1 to indicate that irq is on handling async messages */
	uint8_t cur_msg_async_valid : 1; /* If 1 to indicate that the current async message is valid */
	uint8_t i2c_isbusy : 1;
#endif
};

#define DEV_NAME(dev) ((dev)->name)
#define DEV_CFG(dev) \
	((const struct acts_i2c_config *const)(dev)->config)
#define DEV_DATA(dev) \
	((struct acts_i2c_data *const)(dev)->data)

#ifdef CONFIG_I2C_ASYNC
#if IS_ENABLED(CONFIG_I2C_0)
#define I2C0_RINGBUF_SIZE (CONFIG_I2C_0_MAX_ASYNC_ITEMS * sizeof(struct i2c_msg_async))
static uint8_t i2c0_ringbuf[I2C0_RINGBUF_SIZE];
#endif

#if IS_ENABLED(CONFIG_I2C_1)
#define I2C1_RINGBUF_SIZE (CONFIG_I2C_1_MAX_ASYNC_ITEMS * sizeof(struct i2c_msg_async))
static uint8_t i2c1_ringbuf[I2C1_RINGBUF_SIZE];
#endif
#endif

static void i2c_acts_dump_regs(struct i2c_acts_controller *i2c)
{
	LOG_INF("I2C base 0x%x:\n" \
		"  ctl:      %x  clkdiv: %x  stat:    %x\n" \
		"  addr:     %x  cmd:    %x  fifoctl: %x\n" \
		"  fifostat: %x  datcnt: %x  rcnt:    %x\n",
		(unsigned int)i2c,
		i2c->ctl, i2c->clkdiv, i2c->stat,
		i2c->addr, i2c->cmd, i2c->fifoctl,
		i2c->fifostat, i2c->datcnt, i2c->rcnt);
}

static void i2c_acts_set_clk(struct i2c_acts_controller *i2c,
		 uint32_t clk_freq)
{
	uint32_t div;
	uint32_t pclk_freq = CONFIG_HOSC_CLK_MHZ*1000000;

	if ((pclk_freq == 0) || (clk_freq == 0))
		return;

	div = (pclk_freq + clk_freq * 16 - 1) / (clk_freq * 16);
	i2c->clkdiv = I2C_CLKDIV_DIV(div);

	return;
}

static int i2c_acts_configure(const struct device *dev, uint32_t config)
{
	const struct acts_i2c_config *cfg = DEV_CFG(dev);
	struct acts_i2c_data *data = DEV_DATA(dev);
	struct i2c_acts_controller *i2c = cfg->base;
	uint32_t bitrate;

	if (!(config & I2C_MODE_MASTER)) {
		LOG_ERR("Master Mode is not enabled");
		return -EIO;
	}

	if (config & I2C_ADDR_10_BITS) {
		LOG_ERR("I2C 10-bit addressing is currently not supported");
		LOG_ERR("Please submit a patch");
		return -EIO;
	}

	/* Configure clock */
	switch (I2C_SPEED_GET(config)) {
	case I2C_SPEED_STANDARD:
		bitrate = 100000U;
		break;
	case I2C_SPEED_FAST:
		bitrate = 400000U;
		break;
	case I2C_SPEED_FAST_PLUS:
		bitrate = 1000000U;
		break;
	case I2C_SPEED_HIGH:
	case I2C_SPEED_ULTRA:
		bitrate = 1500000U;
		break;

	default:
		LOG_ERR("Unsupported I2C speed value");
		return -EIO;
	}

	/* Setup clock waveform */
	i2c_acts_set_clk(i2c, bitrate);
	data->clk_freq = bitrate;
	LOG_DBG("%s, set clk=%d", dev->name, bitrate);

	return 0;
}

static void i2c_acts_reset(struct i2c_acts_controller *i2c)
{
	/* reenable i2c controller */
	i2c->ctl = 0;

	/* clear i2c status */
	i2c->stat = 0xff;

	/* clear i2c fifo status */
	i2c->fifoctl = I2C_FIFOCTL_RFR | I2C_FIFOCTL_TFR;

	/* wait until fifo reset complete */
	while(i2c->fifoctl & (I2C_FIFOCTL_RFR | I2C_FIFOCTL_TFR))
		;
}

static void i2c_acts_update_cur_async_msg(const struct device *dev,
				struct i2c_msg_async *src_msg)
{
	struct acts_i2c_data *data = DEV_DATA(dev);

	memcpy(&data->cur_msg_async, src_msg, sizeof(struct i2c_msg_async));
#if 0	// copy buffer could be finish in __i2c_acts_write_by_dma
#ifdef CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
	uint8_t i;
	for (i = 0; i < data->cur_msg_async.num_msgs; i++) {
		if (data->cur_msg_async.msg[i].flags & I2C_MSG_READ) {
			memset(data->cur_msg_async.rx_buf, 0,
					sizeof(data->cur_msg_async.rx_buf));
			data->cur_msg_async.msg[i].buf = data->cur_msg_async.rx_buf;
		} else {
			memcpy(data->cur_msg_async.tx_buf,
					data->cur_msg_async.msg[i].buf,
					data->cur_msg_async.msg[i].len);
			// data->cur_msg_async.msg[i].buf = data->cur_msg_async.tx_buf;
			for (int j = 0; j < data->cur_msg_async.msg[i].len; j++) {
				data->cur_msg_async.tx_buf[j + 1] = data->cur_msg_async.msg[i].buf[j];
				printk("%s, #%d [ %p ] = %04x\n", __func__, j, &(data->cur_msg_async.tx_buf[j]), data->cur_msg_async.tx_buf[j]);
			}
		}
	}
#endif
#endif

	data->cur_msg_async_valid = 1;
}

static int i2c_acts_wait_complete(struct i2c_acts_controller *i2c,
					uint32_t timeout_ms, bool is_read)
{
	uint32_t start_time, curr_time;
	int i = 0;

	start_time = k_cycle_get_32();
	while (!(i2c->fifostat & I2C_FIFOSTAT_CECB)) {
		curr_time = k_cycle_get_32();
		if (k_cyc_to_us_floor32(curr_time - start_time) >= (timeout_ms * 1000)) {
			LOG_ERR("wait i2c cmd done timeout");
			return -ETIMEDOUT;
		}
	}

	/* wait data really output to device */
	if(!is_read){
		for (i = 0; i < I2C_WAIT_TX_FIFO_EMPTY; i++) {
			if (i2c->fifostat & I2C_FIFOSTAT_TFE)
				break;
			k_busy_wait(1);
		}
	}

	if (i == I2C_WAIT_TX_FIFO_EMPTY) {
		LOG_ERR("wait i2c tx fifo:0x%x empty timeout",
				i2c->fifostat);
		return -ETIMEDOUT;
	}

	return 0;
}

#if defined(CONFIG_I2C_SLAVE)
static void i2c_slave_acts_isr(struct device *dev);
#endif

static int __i2c_acts_transfer_by_cpu(const struct device *dev, struct i2c_msg *msgs,
			     uint8_t num_msgs, uint16_t addr)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);
	struct acts_i2c_data *data =  DEV_DATA(dev);
	struct i2c_acts_controller *i2c = config->base;
	struct i2c_msg *msg ;
	int i, is_read, ret = 0;
	uint32_t fifo_cmd;  /* fifostat */

	/* Mark subjective as master; in isr, it will triggered another
	 isr for slave if this master_active is false */
#if defined(CONFIG_I2C_SLAVE)
	data->master_active = true;
#endif

	i2c_acts_reset(i2c);

	/* enable I2C controller IRQ */
	/* I2C_CTL_IRQE: 以下條件產生IRQ: (1) write: 計數器沒有計數到零，irq起來；計數到零，要等stop才起來 (2) read: rxfifo滿足當初設的觸發條件
	(3) 產生/收到stop (4) 從設備收到符合本地從地址 (5) 收到nack(選擇不忽略的情況下) */
	/* I2C_CTL_EN: reset status machine to IDLE */
	/* I2C_CTL_STD_IRQ_EN: Stop detect IRQ Enable */
	/* I2C_CTL_FIFO_IRQ_EN: Transfer Complete in fifo mode, IRQ Enable */
	i2c->ctl = I2C_CTL_IRQE | I2C_CTL_EN | I2C_CTL_STD_IRQ_EN | I2C_CTL_FIFO_IRQ_EN;
	// i2c->ctl = I2C_CTL_EN | I2C_CTL_FIFO_IRQ_EN;

	/* I2C_CMD_EXEC: execute (fifo) */
	/* I2C_CMD_MSS: master mode */
	/* I2C_CMD_SE: stop enable */
	/* I2C_CMD_DE: data enable */
	/* I2C_CMD_NS: generate the NACK signal at 9th clock of SCL of the last byte when read data */
	/* I2C_CMD_SBE: start bit enable */
	fifo_cmd = I2C_CMD_EXEC | I2C_CMD_MSS | I2C_CMD_SE | I2C_CMD_DE
		| I2C_CMD_NS | I2C_CMD_SBE;

	if (num_msgs == 2) {
		/* set internal address and restart cmd for read operation */
		fifo_cmd |= I2C_CMD_AS(msgs[0].len + 1) | I2C_CMD_SAS(1);

		/* write i2c device address */
		i2c->txdat = (addr << 1);

		/* write internal register address */
		for (i = 0; i < msgs[0].len; i++)
			i2c->txdat = msgs[0].buf[i];

		msg = &msgs[1];
		/* restart flag */
		if (msg->flags & I2C_MSG_RESTART) {
			fifo_cmd |= I2C_CMD_RBE;
		}
	} else {
		/* only send device addess for 1 message */
		/* I2C_CMD_AS: Address include 1 byte address */
		fifo_cmd |= I2C_CMD_AS(1);
		msg = &msgs[0];
	}

	data->cur_msg = msg;
	data->msg_buf_ptr = 0;

	LOG_DBG("msg flags:0x%x addr:0x%x buf:%p len:0x%x num_msgs:%d cur_msg:%p",
			msg->flags, addr, msg->buf, msg->len, num_msgs, data->cur_msg);

	is_read = ((msg->flags & I2C_MSG_RW_MASK) == I2C_MSG_READ) ? 1 : 0;
	 /* set data count for the message */
	if (msg->len > MAX_CNT_ONCE) {
		data->xfer_len = MAX_CNT_ONCE;
		/* Set SE(10) and NS(9) as 0(disable): Don't stop, and don't send NACK signal after 9th clock of SCL */
		fifo_cmd &= ~(I2C_CMD_SE | I2C_CMD_NS);
		printk("--i2c is_read=%d  len=%d\n", is_read, msg->len);
	} else {
		data->xfer_len = msg->len;
	}
	i2c->datcnt = data->xfer_len;

	if (is_read) {
		/* read from device, with WR bit */
		i2c->txdat = (addr << 1) | 1;
		data->state = STATE_READ_DATA;
	} else {
		/* write to device */
		if ((num_msgs == 1) || (msg->flags & I2C_MSG_RESTART)) {
			i2c->txdat = (addr << 1);
		}

		/* Write data to FIFO */
		for (i = 0; i < msg->len; i++) {
			if (i2c->fifostat & I2C_FIFOSTAT_TFF){
				LOG_DBG("#%d, i2c->fifostat & I2C_FIFOSTAT_TFF=1", i);
				break;
			}
				
			LOG_DBG("#%d: %02x", i, msg->buf[i]);

			i2c->txdat = msg->buf[i];
		}
		data->msg_buf_ptr = i;
		data->state = STATE_WRITE_DATA;
	}

	i2c->fifoctl = 0;

	/* write fifo command to start transfer */
	i2c->cmd = fifo_cmd;

	return ret;
}

#ifdef I2C_DMA_ENABLED
#if 0
static void i2c_acts_dump_dma_status(const struct device *dev)
{
	const struct acts_i2c_config *cfg = DEV_CFG(dev);
	struct acts_i2c_data *data = DEV_DATA(dev);
	struct dma_status stat = { 0 };

	if (!cfg->use_dma)
		return;

	int ret = dma_get_status(data->dma_dev, data->txdma_chan, &stat);
	LOG_INF("I2C TX dma(ch:%d): get_status ret=%d, busy=%d, rc=%d\n",
			cfg->txdma_chan, ret, stat.busy, stat.pending_length);

	ret = dma_get_status(data->dma_dev, data->rxdma_chan, &stat);
	LOG_INF("I2C RX dma(ch:%d): get_status ret=%d, busy=%d, rc=%d\n",
			cfg->rxdma_chan, ret, stat.busy, stat.pending_length);
}
#endif

static int i2c_acts_start_dma(const struct acts_i2c_config *cfg,
                  struct acts_i2c_data *data,
                  uint32_t dma_chan,
                  uint8_t *buf,
                  int32_t len,
                  bool is_tx,
                  dma_callback_t callback)
{
	struct i2c_acts_controller *i2c = cfg->base;
    struct dma_config dma_cfg = {0};
    struct dma_block_config dma_block_cfg = {0};

    if (callback) {
        dma_cfg.dma_callback = callback;
        dma_cfg.user_data = data;
        dma_cfg.complete_callback_en = 1;
    }

    dma_cfg.block_count = 1;
    dma_cfg.head_block = &dma_block_cfg;
    dma_block_cfg.block_size = len;
    if (is_tx) {
        dma_cfg.dma_slot = cfg->dma_id;
        dma_cfg.channel_direction = MEMORY_TO_PERIPHERAL;
        dma_block_cfg.source_address = (uint32_t)buf;
        dma_block_cfg.dest_address = (uint32_t)&i2c->txdat;
        dma_cfg.dest_data_size = 1;
    } else {
        dma_cfg.dma_slot = cfg->dma_id;
        dma_cfg.channel_direction = PERIPHERAL_TO_MEMORY;
        dma_block_cfg.source_address = (uint32_t)&i2c->rxdat;
        dma_block_cfg.dest_address = (uint32_t)buf;
        dma_cfg.source_data_size = 1;
    }

    dma_cfg.source_burst_length = 1;

    if (dma_config(data->dma_dev, dma_chan, &dma_cfg)) {
        LOG_ERR("dma ch[%d] config error", dma_chan);
        return -1;
    }
    
	if (dma_start(data->dma_dev, dma_chan)) {
        LOG_ERR("dma ch[%d] start error", dma_chan);
        return -1;
    }

    return 0;
}

static inline void i2c_acts_stop_dma(struct acts_i2c_data *data, uint32_t dma_chan)
{
    dma_stop(data->dma_dev, dma_chan);
}

static inline void i2c_stop_dma_and_resetctl(struct i2c_acts_controller *i2c, struct acts_i2c_data *data)
{
	i2c_acts_stop_dma(data, data->rxdma_chan);
	i2c_acts_stop_dma(data, data->txdma_chan);
	i2c->ctl = 0;
}

static int __i2c_acts_transfer(const struct device *dev, struct i2c_msg *msgs,
			     uint8_t num_msgs, uint16_t addr);
static void dma_done_callback(const struct device *dev, void *callback_data, uint32_t ch, int type)
{
    struct acts_i2c_data *data = (struct acts_i2c_data *)callback_data;
	const struct acts_i2c_config *config =  DEV_CFG(dev);
	struct i2c_acts_controller *i2c = config->base;

    LOG_DBG("i2c dma ch:%d transfer is done", ch);

    i2c_stop_dma_and_resetctl(i2c, data);

#ifdef CONFIG_I2C_ASYNC
	/* call async callback function if it exists */
	if (data->cur_msg_async.async_func) {
		data->cur_msg_async.async_func(data->cur_msg_async.cb_data,
		data->cur_msg_async.msg, data->cur_msg_async.num_msgs,
		false);		// there's no reg to check dma error, so always send false
	}
	data->cur_msg_async_valid = 0;
#ifdef CONFIG_I2C_SLAVE
	data->master_active = false;
#endif
#if 0
	/* read next async message */
	struct i2c_msg_async msg_async = {0};
	int ret = ring_buf_get(&data->cbuf, (uint8_t *)&msg_async, sizeof(struct i2c_msg_async));
	if (ret > 0) {
		LOG_DBG("read cbuf msg addr:0x%x", msg_async.addr);
		data->on_irq_async_msg = 1;

		i2c_acts_update_cur_async_msg(dev, &msg_async);

		__i2c_acts_transfer(dev, data->cur_msg_async.msg,
			data->cur_msg_async.num_msgs, data->cur_msg_async.addr);
	} else {
		ring_buf_reset(&data->cbuf);
		data->on_irq_async_msg = 0;
		data->i2c_isbusy = 0; /*i2c is stop*/

#if defined(CONFIG_I2C_SLAVE)
		data->master_active = false;
#endif
	}
#endif
#endif

	k_sem_give(&data->complete_sem);	// for sync write/read
}

static int __i2c_acts_read_by_dma(const struct device *dev, struct i2c_msg *msg,
			     				  uint8_t num_msgs, uint16_t addr)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);
	struct acts_i2c_data *data =  DEV_DATA(dev);
	struct i2c_acts_controller *i2c = config->base;
	int ret = 0;

	/* Mark subjective as master; in isr, it will redirect to isr for slave, if 
	   it's !data->master_active */
#if defined(CONFIG_I2C_SLAVE)
	data->master_active = true;
#endif

	/* enable I2C controller IRQ */
	/* I2C_CTL_DRQRE: RX DRQ Enable(8) */
	/* I2C_CTL_EN: reset status machine to IDLE */
	/* I2C_CTL_BUSSEL: TX/RX FIFO Bus select as DMA */
	i2c->ctl = I2C_CTL_DRQTE | I2C_CTL_DRQRE | I2C_CTL_EN | I2C_CTL_BUSSEL;

	// set datacnt register as msg->len
	i2c->datcnt = msg->len;

	// setting tx_buf as slave address and read bit
	memset(data->cur_msg_async.rx_buf, 0, I2C_MSG_ASYNC_RX_MAX_BUF);
	memset(data->cur_msg_async.tx_buf, 0, I2C_MSG_ASYNC_TX_MAX_BUF);
	data->cur_msg_async.tx_buf[0] = (addr << 1) | 1;	// 1st byte to sent is slave address
	
	ret = i2c_acts_start_dma(config, data, data->txdma_chan, (uint8_t*)&(data->cur_msg_async.tx_buf), 1,
                 true, NULL);
	if (ret) {
        LOG_ERR("faield to start dma tx chan 0x%x\n", data->txdma_chan);
        i2c_stop_dma_and_resetctl(i2c, data);
		return -EBUSY;
    }

	LOG_DBG("[i2c-read-dma] msg flags:0x%x addr:0x%x buf:%p len:0x%x num_msgs:%d cur_msg:%p",
			msg->flags, addr, msg->buf, msg->len, num_msgs, msg);

	// start dma receive data from i2c->rxdat
	ret = i2c_acts_start_dma(config, data, data->rxdma_chan, msg->buf, msg->len,
                 false, dma_done_callback);
    if (ret) {
        LOG_ERR("faield to start dma tx chan 0x%x\n", data->rxdma_chan);
        i2c_stop_dma_and_resetctl(i2c, data);
		return -EBUSY;
    }
#if 0
	printk("%s, DMA8CTL=%08x, DMA8START=%d\nDMA8SADDR0=%08x, DMA8DADDR0=%08x\nDMA8BC=%08x\n", __func__,
			sys_read32(0x4001C000 + 0x0900), sys_read32(0x4001C000 + 0x0904),
			sys_read32(0x4001C000 + 0x0908), sys_read32(0x4001C000 + 0x0910),
			sys_read32(0x4001C000 + 0x0918));

	struct dma_status stat = { 0 };
	ret = dma_get_status(data->dma_dev, data->rxdma_chan, &stat);
	printk("dma: dma_get_status ret=%d, stat->busy=%d, stat->pending_length=%d\n", ret, stat.busy, stat.pending_length);

	i2c_acts_dump_regs(i2c);
#endif

	// clear fifoctl reg
	i2c->fifoctl = 0;

	// write fifo command to start transfer
	i2c->cmd = I2C_CMD_EXEC | I2C_CMD_MSS | I2C_CMD_SE | I2C_CMD_DE
				 | I2C_CMD_NS | I2C_CMD_SBE | I2C_CMD_AS(1);

	return 0;

#if 0		// TO-DO: this block is for sync
    /* wait until dma transfer is done */
    k_sem_take(&data->dma_sync, K_FOREVER);
#endif
}

static int __i2c_acts_write_by_dma(const struct device *dev, struct i2c_msg *msg,
			     				   uint8_t num_msgs, uint16_t addr)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);
	struct acts_i2c_data *data =  DEV_DATA(dev);
	struct i2c_acts_controller *i2c = config->base;
	
	int ret = 0;

	/* Mark subjective as master; in isr, it will redirect to isr for slave, if 
	   it's !data->master_active */
#if defined(CONFIG_I2C_SLAVE)
	data->master_active = true;
#endif

	/* enable I2C controller IRQ */
	/* I2C_CTL_DRQTE: TX DRQ Enable(8) */
	/* I2C_CTL_EN: reset status machine to IDLE */
	/* I2C_CTL_BUSSEL: TX/RX FIFO Bus select as DMA */
	i2c->ctl = I2C_CTL_DRQTE | I2C_CTL_EN | I2C_CTL_BUSSEL;
	
	// set datacnt register as msg->len
	i2c->datcnt = msg->len;

	// TODO: 如果後續有時間要把sync/async(有定義buffer的部分分開來)
	// deep copy data to sent from orignal buffer to tx_buf (to prevent from msg->buf released after
	// async function return)
	memset(data->cur_msg_async.tx_buf, 0, I2C_MSG_ASYNC_TX_MAX_BUF);
	data->cur_msg_async.tx_buf[0] = (addr << 1);	// 1st byte to sent is slave address
	
	for (int i = 1; i <= msg->len; i++) {
		data->cur_msg_async.tx_buf[i] = msg->buf[i - 1];
		LOG_DBG("#%d cur_msg_async.tx_buf[ %p ] = %d", i, &(data->cur_msg_async.tx_buf[i]), data->cur_msg_async.tx_buf[i]);
	}

	LOG_DBG("[i2c-write-dma] msg flags:0x%x addr:0x%x buf:%p len:0x%x num_msgs:%d cur_msg:%p",
			msg->flags, addr, data->cur_msg_async.tx_buf, msg->len, num_msgs, msg);

	ret = i2c_acts_start_dma(config, data, data->txdma_chan, (uint8_t*)&(data->cur_msg_async.tx_buf), msg->len + 1,
                 true, dma_done_callback);

    if (ret) {
        LOG_ERR("faield to start dma tx chan 0x%x\n", data->txdma_chan);        
        i2c_stop_dma_and_resetctl(i2c, data);
		return -EBUSY;
    }
	
#if 0
	printk("%s, DMA8CTL=%08x, DMA8START=%d\nDMA8SADDR0=%08x, DMA8DADDR0=%08x\nDMA8BC=%08x\n", __func__,
			sys_read32(0x4001C000 + 0x0900), sys_read32(0x4001C000 + 0x0904),
			sys_read32(0x4001C000 + 0x0908), sys_read32(0x4001C000 + 0x0910),
			sys_read32(0x4001C000 + 0x0918));

	struct dma_status stat = { 0 };
	ret = dma_get_status(data->dma_dev, data->txdma_chan, &stat);
	printk("dma: dma_get_status ret=%d, stat->busy=%d, stat->pending_length=%d\n", ret, stat.busy, stat.pending_length);

	printk("i2c->fifostat=%08x, i2c->stat=%08x\n", i2c->fifostat, i2c->stat);
#endif
	// clear fifoctl reg
	i2c->fifoctl = 0;

	// write fifo command to start transfer
	i2c->cmd = I2C_CMD_EXEC | I2C_CMD_MSS | I2C_CMD_SE | I2C_CMD_DE
				 | I2C_CMD_NS | I2C_CMD_SBE | I2C_CMD_AS(1);
	
	return 0;

#if 0		// TODO: this block is for sync
    /* wait until dma transfer is done */
    k_sem_take(&data->dma_sync, K_FOREVER);
#endif
}
#endif

static int __i2c_acts_transfer(const struct device *dev, struct i2c_msg *msgs,
			     uint8_t num_msgs, uint16_t addr)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);

	if (!config->use_dma) {
		return __i2c_acts_transfer_by_cpu(dev, msgs, num_msgs, addr);

	} else {
#ifdef I2C_DMA_ENABLED
		struct i2c_acts_controller *i2c = config->base;
		// reset i2c ctl, fifostat register
		i2c_acts_reset(i2c);
		
		// check it's read / write task
		int is_read = ((msgs[0].flags & I2C_MSG_RW_MASK) == I2C_MSG_READ) ? 1 : 0;
		
		if (is_read) {
			return __i2c_acts_read_by_dma(dev, &msgs[0], num_msgs, addr);
		} else {
			return __i2c_acts_write_by_dma(dev, &msgs[0], num_msgs, addr);
		}
#endif
		return -EOPNOTSUPP;
	}
}

void i2c_acts_isr(void *arg)
{
	struct device *dev = (struct device *)arg;
	const struct acts_i2c_config *config =  DEV_CFG(dev);
	struct acts_i2c_data *data =  DEV_DATA(dev);
	struct i2c_acts_controller *i2c = config->base;
	struct i2c_msg *cur_msg = data->cur_msg;
	uint32_t stat, fifostat, len, fifo_cmd;
	int ret;

	LOG_DBG("stat:0x%x fifostat:0x%x state:%d",
			i2c->stat, i2c->fifostat, data->state);
	
#if defined(CONFIG_I2C_SLAVE)
	if (data->slave_attached && !data->master_active) {
		i2c_slave_acts_isr(dev);
		return;
	}
#endif

	stat = i2c->stat;
	fifostat = i2c->fifostat;

	/* check error */
	if (fifostat & I2C_FIFOSTAT_RNB) {
		LOG_ERR("no ACK from device");
		data->state = STATE_TRANSFER_ERROR;
		goto stop;
	} else if (stat & I2C_STAT_BEB) {
		LOG_ERR("bus error");
		data->state = STATE_TRANSFER_ERROR;
		goto stop;
	}
	i2c->stat |= I2C_STAT_IRQP;
	LOG_DBG("msg_buf_ptr:%d cur_msg:%p len:%d", data->msg_buf_ptr, cur_msg, data->xfer_len);

	if (data->state == STATE_READ_DATA) {
		/* read data from FIFO */
		while ((!(i2c->fifostat & I2C_FIFOSTAT_RFE)) &&
			data->msg_buf_ptr < data->xfer_len) {
			
			cur_msg->buf[data->msg_buf_ptr++] = i2c->rxdat;
			LOG_DBG("%s L%d, #%d: %02x", __func__, __LINE__, data->msg_buf_ptr, cur_msg->buf[data->msg_buf_ptr-1]);
		}

		/* all data is transfered? */
		if (data->msg_buf_ptr >= cur_msg->len) {
			LOG_DBG("%s L%d, msg_buf_ptr=%d, cur_msg->len=%d", __func__, __LINE__, data->msg_buf_ptr, cur_msg->len);
			data->state = STATE_TRANSFER_OVER;
		}
	} else {
		/* all data is transfered? */
		if (data->msg_buf_ptr >= cur_msg->len) {
			LOG_INF("%s L%d, msg_buf_ptr=%d, cur_msg->len=%d", __func__, __LINE__, data->msg_buf_ptr, cur_msg->len);
			
			data->state = STATE_TRANSFER_OVER;
		}
		/* write data to FIFO */
		while (!(i2c->fifostat & I2C_FIFOSTAT_TFF) &&
		       data->msg_buf_ptr < data->xfer_len) {
			LOG_INF("%s L%d, #%d: %02x", __func__, __LINE__, data->msg_buf_ptr, cur_msg->buf[data->msg_buf_ptr]);
			i2c->txdat = cur_msg->buf[data->msg_buf_ptr++];

			/* wait fifo stat is updated */
			fifostat = i2c->fifostat;
		}
	}
	/* Fifo command execute complete */
	/* Not so sure, in what circumstances this if statement will be entered */
	if (I2C_FIFOSTAT_CECB & fifostat) {
		if (data->msg_buf_ptr < cur_msg->len){
			len = cur_msg->len - data->msg_buf_ptr;
			printk("--i2c irq remain =%d\n", len);
			if (len > MAX_CNT_ONCE){
				data->xfer_len += MAX_CNT_ONCE;
				i2c->datcnt = MAX_CNT_ONCE;
				fifo_cmd = I2C_CMD_EXEC | I2C_CMD_MSS | I2C_CMD_DE;
			} else {
				data->xfer_len += len;
				i2c->datcnt = len;
				fifo_cmd = I2C_CMD_NS | I2C_CMD_SE | I2C_CMD_EXEC | I2C_CMD_MSS | I2C_CMD_DE; // 0x8f00;
			}
			if (data->state != STATE_READ_DATA) {
				/* write data to FIFO */
				while (!(i2c->fifostat & I2C_FIFOSTAT_TFF) &&
					   data->msg_buf_ptr < data->xfer_len) {
					i2c->txdat = cur_msg->buf[data->msg_buf_ptr++];
					LOG_DBG("%s L%d, #%d: %02x", __func__, __LINE__, data->msg_buf_ptr, cur_msg->buf[data->msg_buf_ptr]);
					/* wait fifo stat is updated */
					fifostat = i2c->fifostat;
				}
			}
			i2c->cmd = fifo_cmd;
		}
	}

stop:
	//i2c->stat |= I2C_STAT_IRQP;

	if (data->state == STATE_TRANSFER_ERROR ||
	    data->state == STATE_TRANSFER_OVER) {

		/* FIXME: add extra 2 bytes for TX to generate IRQ */
		if (i2c->datcnt != cur_msg->len) {
			i2c->datcnt = cur_msg->len;
		}

		ret = i2c_acts_wait_complete(i2c, I2C_WAIT_COMPLETE_MS,
			((cur_msg->flags & I2C_MSG_RW_MASK) == I2C_MSG_READ) ? true : false);
		if (ret)
			data->state = STATE_TRANSFER_ERROR;

		/* disable i2c controller */
		i2c->ctl = 0;

		if (data->cur_msg_async.async_func) {
			data->cur_msg_async.async_func(data->cur_msg_async.cb_data,
				data->cur_msg_async.msg, data->cur_msg_async.num_msgs,
			(data->state == STATE_TRANSFER_ERROR)? true : false);
		}
		data->cur_msg_async_valid = 0;

		/* read next async message */
		struct i2c_msg_async msg_async = {0};
		ret = ring_buf_get(&data->cbuf, (uint8_t *)&msg_async, sizeof(struct i2c_msg_async));
		if (ret > 0) {
			LOG_DBG("read cbuf msg addr:0x%x", msg_async.addr);
			data->on_irq_async_msg = 1;

			i2c_acts_update_cur_async_msg(dev, &msg_async);

			__i2c_acts_transfer(dev, data->cur_msg_async.msg,
				data->cur_msg_async.num_msgs, data->cur_msg_async.addr);
		} else {
			ring_buf_reset(&data->cbuf);
			data->on_irq_async_msg = 0;
			data->i2c_isbusy = 0; /*i2c is stop*/

#if defined(CONFIG_I2C_SLAVE)
			data->master_active = false;
#endif
		}
	}
}

static int i2c_acts_transfer_async(const struct device *dev, struct i2c_msg_async *msg_async)
{
	struct acts_i2c_data *data = DEV_DATA(dev);
	// const struct acts_i2c_config *config = DEV_CFG(dev);
	// struct i2c_acts_controller *i2c = config->base;
	int ret, flags;
	bool need_start_flag = false;

	if ((!msg_async->num_msgs) || (msg_async->num_msgs >= 2)) {
		LOG_ERR("invalid msg number:%d (not yet support write_read_async)", msg_async->num_msgs);
		return -EINVAL;
	}

	flags = irq_lock();

	/* If irq is not handling async messages and the current async message is not valid */	
	if ((!data->on_irq_async_msg) && (!data->cur_msg_async_valid))
		need_start_flag = 1;

	if (need_start_flag) {
		i2c_acts_update_cur_async_msg(dev, msg_async);

		ret = __i2c_acts_transfer(dev, data->cur_msg_async.msg,
				data->cur_msg_async.num_msgs, data->cur_msg_async.addr);
		data->i2c_isbusy = 1; /*i2c is busy*/

	} else {	
#ifdef I2C_DMA_ENABLED
		// not yet support task queues (accept task wait in line and support cancel task request)
		LOG_ERR("i2c is on other task: i2c_isbusy=%d, on_irq_async_msg=%d, cur_msg_async_valid=%d",
				 data->i2c_isbusy, data->on_irq_async_msg, data->cur_msg_async_valid);
		ret = -EBUSY;
#else
		LOG_DBG("write msg addr:0x%x to cbuf", msg_async->addr);
		ret = ring_buf_put(&data->cbuf, (const uint8_t *)msg_async, sizeof(struct i2c_msg_async));
		if (!ret) {
			LOG_ERR("write cbuf error(%d, %d)", data->on_irq_async_msg, data->cur_msg_async_valid);
			//i2c_acts_dump_regs(i2c);
			//i2c_acts_reset(i2c);
			//ring_buf_reset(&data->cbuf);
			//data->cur_msg_async_valid = 0;
			//data->on_irq_async_msg = 0;
			ret = -EAGAIN;
		} else {
			ret = 0;
		}
#endif
	}

	irq_unlock(flags);

	return ret;
}

static int i2c_acts_async_dummy_cb(void *cb_data, struct i2c_msg *msgs,
					uint8_t num_msgs, bool is_err)
{
	const struct device *dev = (const struct device *)cb_data;
	struct acts_i2c_data *data = DEV_DATA(dev);

#ifdef CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
	if (data->rx_sync_buf && data->cur_msg_async_valid) {
		uint8_t i;
		for (i = 0; i < data->cur_msg_async.num_msgs; i++) {
			if (data->cur_msg_async.msg[i].flags & I2C_MSG_READ) {
				memcpy(data->rx_sync_buf,
						data->cur_msg_async.msg[i].buf,
						data->cur_msg_async.msg[i].len);
				break;
			}
		}
	}
#endif
	LOG_DBG("%s, finish: release complete_sem", __func__);

	k_sem_give(&data->complete_sem);

	return 0;
}
static uint32_t i2c_get_msg_overtimer(const struct device *dev, struct i2c_msg_async *msg)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);
	struct acts_i2c_data *data =  DEV_DATA(dev);
	uint32_t tlen ,clk_freq, ot;

	tlen = msg->msg[0].len;
	if(msg->num_msgs == 2){
		tlen +=  msg->msg[1].len;
	}
	clk_freq = config->clk_freq;
	if(data->clk_freq)
		clk_freq = data->clk_freq;
	/* > 50*/
	ot = ((tlen<<15)/clk_freq + 50);
	//printk("ot=%d, len=%d, clk_freq=%d\n", ot, tlen, clk_freq);
	return  ot;   // ms = tlen*1000 /clk_freq/10 = 10000*tlen/clk_freq, mul= 3, # (tlen<<15)/clk_freq
}
			
static int i2c_acts_transfer(const struct device *dev, struct i2c_msg *msgs,
			     uint8_t num_msgs, uint16_t addr)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);
	struct acts_i2c_data *data =  DEV_DATA(dev);
	struct i2c_acts_controller *i2c = config->base;
	int i = 0, ret = 0;
	uint32_t start_time, ot;
	struct i2c_msg_async msg_async = {0};

	/* num_msgs: write/read cmd or write+read cmd at the same time, so num_msgs shall be no more than 2*/
	if ((!num_msgs) || (num_msgs > 2)) {
		LOG_ERR("invalid num_msgs:%d", num_msgs);
		return -EINVAL;
	}

	/* init msg_async */
	msg_async.num_msgs = num_msgs;
	msg_async.async_func = i2c_acts_async_dummy_cb;
	msg_async.cb_data = (void *)dev;
	msg_async.addr = addr;

	for (i = 0; i < num_msgs; i++)
		memcpy(&msg_async.msg[i], &msgs[i], sizeof(struct i2c_msg));

	k_mutex_lock(&data->mutex, K_FOREVER);

	/* unknown usage.. */
#ifdef CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
	for (i = 0; i < num_msgs; i++) {
		if (msgs[i].flags & I2C_MSG_READ) {
			data->rx_sync_buf = msgs[i].buf;
			break;
		}
	}
#endif

	/* unknown usage of all ring_buf: wait async messages finished */
	start_time = k_cycle_get_32();
	while (ring_buf_size_get(&data->cbuf) > 0) {
		if (k_cyc_to_us_floor32(k_cycle_get_32() - start_time)
			>= I2C_WAIT_ASYNC_TIMEOUT_US) {
			LOG_ERR("wait async timeout");
			k_mutex_unlock(&data->mutex);
			return -ETIMEDOUT;
		}
	}

	/* reset sem to fix i2c-isr problem */
	k_sem_reset(&data->complete_sem);
	
	/* trigger i2c_acts_transfer_async: for the first 8 bytes of data */
	ret = i2c_acts_transfer_async(dev, &msg_async);
	if (ret) {
		LOG_ERR("i2c async error:%d", ret);
		i2c_acts_dump_regs(i2c);
		goto out;
	}

	//ret = k_sem_take(&data->complete_sem, I2C_TIMEOUT_MS);
	ot = i2c_get_msg_overtimer(dev, &msg_async);
	ret = k_sem_take(&data->complete_sem, Z_TIMEOUT_MS(ot));
	if (ret) {
		/* wait timeout */
		LOG_ERR("addr 0x%x: wait =%d ms timeout ", addr, ot);
		ret = -ETIMEDOUT;
	}

	if (data->state == STATE_TRANSFER_ERROR) {
		LOG_ERR("addr 0x%x: transfer error", addr);
		i2c_acts_dump_regs(i2c);
		ret = -EIO;
	}

#ifdef CONFIG_I2C_ASYNC_MSG_INTERNAL_BUFFER
	data->rx_sync_buf = NULL;
#endif

out:
	if (ret) {
		i2c_acts_dump_regs(i2c);
#ifdef CONFIG_I2C_ASYNC
		data->cur_msg_async_valid = 0;
		ring_buf_reset(&data->cbuf);
		data->on_irq_async_msg = 0;
#endif
	}
	
	k_mutex_unlock(&data->mutex);
	return ret;
}

int i2c_acts_init(const struct device *dev)
{
	const struct acts_i2c_config *config = DEV_CFG(dev);;
	struct acts_i2c_data *data = DEV_DATA(dev);
	int chan;

	/* init semaphore for dma transfer */
	k_sem_init(&data->dma_sync, 0, 1);

	/* init dma dev and channel for i2c */
	if (config->use_dma) {
		data->dma_dev = device_get_binding(config->dma_dev_name);
		if (!config->dma_dev_name) {
			LOG_ERR("dma-dev binding err:%s", config->dma_dev_name);
			return -ENODEV;
		}
		chan = dma_request(data->dma_dev, config->txdma_chan);
		if (chan < 0) {
			LOG_ERR("dma-dev chan config err chan=%d", config->txdma_chan);
			return -ENODEV;
		}
		data->txdma_chan = chan;
		chan = dma_request(data->dma_dev, config->rxdma_chan);
		if (chan < 0) {
			LOG_ERR("dma-dev chan config err chan=%d", config->rxdma_chan);
			return -ENODEV;
		}
		data->rxdma_chan = chan;
	}

	/* enable i2c controller clock */
	acts_clock_peripheral_enable(config->clock_id);

	/* reset i2c controller */
	acts_reset_peripheral(config->reset_id);

	/* setup default clock to 100K */
	i2c_acts_set_clk(config->base, config->clk_freq);

	printk("i2c%d:clk=%d,cmd=%d,dma=%d\n",config->clock_id-CLOCK_ID_I2C0, config->clk_freq,
		config->use_cmd, config->use_dma);

	k_mutex_init(&data->mutex);	// async_ringbuf data
	k_sem_init(&data->complete_sem, 0, UINT_MAX);	// async transfer semaphore

#if IS_ENABLED(CONFIG_I2C_0)
	if ((uint32_t)config->base == I2C0_REG_BASE)
		ring_buf_init(&data->cbuf, sizeof(i2c0_ringbuf), i2c0_ringbuf);
#endif
#if IS_ENABLED(CONFIG_I2C_1)
	if ((uint32_t)config->base == I2C1_REG_BASE)
		ring_buf_init(&data->cbuf, sizeof(i2c1_ringbuf), i2c1_ringbuf);
#endif
	data->on_irq_async_msg = 0;
	data->i2c_isbusy = 0;
	config->irq_config_func();

	return 0;
}

#if defined(CONFIG_I2C_SLAVE)

#define I2C_SLAVE_STATUS_IDLE		0
#define I2C_SLAVE_STATUS_READY		1
#define I2C_SLAVE_STATUS_START		2
#define I2C_SLAVE_STATUS_ADDRESS	3
#define I2C_SLAVE_STATUS_MASTER_WRITE	4
#define I2C_SLAVE_STATUS_MASTER_READ	5
#define I2C_SLAVE_STATUS_STOPED		6
#define I2C_SLAVE_STATUS_ERR		15

static void i2c_slave_acts_isr(struct device *dev)
{
	const struct acts_i2c_config *cfg = DEV_CFG(dev);
	struct acts_i2c_data *data = DEV_DATA(dev);
	struct i2c_acts_controller *i2c = cfg->base;
	const struct i2c_slave_callbacks *cf= data->slave_cfg->callbacks;

	uint32_t stat;
	uint16_t addr;
	uint8_t val;

	stat = i2c->stat;
	/* clear pending */
	i2c->stat = I2C_STAT_IRQP;

	/* check error */
	if (stat & I2C_STAT_BEB) {
		LOG_ERR("bus error\n");
		i2c_acts_dump_regs(i2c);
		i2c_acts_reset(i2c);
		data->status = I2C_SLAVE_STATUS_ERR;
		goto out;
	}

	/* detected start signal */
	if (stat & I2C_STAT_STAD) {
		i2c->stat = I2C_STAT_STAD;
		data->status = I2C_SLAVE_STATUS_START;
	}

	/* recieved address or data  */
	if (stat & I2C_STAT_TCB) {
		i2c->stat = I2C_STAT_TCB;
		if (!(i2c->stat & I2C_STAT_LBST)) {
			/* receive address */
			data->status = I2C_SLAVE_STATUS_ADDRESS;
			addr = i2c->rxdat;
			if ((addr >> 1) != data->slave_cfg->address) {
				LOG_ERR("bus address (0x%x) not matched with (0x%x)\n",
					addr >> 1, data->slave_cfg->address);
				i2c->stat |= 0x1ff;
				goto out;
			}

			if (addr & 1) {
				/* master read */
				cf->read_requested(data->slave_cfg, &val);
				i2c->txdat = val;
				data->status = I2C_SLAVE_STATUS_MASTER_READ;
			} else {
				/* master write */
				i2c->ctl &= ~I2C_CTL_GRAS_ACK;
				cf->write_requested(data->slave_cfg);
				data->status = I2C_SLAVE_STATUS_MASTER_WRITE;
			}
		} else {
			/* receive data */
			if (data->status == I2C_SLAVE_STATUS_MASTER_READ) {
				/* master <--- slave */
				if (!(stat & I2C_STAT_RACK)) {
					/* received NACK */
					goto out;
				}
				cf->read_processed(data->slave_cfg, &val);
				i2c->txdat = val;

		    } else {
				/* master ---> slave */
				val = i2c->rxdat;
				if (!cf->write_received(data->slave_cfg, val))
					i2c->ctl |= I2C_CTL_GRAS_ACK;
			}
		}
	}

	/* detected stop signal */
	if (stat & I2C_STAT_STPD) {
		i2c->stat = I2C_STAT_STPD;
		cf->stop(data->slave_cfg);
		i2c->ctl &= ~I2C_CTL_GRAS_ACK;
		LOG_DBG("%s: Detect stop signal\n", dev->name);
 	}
out:
	i2c->ctl |= I2C_CTL_RB;
}


/* Attach and start I2C as slave */
int i2c_acts_slave_register(const struct device *dev,
			     struct i2c_slave_config *config)
{
	const struct acts_i2c_config *cfg = DEV_CFG(dev);
	struct acts_i2c_data *data = DEV_DATA(dev);
	struct i2c_acts_controller *i2c = cfg->base;

	if (!config) {
		return -EINVAL;
	}

	if (data->slave_attached) {
		LOG_ERR("i2c: err slave is registered\n");
		return -EBUSY;
	}

	if (data->master_active) {
		LOG_ERR("i2c: master is transfer\n");
		return -EBUSY;
	}
	i2c->addr = config->address << 1;
	data->slave_cfg = config;
	data->slave_attached = true;
	LOG_DBG("i2c: slave registered");
	i2c->ctl = I2C_CTL_IRQE | I2C_CTL_EN | I2C_CTL_RB | I2C_CTL_GRAS_ACK | I2C_CTL_STD_IRQ_EN;

	return 0;
}

int i2c_acts_slave_unregister(const struct device *dev,
			       struct i2c_slave_config *config)
{
	const struct acts_i2c_config *cfg = DEV_CFG(dev);
	struct acts_i2c_data *data = DEV_DATA(dev);
	struct i2c_acts_controller *i2c = cfg->base;

	if (!data->slave_attached) {
		return -EINVAL;
	}

	if (data->master_active) {
		return -EBUSY;
	}
	data->slave_cfg = NULL;
	/* disable i2c controller */
	i2c->ctl = 0;

	data->slave_attached = false;

	LOG_DBG("i2c: slave unregistered");

	return 0;
}

#endif /* defined(CONFIG_I2C_SLAVE) */

const struct i2c_driver_api i2c_acts_driver_api = {
	.configure = i2c_acts_configure,
	.transfer = i2c_acts_transfer,
#if defined(CONFIG_I2C_SLAVE)
	.slave_register = i2c_acts_slave_register,
	.slave_unregister = i2c_acts_slave_unregister,
#endif

#if defined(CONFIG_I2C_ASYNC)
	.transfer_async = i2c_acts_transfer_async,
#endif
};

#ifdef CONFIG_PM_DEVICE
int i2c_acts_pm_control(const struct device *device, enum pm_device_action action)
{
	struct acts_i2c_data *data = DEV_DATA(device);
	if(action == PM_DEVICE_ACTION_SUSPEND){
		if (data->i2c_isbusy) {
			printk("i2c busy, not suspend\n");
			return -EINVAL;
		}
	}
	return 0;
}
#else
#define i2c_acts_pm_control 	NULL
#endif


#define  dma_use(n)  	(\
		.dma_dev_name =  CONFIG_DMA_0_NAME, \
		.dma_id = CONFIG_I2C_##n##_DMA_ID, \
		.txdma_chan = CONFIG_I2C_##n##_TXDMA_CHAN, \
		.rxdma_chan = CONFIG_I2C_##n##_RXDMA_CHAN, \
		.use_dma = 1,					\
		)

#define dma_not(n)	(\
		.use_dma = 0, \
		 )

#define I2C_ACTS_INIT(n)						\
	static const struct device DEVICE_NAME_GET(i2c##n##_acts);		\
									\
	static void i2c##n##_acts_irq_config(void)			\
	{								\
		IRQ_CONNECT(IRQ_ID_I2C##n, CONFIG_I2C_##n##_IRQ_PRI,	\
			    i2c_acts_isr,				\
			    DEVICE_GET(i2c##n##_acts), 0);		\
		irq_enable(IRQ_ID_I2C##n);				\
	}								\
									\
	static const struct acts_i2c_config i2c##n##_acts_config = {	\
		.base = (struct i2c_acts_controller *)I2C##n##_REG_BASE,			\
		.irq_config_func = i2c##n##_acts_irq_config,			\
		.clock_id = CLOCK_ID_I2C##n,\
		.reset_id = RESET_ID_I2C##n,\
		.use_cmd = 0,\
		.irq_id = IRQ_ID_I2C##n,			\
		.clk_freq = CONFIG_I2C_##n##_CLK_FREQ,		\
		COND_CODE_1(CONFIG_I2C_##n##_USE_DMA, dma_use(n), dma_not(n))\
	};								\
									\
	static  struct acts_i2c_data i2c##n##_acts_data;		\
									\
	DEVICE_DEFINE(i2c##n##_acts, CONFIG_I2C_##n##_NAME,		\
			    &i2c_acts_init, i2c_acts_pm_control,		\
			    &i2c##n##_acts_data, &i2c##n##_acts_config,	\
			    POST_KERNEL, CONFIG_I2C_INIT_PRIORITY,	\
			    &i2c_acts_driver_api);

#if IS_ENABLED(CONFIG_I2C_0)
I2C_ACTS_INIT(0)
#endif
#if IS_ENABLED(CONFIG_I2C_1)
I2C_ACTS_INIT(1)
#endif

