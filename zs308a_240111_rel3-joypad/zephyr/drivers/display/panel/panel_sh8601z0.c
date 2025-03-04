/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <board.h>
#include <device.h>
#include <sys/byteorder.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <drivers/display.h>
#include <drivers/display/display_controller.h>
#include <drivers/display/display_engine.h>
#include <tracing/tracing.h>

#include "panel_common.h"
#include "panel_sh8601z0.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(lcd_panel, CONFIG_DISPLAY_LOG_LEVEL);

/*********************
 *      DEFINES
 *********************/
#if defined(CONFIG_PANEL_BACKLIGHT_PWM_CHAN) || defined(CONFIG_PANEL_BACKLIGHT_GPIO)
#  define CONFIG_PANEL_BACKLIGHT_CTRL   (1)
#ifndef CONFIG_PANEL_BACKLIGHT_DELAYED_FRAMES
#  define CONFIG_PANEL_BACKLIGHT_DELAYED_FRAMES (1)
#endif
#endif

#ifdef CONFIG_LCD_WORK_QUEUE
K_THREAD_STACK_DEFINE(lcd_work_q_stack, CONFIG_LCD_WORK_Q_STACK_SIZE);
#endif

/**
 * For 466x466 round panel, CONFIG_PANEL_FULL_SCREEN_OPT_AREA can be defined as follows:
 * 1) 7 areas
 * #define CONFIG_PANEL_FULL_SCREEN_OPT_AREA \
 * 	{ \
 * 		{ 124,   0, 341,  27 }, \
 * 		{  68,  28, 397,  67 }, \
 * 		{  28,  68, 437, 123 }, \
 * 		{  0,  124, 465, 341 }, \
 * 		{  28, 342, 437, 397 }, \
 * 		{  68, 398, 397, 437 }, \
 * 		{ 124, 438, 341, 465 }, \
 * 	}
 *
 * 2) 3 areas
 * #define CONFIG_PANEL_FULL_SCREEN_OPT_AREA \
 * 	{ \
 * 		{ 68,   0, 397,  67 }, \
 * 		{  0,  68, 465, 397 }, \
 * 		{ 68, 398, 397, 465 }, \
 * 	}
 */

/* time to wait between pixel transfer and next command */
#ifdef CONFIG_PANEL_FULL_SCREEN_OPT_AREA
#  define CONFIG_PANEL_PIXEL_WR_DELAY_US  (17)
#else
#  define CONFIG_PANEL_PIXEL_WR_DELAY_US  (0)
#endif

/**********************
 *      TYPEDEFS
 **********************/
struct lcd_panel_config {
	struct display_videoport videoport;
	struct display_videomode videomode;

#ifdef CONFIG_PANEL_POWER_GPIO
	struct gpio_cfg power_cfg;
#endif

#ifdef CONFIG_PANEL_RESET_GPIO
	struct gpio_cfg reset_cfg;
#endif

#ifdef CONFIG_PANEL_TE_GPIO
	struct gpio_cfg te_cfg;
#endif

#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
	struct {
		uint8_t pwm_chan;
	} backlight_cfg;
#elif defined(CONFIG_PANEL_BACKLIGHT_GPIO)
	struct gpio_cfg backlight_cfg;
#endif
};

struct lcd_panel_data {
	const struct device *lcdc_dev;
	const struct display_callback *callback;
	struct display_buffer_descriptor refr_desc;

#ifdef CONFIG_PANEL_RESET_GPIO
	const struct device *reset_gpio;
#endif

#ifdef CONFIG_PANEL_POWER_GPIO
	const struct device *power_gpio;
#endif

#ifdef CONFIG_PANEL_TE_GPIO
	const struct device *te_gpio;
	struct gpio_callback te_gpio_cb;
#else
	struct k_timer te_timer;
#endif

#ifdef CONFIG_PANEL_BACKLIGHT_CTRL
	const struct device *backlight_dev;
#endif

#ifdef CONFIG_PM_DEVICE
	uint32_t pm_state;
	struct k_work resume_work;
#ifdef CONFIG_LCD_WORK_QUEUE
	struct k_work_q lcd_work_q;
#endif
	uint8_t pm_post_changed : 1;
#endif

#if CONFIG_PANEL_PIXEL_WR_DELAY_US > 0
	uint8_t transfering_last : 1; /* the last transfer in one frame */
#endif

	uint8_t transfering : 1;
	uint8_t disp_on : 1;
	uint8_t in_sleep : 1;
#if CONFIG_PANEL_BACKLIGHT_CTRL
	/* delay several vsync to turn on backlight after blank off */
	uint8_t backlight_delayed;
#endif
	uint8_t brightness;
	uint8_t pending_brightness;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int _lcd_panel_blanking_on(const struct device *dev);
static int _lcd_panel_blanking_off(const struct device *dev);
static int _panel_te_set_enable(const struct device *dev, bool enabled);

#ifdef CONFIG_PM_DEVICE
static void _panel_pm_resume_handler(struct k_work *work);
static void _panel_pm_apply_post_change(const struct device *dev);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *  FUNCTIONS
 **********************/
static void _panel_transmit(struct lcd_panel_data *data, uint32_t cmd,
		const uint8_t *tx_data, size_t tx_count)
{
	display_controller_write_config(data->lcdc_dev, DDIC_QSPI_CMD_WR(cmd), tx_data, tx_count);
}

static void _panel_transmit_p1(struct lcd_panel_data *data, uint32_t cmd, uint8_t tx_data)
{
	_panel_transmit(data, cmd, &tx_data, 1);
}

static void _panel_transmit_p2(struct lcd_panel_data *data, uint32_t cmd, uint8_t tx_data0, uint8_t tx_data1)
{
	uint8_t tx_data[] = { tx_data0, tx_data1 };
	_panel_transmit(data, cmd, tx_data, sizeof(tx_data));
}

static void _panel_exit_sleep(struct lcd_panel_data *data)
{
	_panel_transmit(data, DDIC_CMD_SLPOUT, NULL, 0);
	k_msleep(150);

	data->in_sleep = 0;
}

static void _panel_set_mem_area(struct lcd_panel_data *data, uint16_t x,
				 uint16_t y, uint16_t w, uint16_t h)
{
	uint16_t cmd_data[2];

	x += CONFIG_PANEL_OFFSET_X;
	y += CONFIG_PANEL_OFFSET_Y;

	cmd_data[0] = sys_cpu_to_be16(x);
	cmd_data[1] = sys_cpu_to_be16(x + w - 1);
	_panel_transmit(data, DDIC_CMD_CASET, (uint8_t *)&cmd_data[0], 4);

	cmd_data[0] = sys_cpu_to_be16(y);
	cmd_data[1] = sys_cpu_to_be16(y + h - 1);
	_panel_transmit(data, DDIC_CMD_PASET, (uint8_t *)&cmd_data[0], 4);
}

static void _panel_reset(const struct device *dev)
{
	const struct lcd_panel_config *config = dev->config;
	struct lcd_panel_data *data = dev->data;

	LOG_DBG("Resetting display");

#ifdef CONFIG_PANEL_RESET_GPIO
	gpio_pin_set(data->reset_gpio, config->reset_cfg.gpion, 1);
#endif

#ifdef CONFIG_PANEL_POWER_GPIO
	gpio_pin_set(data->power_gpio, config->power_cfg.gpion, 1);
#endif

#ifdef CONFIG_PANEL_RESET_GPIO
	k_msleep(10);
	gpio_pin_set(data->reset_gpio, config->reset_cfg.gpion, 0);
	k_msleep(150);
#endif
}

static void _panel_init(struct lcd_panel_data *data)
{
	/* Sleep Out */
	_panel_exit_sleep(data);

	/* mem area */
	_panel_set_mem_area(data, 0, 0, 466, 466);

	/* te */
	const uint8_t data_44[] = { CONFIG_PANEL_TE_SCANLINE >> 8, CONFIG_PANEL_TE_SCANLINE & 0xFF };
	_panel_transmit(data, DDIC_CMD_TESCAN, data_44, sizeof(data_44));
	_panel_transmit_p1(data, DDIC_CMD_TEON, 0x00);

	/* pixel format RGB-565 */
	_panel_transmit_p1(data, DDIC_CMD_COLMOD, 0x55);

	/* brightness */
	_panel_transmit_p2(data, DDIC_CMD_WRDISBV, CONFIG_PANEL_BRIGHTNESS, 0);
	/* HBM brightness */
	_panel_transmit_p2(data, DDIC_CMD_HBM_WRDISBV, CONFIG_PANEL_BRIGHTNESS, 0);
	/* AOD brightness */
	_panel_transmit_p2(data, DDIC_CMD_AOD_WRDISBV, CONFIG_PANEL_AOD_BRIGHTNESS, 0);

	_panel_transmit_p1(data, 0x53, 0x20);
	_panel_transmit_p1(data, 0xC4, 0x00);

	/* Display on */
	//_panel_transmit(data, DDIC_CMD_DISPON, NULL, 0);
}

static void _panel_power_on(const struct device *dev)
{
	const struct lcd_panel_config *config = dev->config;
	struct lcd_panel_data *data = dev->data;

	display_controller_enable(data->lcdc_dev, &config->videoport);
	display_controller_set_mode(data->lcdc_dev, &config->videomode);

	_panel_reset(dev);
	_panel_init(data);

	/* Display On */
	_lcd_panel_blanking_off(dev);
}

#ifdef CONFIG_PM_DEVICE
static void _panel_power_off(const struct device *dev)
{
	const struct lcd_panel_config *config = dev->config;
	struct lcd_panel_data *data = dev->data;

	_lcd_panel_blanking_on(dev);

#ifdef CONFIG_PANEL_POWER_GPIO
#ifdef CONFIG_PANEL_RESET_GPIO
	gpio_pin_set(data->reset_gpio, config->reset_cfg.gpion, 1);
#endif
	gpio_pin_set(data->power_gpio, config->power_cfg.gpion, 0);
#endif /* CONFIG_PANEL_POWER_GPIO */
}
#endif /* CONFIG_PM_DEVICE */

static void _panel_apply_brightness(const struct device *dev, uint8_t brightness)
{
	const struct lcd_panel_config *config = dev->config;
	struct lcd_panel_data *data = dev->data;

	if (data->brightness == brightness)
		return;

#if CONFIG_PANEL_BACKLIGHT_CTRL
	bool ready = (data->backlight_delayed == CONFIG_PANEL_BACKLIGHT_DELAYED_FRAMES + 1);

	if (ready || brightness == 0) {
	#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
		pwm_pin_set_cycles(data->backlight_dev, config->backlight_cfg.pwm_chan, 255, brightness, 1);
	#else
		gpio_pin_set(data->backlight_dev, config->backlight_cfg.gpion, brightness ? 1 : 0);
	#endif

		/* FIXME: also set the LCD brightness ? */
		_panel_transmit_p2(data, DDIC_CMD_WRDISBV, brightness, 0);
		data->brightness = brightness;
	}

	/* delay several vsync after blank off */
	if (!ready && data->backlight_delayed) {
		data->backlight_delayed++;
	}
#else
	_panel_transmit_p2(data, DDIC_CMD_WRDISBV, brightness, 0);
	data->brightness = brightness;
#endif /* CONFIG_PANEL_BACKLIGHT_CTRL */
}

DEVICE_DECLARE(lcd_panel);

static void _panel_te_handler(void)
{
	const struct device *dev = DEVICE_GET(lcd_panel);
	struct lcd_panel_data *data = dev->data;
	uint32_t timestamp = k_cycle_get_32();

	sys_trace_void(SYS_TRACE_ID_VSYNC);

	if (data->transfering)
		sys_trace_void(SYS_TRACE_ID_LCD_POST_OVERTIME);

	_panel_apply_brightness(dev, data->pending_brightness);
#ifdef CONFIG_PM_DEVICE
	_panel_pm_apply_post_change(dev);
#endif

	if (data->callback && data->callback->vsync) {
		data->callback->vsync(data->callback, timestamp);
	}

	sys_trace_end_call(SYS_TRACE_ID_VSYNC);
}

#ifdef CONFIG_PANEL_TE_GPIO
static void _panel_te_gpio_handler(const struct device *port,
		struct gpio_callback *cb, gpio_port_pins_t pins)
{
	_panel_te_handler();
}

static int _panel_te_set_enable(const struct device *dev, bool enabled)
{
	const struct lcd_panel_config *config = dev->config;
	struct lcd_panel_data *data = dev->data;

	if (gpio_pin_interrupt_configure(data->te_gpio, config->te_cfg.gpion,
				enabled ? GPIO_INT_EDGE_TO_ACTIVE : GPIO_INT_DISABLE)) {
		LOG_ERR("Couldn't config te interrupt (en=%d)", enabled);
		return -ENODEV;
	}

	return 0;
}

#else  /* CONFIG_PANEL_TE_GPIO */

static void _panel_te_timer_handler(struct k_timer *timer)
{
	_panel_te_handler();
}

static int _panel_te_set_enable(const struct device *dev, bool enabled)
{
	struct lcd_panel_data *data = dev->data;

	if (enabled) {
		k_timer_start(&data->te_timer, K_MSEC(0), K_MSEC(CONFIG_PANEL_TE_SCANLINE));
	} else {
		k_timer_stop(&data->te_timer);
	}

	return 0;
}

#endif /* CONFIG_PANEL_TE_GPIO */

static void _panel_prepare_de_transfer(void *arg, const display_rect_t *area)
{
	const struct device *dev = arg;
#if CONFIG_PANEL_PIXEL_WR_DELAY_US > 0
	const struct lcd_panel_config *config = dev->config;
#endif
	struct lcd_panel_data *data = dev->data;

	data->refr_desc.pixel_format = PIXEL_FORMAT_RGB_565;
	data->refr_desc.width = area->w;
	data->refr_desc.height = area->h;
	data->refr_desc.pitch = area->w;

	data->transfering = 1;
#if CONFIG_PANEL_PIXEL_WR_DELAY_US > 0
	data->transfering_last = (area->y + area->h == config->videomode.vactive);
#endif

	sys_trace_u32x4(SYS_TRACE_ID_LCD_POST,
			area->x, area->y, area->x + area->w - 1, area->y + area->h - 1);

	_panel_set_mem_area(data, area->x, area->y, area->w, area->h);

#if 0
	if (config->videoport.type != DISPLAY_PORT_SPI_QUAD) {
		_panel_transmit(data, DDIC_CMD_RAMWR, 0, 0);
	}
#endif
}

static void _panel_start_de_transfer(void *arg)
{
	const struct device *dev = arg;
	struct lcd_panel_data *data = dev->data;
	int res;

	/* source from DE */
	display_controller_set_source(data->lcdc_dev, DISPLAY_CONTROLLER_SOURCE_ENGINE, NULL);
	display_controller_write_pixels(data->lcdc_dev,
			DDIC_QSPI_CMD_WR_PX(DDIC_CMD_RAMWR), DC_INVALID_CMD,
			&data->refr_desc, NULL);
	if (res) {
		LOG_ERR("write pixels failed");
		data->transfering = 0;
	}
}

static void _panel_complete_transfer(void *arg)
{
	const struct device *dev = arg;
	struct lcd_panel_data *data = dev->data;

	sys_trace_end_call(SYS_TRACE_ID_LCD_POST);

	data->transfering = 0;
#if CONFIG_PANEL_BACKLIGHT_CTRL
	/* at least 1 part has refreshed */
	if (data->backlight_delayed == 0) {
		data->backlight_delayed = 1;
	}
#endif

#if CONFIG_PANEL_PIXEL_WR_DELAY_US > 0
	if (data->transfering_last == 0) {
		k_busy_wait(CONFIG_PANEL_PIXEL_WR_DELAY_US);
	}
#endif

	if (data->callback && data->callback->complete) {
		data->callback->complete(data->callback);
	}
}

static int _lcd_panel_blanking_on(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;

#ifdef CONFIG_PM_DEVICE
	if (data->pm_state != PM_DEVICE_STATE_ACTIVE &&
		data->pm_state != PM_DEVICE_STATE_LOW_POWER)
		return -EPERM;
#endif

	if (data->disp_on == 0)
		return 0;

	LOG_INF("display blanking on");
	data->disp_on = 0;
	data->in_sleep = 1;

#ifdef CONFIG_PANEL_BACKLIGHT_CTRL
	data->backlight_delayed = 0;
#endif

	_panel_te_set_enable(dev, false);
	_panel_apply_brightness(dev, 0);

	_panel_transmit(data, DDIC_CMD_DISPOFF, NULL, 0);
	_panel_transmit(data, DDIC_CMD_SLPIN, NULL, 0);

	display_controller_disable(data->lcdc_dev);
	return 0;
}

static int _lcd_panel_blanking_off(const struct device *dev)
{
	const struct lcd_panel_config *config = dev->config;
	struct lcd_panel_data *data = dev->data;

#ifdef CONFIG_PM_DEVICE
	if (data->pm_state != PM_DEVICE_STATE_ACTIVE &&
		data->pm_state != PM_DEVICE_STATE_LOW_POWER)
		return -EPERM;
#endif

	if (data->disp_on == 1)
		return 0;

	LOG_INF("display blanking off");
	data->disp_on = 1;

	if (data->in_sleep)
		_panel_exit_sleep(data);

	_panel_transmit(data, DDIC_CMD_DISPON, NULL, 0);
	k_msleep(80);

	display_controller_enable(data->lcdc_dev, &config->videoport);
	display_controller_set_mode(data->lcdc_dev, &config->videomode);

	_panel_te_set_enable(dev, true);

	return 0;
}

static int _lcd_panel_read(const struct device *dev,
			const uint16_t x,
			const uint16_t y,
			const struct display_buffer_descriptor *desc,
			void *buf)
{
	return -ENOTSUP;
}

static int _lcd_panel_write(const struct device *dev,
			 const uint16_t x,
			 const uint16_t y,
			 const struct display_buffer_descriptor *desc,
			 const void *buf)
{
#if CONFIG_PANEL_PIXEL_WR_DELAY_US > 0
	const struct lcd_panel_config *config = dev->config;
#endif
	struct lcd_panel_data *data = dev->data;
	int res;

	if (desc->pitch < desc->width)
		return -EDOM;

	if (data->transfering) {
		LOG_WRN("last post not finished");
		return -EBUSY;
	}

	data->transfering = 1;
#if CONFIG_PANEL_PIXEL_WR_DELAY_US > 0
	data->transfering_last = (y + desc->height == config->videomode.vactive);
#endif

	sys_trace_u32x4(SYS_TRACE_ID_LCD_POST,
			x, y, x + desc->width - 1, y + desc->height - 1);

	_panel_set_mem_area(data, x, y, desc->width, desc->height);

#if 0
	if (config->videoport.type != DISPLAY_PORT_SPI_QUAD) {
		_panel_transmit(data, DDIC_CMD_RAMWR, 0, 0);
	}
#endif

	display_controller_set_source(data->lcdc_dev, DISPLAY_CONTROLLER_SOURCE_DMA, NULL);

	res = display_controller_write_pixels(data->lcdc_dev,
			DDIC_QSPI_CMD_WR_PX(DDIC_CMD_RAMWR), DC_INVALID_CMD, desc, buf);
	if (res) {
		LOG_ERR("write pixels failed");
		data->transfering = 0;
	}

	return res;
}

static void *_lcd_panel_get_framebuffer(const struct device *dev)
{
	return NULL;
}

static int _lcd_panel_set_brightness(const struct device *dev,
			   const uint8_t brightness)
{
	struct lcd_panel_data *data = dev->data;

	if (data->disp_on == 0)
		return -EPERM;

	if (brightness == data->pending_brightness)
		return 0;

	LOG_INF("display set_brightness %u", brightness);
	data->pending_brightness = brightness;

	/* delayed set in TE interrupt handler */

	return 0;
}

static int _lcd_panel_set_contrast(const struct device *dev,
			 const uint8_t contrast)
{
	return -ENOTSUP;
}

static void _lcd_panel_get_capabilities(const struct device *dev,
			      struct display_capabilities *capabilities)
{
	const struct lcd_panel_config *config = dev->config;

	memset(capabilities, 0, sizeof(struct display_capabilities));
	capabilities->x_resolution = CONFIG_PANEL_HOR_RES;
	capabilities->y_resolution = CONFIG_PANEL_VER_RES;
	capabilities->vsync_period = 1000000u / config->videomode.refresh_rate;
	capabilities->supported_pixel_formats = PIXEL_FORMAT_RGB_565;
	capabilities->current_pixel_format = PIXEL_FORMAT_RGB_565;
	capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;

#ifdef CONFIG_PANEL_TE_GPIO
	capabilities->screen_info = SCREEN_INFO_VSYNC;
#endif
}

static int _lcd_panel_set_pixel_format(const struct device *dev,
			     const enum display_pixel_format pixel_format)
{
	if (pixel_format == PIXEL_FORMAT_RGB_565) {
		return 0;
	}

	LOG_ERR("Pixel format change not implemented");
	return -ENOTSUP;
}

static int _lcd_panel_set_orientation(const struct device *dev,
			    const enum display_orientation orientation)
{
	if (orientation == DISPLAY_ORIENTATION_NORMAL) {
		return 0;
	}

	LOG_ERR("Changing display orientation not implemented");
	return -ENOTSUP;
}

static int _lcd_panel_register_callback(const struct device *dev,
					  const struct display_callback *callback)
{
	struct lcd_panel_data *data = dev->data;

	if (data->callback == NULL) {
		data->callback = callback;
		return 0;
	}

	return -EBUSY;
}

static int _lcd_panel_unregister_callback(const struct device *dev,
					  const struct display_callback *callback)
{
	struct lcd_panel_data *data = dev->data;

	if (data->callback == callback) {
		data->callback = NULL;
		return 0;
	}

	return -EINVAL;
}

static int _lcd_panel_init(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;
	const struct lcd_panel_config *config = dev->config;
	const struct device *de_dev = NULL;

#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
	data->backlight_dev = device_get_binding(CONFIG_PWM_NAME);
	if (data->backlight_dev == NULL) {
		LOG_ERR("Couldn't find pwm device\n");
		return -ENODEV;
	}

	pwm_pin_set_cycles(data->backlight_dev, config->backlight_cfg.pwm_chan, 255, 0, 1);
#elif defined(CONFIG_PANEL_BACKLIGHT_GPIO)
	data->backlight_dev = device_get_binding(config->backlight_cfg.gpio_dev_name);
	if (data->backlight_dev == NULL) {
		LOG_ERR("Couldn't find backlight pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->backlight_dev, config->backlight_cfg.gpion,
				GPIO_OUTPUT_INACTIVE | config->backlight_cfg.flag)) {
		LOG_ERR("Couldn't configure backlight pin");
		return -ENODEV;
	}
#endif /* CONFIG_PANEL_BACKLIGHT_PWM_CHAN */

#ifdef CONFIG_PANEL_RESET_GPIO
	data->reset_gpio = device_get_binding(config->reset_cfg.gpio_dev_name);
	if (data->reset_gpio == NULL) {
		LOG_ERR("Couldn't find reset pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->reset_gpio, config->reset_cfg.gpion,
				GPIO_OUTPUT_ACTIVE | config->reset_cfg.flag)) {
		LOG_ERR("Couldn't configure reset pin");
		return -ENODEV;
	}
#endif /* CONFIG_PANEL_RESET_GPIO */

#ifdef CONFIG_PANEL_POWER_GPIO
	data->power_gpio = device_get_binding(config->power_cfg.gpio_dev_name);
	if (data->power_gpio == NULL) {
		LOG_ERR("Couldn't find power pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->power_gpio, config->power_cfg.gpion,
				GPIO_OUTPUT_ACTIVE | config->power_cfg.flag)) {
		LOG_ERR("Couldn't configure power pin");
		return -ENODEV;
	}
#endif /* CONFIG_PANEL_POWER_GPIO */

#ifdef CONFIG_PANEL_TE_GPIO
	data->te_gpio = device_get_binding(config->te_cfg.gpio_dev_name);
	if (data->te_gpio == NULL) {
		LOG_ERR("Couldn't find te pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->te_gpio, config->te_cfg.gpion,
				GPIO_INPUT | config->te_cfg.flag)) {
		LOG_ERR("Couldn't configure te pin");
		return -ENODEV;
	}

	gpio_init_callback(&data->te_gpio_cb, _panel_te_gpio_handler, BIT(config->te_cfg.gpion));
	gpio_add_callback(data->te_gpio, &data->te_gpio_cb);
#else
	k_timer_init(&data->te_timer, _panel_te_timer_handler, NULL);
#endif /* CONFIG_PANEL_TE_GPIO */

	data->lcdc_dev = device_get_binding(CONFIG_LCDC_DEV_NAME);
	if (data->lcdc_dev == NULL) {
		LOG_ERR("Could not get LCD controller device");
		return -EPERM;
	}

	display_controller_control(data->lcdc_dev,
			DISPLAY_CONTROLLER_CTRL_COMPLETE_CB, _panel_complete_transfer, (void *)dev);

	de_dev = device_get_binding(CONFIG_DISPLAY_ENGINE_DEV_NAME);
	if (de_dev == NULL) {
		LOG_WRN("Could not get display engine device");
	} else {
		display_engine_control(de_dev,
				DISPLAY_ENGINE_CTRL_DISPLAY_PORT, (void *)&config->videoport, NULL);
		display_engine_control(de_dev,
				DISPLAY_ENGINE_CTRL_DISPLAY_MODE, (void *)&config->videomode, NULL);
		display_engine_control(de_dev,
				DISPLAY_ENGINE_CTRL_DISPLAY_PREPARE_CB, _panel_prepare_de_transfer, (void *)dev);
		display_engine_control(de_dev,
				DISPLAY_ENGINE_CTRL_DISPLAY_START_CB, _panel_start_de_transfer, (void *)dev);
	}

	data->disp_on = 0;
	data->pending_brightness = CONFIG_PANEL_BRIGHTNESS;
#ifdef CONFIG_PM_DEVICE
	data->pm_state = PM_DEVICE_STATE_ACTIVE;
	k_work_init(&data->resume_work, _panel_pm_resume_handler);
#ifdef CONFIG_LCD_WORK_QUEUE
	k_work_queue_start(&data->lcd_work_q, lcd_work_q_stack, K_THREAD_STACK_SIZEOF(lcd_work_q_stack), 6, NULL); 
#endif
#endif

	_panel_power_on(dev);
	return 0;
}

#ifdef CONFIG_PM_DEVICE
static void _panel_pm_notify(struct lcd_panel_data *data, uint32_t pm_action)
{
	if (data->callback && data->callback->pm_notify) {
		data->callback->pm_notify(data->callback, pm_action);
	}
}

static void _panel_pm_apply_post_change(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;

	if (data->pm_post_changed == 0) {
		return;
	}

	if (data->pm_state == PM_DEVICE_STATE_LOW_POWER) {
		_panel_transmit(data, DDIC_CMD_AODON, NULL, 0);
		_panel_te_set_enable(dev, false);
	} else {
		_panel_transmit(data, DDIC_CMD_AODOFF, NULL, 0);
		_panel_te_set_enable(dev, true);
	}

	data->pm_post_changed = 0;
}

static int _panel_pm_early_suspend(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;

	if (data->pm_state != PM_DEVICE_STATE_ACTIVE &&
		data->pm_state != PM_DEVICE_STATE_LOW_POWER) {
		return -EPERM;
	}

	LOG_INF("panel early-suspend");

	_panel_pm_notify(data, PM_DEVICE_ACTION_EARLY_SUSPEND);
	_panel_power_off(dev);

	data->pm_state = PM_DEVICE_STATE_SUSPENDED;
	return 0;
}

static void _panel_pm_resume_handler(struct k_work *work)
{
	const struct device *dev = DEVICE_GET(lcd_panel);
	struct lcd_panel_data *data = dev->data;

	LOG_INF("panel resuming");

	data->pm_state = PM_DEVICE_STATE_ACTIVE;
	data->transfering = 0; /* reset flag in case fail again */

#ifdef CONFIG_PANEL_POWER_GPIO
	_panel_power_on(dev);
#else
	_lcd_panel_blanking_off(dev);
#endif /* CONFIG_PANEL_POWER_GPIO */

	_panel_pm_notify(data, PM_DEVICE_ACTION_LATE_RESUME);
	LOG_INF("panel active");
}

static int _panel_pm_late_resume(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;

	if (data->pm_state != PM_DEVICE_STATE_SUSPENDED &&
		data->pm_state != PM_DEVICE_STATE_OFF) {
		return -EPERM;
	}

	LOG_INF("panel late-resume");
#ifdef CONFIG_LCD_WORK_QUEUE
	k_work_submit_to_queue(&data->lcd_work_q, &data->resume_work);
#else
	k_work_submit(&data->resume_work);
#endif
	return 0;
}

static int _panel_pm_enter_low_power(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;

	if (data->pm_state != PM_DEVICE_STATE_ACTIVE) {
		return -EPERM;
	}

	data->pm_state = PM_DEVICE_STATE_LOW_POWER;
	data->pm_post_changed = 1;
	_panel_pm_notify(data, PM_DEVICE_ACTION_LOW_POWER);

	LOG_INF("panel enter low-power");
	return 0;
}

static int _panel_pm_exit_low_power(const struct device *dev)
{
	struct lcd_panel_data *data = dev->data;

	if (data->pm_state != PM_DEVICE_STATE_LOW_POWER) {
		return -EPERM;
	}

	data->pm_state = PM_DEVICE_STATE_ACTIVE;

	/* wait exit flushing */
	while (data->transfering) {
		k_msleep(2);
	}

	data->pm_post_changed = 1;
	_panel_pm_apply_post_change(dev);

	LOG_INF("panel exit low-power");
	return 0;
}

static int _lcd_panel_pm_control(const struct device *dev, enum pm_device_action action)
{
	struct lcd_panel_data *data = dev->data;
	int ret = 0;

	switch (action) {
	case PM_DEVICE_ACTION_EARLY_SUSPEND:
		if (soc_get_aod_mode()) {
			ret = _panel_pm_enter_low_power(dev);
		} else {
			ret = _panel_pm_early_suspend(dev);
		}
		break;

	case PM_DEVICE_ACTION_LATE_RESUME:
		if (data->pm_state == PM_DEVICE_STATE_LOW_POWER) {
			ret = _panel_pm_exit_low_power(dev);
		} else {
			ret = _panel_pm_late_resume(dev);
		}
		break;

	case PM_DEVICE_ACTION_TURN_OFF:
		_panel_pm_early_suspend(dev);
		data->pm_state = PM_DEVICE_STATE_OFF;
		LOG_INF("panel turn-off");
		break;

	default:
		break;
	}

	return ret;
}
#endif /* CONFIG_PM_DEVICE */

static const struct display_driver_api lcd_panel_driver_api = {
	.blanking_on = _lcd_panel_blanking_on,
	.blanking_off = _lcd_panel_blanking_off,
	.write = _lcd_panel_write,
	.read = _lcd_panel_read,
	.get_framebuffer = _lcd_panel_get_framebuffer,
	.set_brightness = _lcd_panel_set_brightness,
	.set_contrast = _lcd_panel_set_contrast,
	.get_capabilities = _lcd_panel_get_capabilities,
	.set_pixel_format = _lcd_panel_set_pixel_format,
	.set_orientation = _lcd_panel_set_orientation,
	.register_callback = _lcd_panel_register_callback,
	.unregister_callback = _lcd_panel_unregister_callback,
};

static const struct lcd_panel_config lcd_panel_config = {
	.videoport = PANEL_VIDEO_PORT_INITIALIZER,
	.videomode = PANEL_VIDEO_MODE_INITIALIZER(PIXEL_FORMAT_RGB_565),

#ifdef CONFIG_PANEL_POWER_GPIO
	.power_cfg = CONFIG_PANEL_POWER_GPIO,
#endif

#ifdef CONFIG_PANEL_RESET_GPIO
	.reset_cfg = CONFIG_PANEL_RESET_GPIO,
#endif

#ifdef CONFIG_PANEL_TE_GPIO
	.te_cfg = CONFIG_PANEL_TE_GPIO,
#endif

#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
	.backlight_cfg = { .pwm_chan = CONFIG_PANEL_BACKLIGHT_PWM_CHAN, }
#elif defined(CONFIG_PANEL_BACKLIGHT_GPIO)
	.backlight_cfg = CONFIG_PANEL_BACKLIGHT_GPIO,
#endif
};

static struct lcd_panel_data lcd_panel_data;

#if IS_ENABLED(CONFIG_PANEL)
DEVICE_DEFINE(lcd_panel, CONFIG_LCD_DISPLAY_DEV_NAME, &_lcd_panel_init,
			_lcd_panel_pm_control, &lcd_panel_data, &lcd_panel_config, POST_KERNEL,
			60, &lcd_panel_driver_api);
#endif
