/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "panel_st77903.h"
#include "panel_common.h"

#include <soc.h>
#include <board.h>
#include <device.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <drivers/display.h>
#include <drivers/display/display_controller.h>
#include <drivers/display/display_engine.h>
#include <tracing/tracing.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(panel_st77903, CONFIG_DISPLAY_LOG_LEVEL);

#if CONFIG_PANEL_PORT_TYPE != DISPLAY_PORT_SPI_QUAD_SYNC
#  error "only support QSPI sync mode"
#endif

#if CONFIG_PANEL_HOR_RES != CONFIG_PANEL_TIMING_HACTIVE
#  error "CONFIG_PANEL_HOR_RES must equal to CONFIG_PANEL_TIMING_HACTIVE"
#endif

#if CONFIG_PANEL_VER_RES != CONFIG_PANEL_TIMING_VACTIVE
#  error "CONFIG_PANEL_VER_RES must equal to CONFIG_PANEL_TIMING_VACTIVE"
#endif

#if defined(CONFIG_PANEL_BACKLIGHT_PWM_CHAN) || defined(CONFIG_PANEL_BACKLIGHT_GPIO)
#  define CONFIG_PANEL_BACKLIGHT_CTRL 1
#endif

#ifdef CONFIG_LCD_WORK_QUEUE
K_THREAD_STACK_DEFINE(lcd_work_q_stack, CONFIG_LCD_WORK_Q_STACK_SIZE);
#endif
struct st77903_data {
	const struct device *lcdc_dev;
	const struct display_callback *callback;
	const struct device *de_dev;
	struct k_timer te_timer;

#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
	const struct device *pwm_dev;
#elif defined(CONFIG_PANEL_BACKLIGHT_GPIO)
	const struct device *backlight_gpio_dev;
#endif

#ifdef CONFIG_PANEL_RESET_GPIO
	const struct device *reset_gpio_dev;
#endif

#ifdef CONFIG_PANEL_POWER_GPIO
	const struct device *power_gpio_dev;
#endif

#ifdef CONFIG_PM_DEVICE
	uint32_t pm_state;
	struct k_work resume_work;
#ifdef CONFIG_LCD_WORK_QUEUE
	struct k_work_q lcd_work_q;
#endif
#endif

	uint8_t blanking_off : 1;
	uint8_t brightness;
	uint8_t pending_brightness;
};


#ifdef CONFIG_PANEL_RESET_GPIO
static const struct gpio_cfg reset_gpio_cfg = CONFIG_PANEL_RESET_GPIO;
#endif

#ifdef CONFIG_PANEL_BACKLIGHT_GPIO
static const struct gpio_cfg backlight_gpio_cfg = CONFIG_PANEL_BACKLIGHT_GPIO;
#endif

#ifdef CONFIG_PANEL_POWER_GPIO
static const struct gpio_cfg power_gpio_cfg = CONFIG_PANEL_POWER_GPIO;
#endif

static const PANEL_VIDEO_PORT_DEFINE(st77903_videoport);
static const PANEL_VIDEO_MODE_DEFINE(st77903_videomode, PIXEL_FORMAT_RGB_565);


static void st77903_transmit(struct st77903_data *data, int cmd,
		const uint8_t *tx_data, size_t tx_count)
{
	display_controller_write_config(data->lcdc_dev, ST77903_WR_CMD(cmd), tx_data, tx_count);
}

static void st77903_transmit_p1(struct st77903_data *data, int cmd, uint8_t tx_data)
{
	st77903_transmit(data, cmd, &tx_data, 1);
}

static void st77903_apply_brightness(struct st77903_data *data, uint8_t brightness)
{
	if (data->brightness == brightness)
		return;

#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
	pwm_pin_set_cycles(data->pwm_dev, CONFIG_PANEL_BACKLIGHT_PWM_CHAN, 255, brightness, 1);
#elif defined(CONFIG_PANEL_BACKLIGHT_GPIO)
	gpio_pin_set(data->backlight_gpio_dev, backlight_gpio_cfg.gpion, brightness ? 1 : 0);
#endif

	data->brightness = brightness;
}

static int st77903_blanking_on(const struct device *dev)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

#ifdef CONFIG_PM_DEVICE
	if (data->pm_state != PM_DEVICE_STATE_ACTIVE &&
		data->pm_state != PM_DEVICE_STATE_LOW_POWER)
		return -EPERM;
#endif

	if (data->blanking_off == 0)
		return 0;

	LOG_INF("display blanking on");
	data->blanking_off = 0;

	k_timer_stop(&data->te_timer);
	st77903_apply_brightness(data, 0);
	//st77903_transmit(data, ST77903_CMD_DISPOFF, NULL, 0);

	/* wait 40 ms timeout */
	display_engine_control(data->de_dev, DISPLAY_ENGINE_CTRL_DISPLAY_SYNC_STOP, (void *)(uintptr_t)40, NULL);
	display_controller_disable(data->lcdc_dev);
	return 0;
}

static int st77903_blanking_off(const struct device *dev)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

#ifdef CONFIG_PM_DEVICE
	if (data->pm_state != PM_DEVICE_STATE_ACTIVE &&
		data->pm_state != PM_DEVICE_STATE_LOW_POWER)
		return -EPERM;
#endif

	if (data->blanking_off == 1)
		return 0;

	LOG_INF("display blanking off");
	data->blanking_off = 1;

	display_controller_enable(data->lcdc_dev, &st77903_videoport);
	display_controller_set_mode(data->lcdc_dev, &st77903_videomode);

	/* refresh not started yet, simply use k_timer as te signal */
	k_timer_start(&data->te_timer, K_MSEC(0), K_MSEC(CONFIG_PANEL_TE_SCANLINE));

	//st77903_transmit(data, ST77903_CMD_DISPON, NULL, 0);
	//k_sleep(K_MSEC(120));

	return 0;
}

static int st77903_read(const struct device *dev,
			const uint16_t x,
			const uint16_t y,
			const struct display_buffer_descriptor *desc,
			void *buf)
{
	return -ENOTSUP;
}

static int st77903_write(const struct device *dev,
			const uint16_t x,
			const uint16_t y,
			const struct display_buffer_descriptor *desc,
			const void *buf)
{
	return -ENOTSUP;
}

static void *st77903_get_framebuffer(const struct device *dev)
{
	return NULL;
}

static int st77903_set_brightness(const struct device *dev,
			   const uint8_t brightness)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

	if (data->blanking_off == 0)
		return -EPERM;

	if (brightness == data->pending_brightness)
		return 0;

	LOG_INF("display set_brightness %u", brightness);
	data->pending_brightness = brightness;

	/* delayed set in TE interrupt handler */

	return 0;
}

static int st77903_set_contrast(const struct device *dev,
			 const uint8_t contrast)
{
	return -ENOTSUP;
}

static void st77903_get_capabilities(const struct device *dev,
			      struct display_capabilities *capabilities)
{
	memset(capabilities, 0, sizeof(struct display_capabilities));
	capabilities->x_resolution = st77903_videomode.hactive;
	capabilities->y_resolution = st77903_videomode.vactive;
	capabilities->vsync_period = (1000000u / st77903_videomode.refresh_rate);
	capabilities->supported_pixel_formats = PIXEL_FORMAT_RGB_565;
	capabilities->current_pixel_format = PIXEL_FORMAT_RGB_565;
	capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;
	capabilities->screen_info = SCREEN_INFO_VSYNC | SCREEN_INFO_ZERO_BUFFER;
}

static int st77903_set_pixel_format(const struct device *dev,
			     const enum display_pixel_format pixel_format)
{
	if (pixel_format == PIXEL_FORMAT_RGB_565) {
		return 0;
	}

	LOG_ERR("Pixel format change not implemented");
	return -ENOTSUP;
}

static int st77903_set_orientation(const struct device *dev,
			    const enum display_orientation orientation)
{
	if (orientation == DISPLAY_ORIENTATION_NORMAL) {
		return 0;
	}

	LOG_ERR("Changing display orientation not implemented");
	return -ENOTSUP;
}

static void st77903_reset_display(struct st77903_data *data)
{
	LOG_DBG("Resetting display");

#ifdef CONFIG_PANEL_RESET_GPIO
	gpio_pin_set(data->reset_gpio_dev, reset_gpio_cfg.gpion, 1);
#endif

#ifdef CONFIG_PANEL_POWER_GPIO
	gpio_pin_set(data->power_gpio_dev, power_gpio_cfg.gpion, 1);
#endif

#ifdef CONFIG_PANEL_RESET_GPIO
	k_sleep(K_MSEC(10));
	gpio_pin_set(data->reset_gpio_dev, reset_gpio_cfg.gpion, 0);
	k_sleep(K_MSEC(120));
#endif
}

static void st77903_lcd_init(struct st77903_data *p_st77903)
{
	st77903_transmit_p1(p_st77903, 0xf0, 0xc3);
	st77903_transmit_p1(p_st77903, 0xf0, 0x96);
	st77903_transmit_p1(p_st77903, 0xf0, 0xa5);
	st77903_transmit_p1(p_st77903, 0xe9, 0x20);

	const uint8_t data_0xe7[] = { 0x80, 0x77, 0x1f, 0xcc, };
	st77903_transmit(p_st77903, 0xe7, data_0xe7, sizeof(data_0xe7));

	const uint8_t data_0xc1[] = { 0x77, 0x07, 0xc2, 0x07, };
	st77903_transmit(p_st77903, 0xc1, data_0xc1, sizeof(data_0xc1));

	const uint8_t data_0xc2[] = { 0x77, 0x07, 0xc2, 0x07, };
	st77903_transmit(p_st77903, 0xc2, data_0xc2, sizeof(data_0xc2));

	const uint8_t data_0xc3[] = { 0x22, 0x02, 0x22, 0x04, };
	st77903_transmit(p_st77903, 0xc3, data_0xc3, sizeof(data_0xc3));

	const uint8_t data_0xc4[] = { 0x22, 0x02, 0x22, 0x04, };
	st77903_transmit(p_st77903, 0xc4, data_0xc4, sizeof(data_0xc4));

	st77903_transmit_p1(p_st77903, 0xc5, 0x71);

	const uint8_t data_0xe0[] = {
		0x87, 0x09, 0x0c, 0x06, 0x05, 0x03, 0x29, 0x32,
		0x49, 0x0f, 0x1b, 0x17, 0x2a, 0x2f,
	};
	st77903_transmit(p_st77903, 0xe0, data_0xe0, sizeof(data_0xe0));

	const uint8_t data_0xe1[] = {
		0x87, 0x09, 0x0c, 0x06, 0x05, 0x03, 0x29, 0x32,
		0x49, 0x0f, 0x1b, 0x17, 0x2a, 0x2f,
	};
	st77903_transmit(p_st77903, 0xe1, data_0xe1, sizeof(data_0xe1));

	const uint8_t data_0xe5[] = {
		0xb2, 0xf5, 0xbd, 0x24, 0x22, 0x25, 0x10, 0x22,
		0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
	};
	st77903_transmit(p_st77903, 0xe5, data_0xe5, sizeof(data_0xe5));

	const uint8_t data_0xe6[] = {
		0xb2, 0xf5, 0xbd, 0x24, 0x22, 0x25, 0x10, 0x22,
		0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
	};
	st77903_transmit(p_st77903, 0xe6, data_0xe6, sizeof(data_0xe6));

	const uint8_t data_0xec[] = { 0x40, 0x03, };
	st77903_transmit(p_st77903, 0xec, data_0xec, sizeof(data_0xec));

	st77903_transmit_p1(p_st77903, 0x36, 0x0c);

	if (st77903_videomode.pixel_format == PIXEL_FORMAT_RGB_565) {
		st77903_transmit_p1(p_st77903, 0x3a, 0x05);
	} else { /* RGB_888 */
		st77903_transmit_p1(p_st77903, 0x3a, 0x07);
	}

	st77903_transmit_p1(p_st77903, 0xb2, 0x00);
	st77903_transmit_p1(p_st77903, 0xb3, 0x00); // ???
	st77903_transmit_p1(p_st77903, 0xb4, 0x00);

	/* Set VFP(8) and VBP(8) in idle mode */
	const uint8_t data_0xb5[] = { 0x00, 0x08, 0x00, 0x8 };
	st77903_transmit(p_st77903, 0xb5, data_0xb5, sizeof(data_0xb5));

	/* Resolution 400x400 */
	const uint8_t data_0xb6[] = { 0xc7, 0x31, };
	st77903_transmit(p_st77903, 0xb6, data_0xb6, sizeof(data_0xb6));

	const uint8_t data_0xa5[] = {
		0x00, 0x00, 0x00, 0x00, 0x20, 0x15, 0x2a, 0x8a,
		0x02
	};
	st77903_transmit(p_st77903, 0xa5, data_0xa5, sizeof(data_0xa5));

	const uint8_t data_0xa6[] = {
		0x00, 0x00, 0x00, 0x00, 0x20, 0x15, 0x2a, 0x8a,
		0x02
	};
	st77903_transmit(p_st77903, 0xa6, data_0xa6, sizeof(data_0xa6));

	const uint8_t data_0xba[] = {
		0x0a, 0x5a, 0x23, 0x10, 0x25, 0x02, 0x00
	};
	st77903_transmit(p_st77903, 0xba, data_0xba, sizeof(data_0xba));

	const uint8_t data_0xbb[] = {
		0x00, 0x30, 0x00, 0x29, 0x88, 0x87, 0x18, 0x00,
	};
	st77903_transmit(p_st77903, 0xbb, data_0xbb, sizeof(data_0xbb));

	const uint8_t data_0xbc[] = {
		0x00, 0x30, 0x00, 0x29, 0x88, 0x87, 0x18, 0x00,
	};
	st77903_transmit(p_st77903, 0xbc, data_0xbc, sizeof(data_0xbc));

	const uint8_t data_0xbd[] = {
		0xa1, 0xb2, 0x2b, 0x1a, 0x56, 0x43, 0x34, 0x65,
		0xff, 0xff, 0x0f,
	};
	st77903_transmit(p_st77903, 0xbd, data_0xbd, sizeof(data_0xbd));

	/* TE on */
	st77903_transmit_p1(p_st77903, 0x35, 0x00);

	st77903_transmit(p_st77903, 0x21, NULL, 0);
	//st77903_transmit(p_st77903, 0x38, NULL, 0);	 /* idle mode off */

	st77903_transmit(p_st77903, 0x11, NULL, 0);	 /* Sleep Out */
	k_sleep(K_MSEC(120));

	st77903_transmit(p_st77903, 0x29, NULL, 0);	/* Display On */
	k_sleep(K_MSEC(120));
}

static int st77903_register_callback(const struct device *dev,
					  const struct display_callback *callback)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

	if (data->callback == NULL) {
		data->callback = callback;
		return 0;
	}

	return -EBUSY;
}

static int st77903_unregister_callback(const struct device *dev,
					  const struct display_callback *callback)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

	if (data->callback == callback) {
		data->callback = NULL;
		return 0;
	}

	return -EINVAL;
}

static void st77903_start_de_transfer(void *arg)
{
	const struct device *dev = arg;
	struct st77903_data *data = (struct st77903_data *)dev->data;
	struct display_buffer_descriptor desc;

	desc.pixel_format = PIXEL_FORMAT_RGB_565;
	desc.pitch = CONFIG_PANEL_TIMING_HACTIVE;
	desc.width = CONFIG_PANEL_TIMING_HACTIVE;
	desc.height = CONFIG_PANEL_TIMING_VACTIVE;

	display_controller_set_source(data->lcdc_dev, DISPLAY_CONTROLLER_SOURCE_ENGINE, NULL);
	display_controller_write_pixels(data->lcdc_dev, ST77903_WR_CMD(ST77903_CMD_VS),
			ST77903_WR_CMD(ST77903_CMD_HS), &desc, NULL);
}

DEVICE_DECLARE(st77903);

static void st77903_vsync_handler(void *arg)
{
	const struct device *dev = DEVICE_GET(st77903);
	struct st77903_data *data = (struct st77903_data *)dev->data;

	st77903_apply_brightness(data, data->pending_brightness);
	k_timer_start(&data->te_timer, K_MSEC(CONFIG_PANEL_TE_SCANLINE), K_MSEC(0));
}

static void st77903_te_timer_handler(struct k_timer *timer)
{
	const struct device *dev = DEVICE_GET(st77903);
	struct st77903_data *data = (struct st77903_data *)dev->data;
	uint32_t timestamp = k_cycle_get_32();

	sys_trace_void(SYS_TRACE_ID_VSYNC);

	if (data->callback && data->callback->vsync) {
		data->callback->vsync(data->callback, timestamp);
	}

	sys_trace_end_call(SYS_TRACE_ID_VSYNC);
}

static int st77903_panel_init(const struct device *dev)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

	display_controller_enable(data->lcdc_dev, &st77903_videoport);

	st77903_reset_display(data);
	st77903_lcd_init(data);

	/* Display On */
	st77903_blanking_off(dev);

	return 0;
}

static void st77903_resume(struct k_work *work);

static int st77903_init(const struct device *dev)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

#ifdef CONFIG_PANEL_BACKLIGHT_PWM_CHAN
	data->pwm_dev = device_get_binding(CONFIG_PWM_NAME);
	if (data->pwm_dev == NULL) {
		LOG_ERR("Couldn't find pwm device\n");
		return -ENODEV;
	}

	pwm_pin_set_cycles(data->pwm_dev, CONFIG_PANEL_BACKLIGHT_PWM_CHAN, 255, 0, 1);
#elif defined(CONFIG_PANEL_BACKLIGHT_GPIO)
	data->backlight_gpio_dev = device_get_binding(backlight_gpio_cfg.gpio_dev_name);
	if (data->backlight_gpio_dev == NULL) {
		LOG_ERR("Couldn't find backlight pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->backlight_gpio_dev, backlight_gpio_cfg.gpion,
				GPIO_OUTPUT_INACTIVE | backlight_gpio_cfg.flag)) {
		LOG_ERR("Couldn't configure backlight pin");
		return -ENODEV;
	}
#endif

#ifdef CONFIG_PANEL_RESET_GPIO
	data->reset_gpio_dev = device_get_binding(reset_gpio_cfg.gpio_dev_name);
	if (data->reset_gpio_dev == NULL) {
		LOG_ERR("Couldn't find reset pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->reset_gpio_dev, reset_gpio_cfg.gpion,
				GPIO_OUTPUT_ACTIVE | reset_gpio_cfg.flag)) {
		LOG_ERR("Couldn't configure reset pin");
		return -ENODEV;
	}
#endif

#ifdef CONFIG_PANEL_POWER_GPIO
	data->power_gpio_dev = device_get_binding(power_gpio_cfg.gpio_dev_name);
	if (data->power_gpio_dev == NULL) {
		LOG_ERR("Couldn't find power pin");
		return -ENODEV;
	}

	if (gpio_pin_configure(data->power_gpio_dev, power_gpio_cfg.gpion,
				GPIO_OUTPUT_ACTIVE | power_gpio_cfg.flag)) {
		LOG_ERR("Couldn't configure power pin");
		return -ENODEV;
	}
#endif

	data->lcdc_dev = (struct device *)device_get_binding(CONFIG_LCDC_DEV_NAME);
	if (data->lcdc_dev == NULL) {
		LOG_ERR("Could not get LCD controller device");
		return -EPERM;
	}

	display_controller_control(data->lcdc_dev, DISPLAY_CONTROLLER_CTRL_COMPLETE_CB,
			st77903_vsync_handler, (void *)dev);

	data->de_dev = (struct device *)device_get_binding(CONFIG_DISPLAY_ENGINE_DEV_NAME);
	if (data->de_dev == NULL) {
		LOG_ERR("Could not get display engine device");
		return -ENODEV;
	}

	display_engine_control(data->de_dev,
			DISPLAY_ENGINE_CTRL_DISPLAY_PORT, (void *)&st77903_videoport, NULL);
	display_engine_control(data->de_dev,
			DISPLAY_ENGINE_CTRL_DISPLAY_MODE, (void *)&st77903_videomode, NULL);
	display_engine_control(data->de_dev,
			DISPLAY_ENGINE_CTRL_DISPLAY_START_CB, st77903_start_de_transfer, (void *)dev);

	data->blanking_off = 0;
	data->pending_brightness = CONFIG_PANEL_BRIGHTNESS;
	k_timer_init(&data->te_timer, st77903_te_timer_handler, NULL);
#ifdef CONFIG_PM_DEVICE
	data->pm_state = PM_DEVICE_STATE_ACTIVE;
	k_work_init(&data->resume_work, st77903_resume);
#ifdef CONFIG_LCD_WORK_QUEUE
	k_work_queue_start(&data->lcd_work_q, lcd_work_q_stack, K_THREAD_STACK_SIZEOF(lcd_work_q_stack), 6, NULL); 
#endif
#endif

	st77903_panel_init(dev);
	return 0;
}

#ifdef CONFIG_PM_DEVICE
__unused static void st77903_enter_sleep(struct st77903_data *data)
{
	st77903_transmit(data, ST77903_CMD_SLPIN, NULL, 0);
//	k_sleep(K_MSEC(5));
}

__unused static void st77903_exit_sleep(struct st77903_data *data)
{
	st77903_transmit(data, ST77903_CMD_SLPOUT, NULL, 0);
	k_sleep(K_MSEC(120));
}

static void st77903_pm_notify(struct st77903_data *data, uint32_t pm_action)
{
	if (data->callback && data->callback->pm_notify) {
		data->callback->pm_notify(data->callback, pm_action);
	}
}

static int st77903_pm_early_suspend(const struct device *dev)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

	if (data->pm_state != PM_DEVICE_STATE_ACTIVE &&
		data->pm_state != PM_DEVICE_STATE_LOW_POWER) {
		return -EPERM;
	}

	LOG_INF("panel early-suspend");

	/* blank display first */
	//st77903_apply_brightness(data, 0);

	st77903_pm_notify(data, PM_DEVICE_ACTION_EARLY_SUSPEND);
	st77903_blanking_on(dev);
	//st77903_enter_sleep(data);

#ifdef CONFIG_PANEL_POWER_GPIO
#ifdef CONFIG_PANEL_RESET_GPIO
	gpio_pin_set(data->reset_gpio_dev, reset_gpio_cfg.gpion, 1);
#endif
	gpio_pin_set(data->power_gpio_dev, power_gpio_cfg.gpion, 0);
#endif /* CONFIG_PANEL_POWER_GPIO */

	data->pm_state = PM_DEVICE_STATE_SUSPENDED;
	return 0;
}

static void st77903_resume(struct k_work *work)
{
	const struct device *dev = DEVICE_GET(st77903);
	struct st77903_data *data = (struct st77903_data *)dev->data;

	LOG_INF("panel resuming");

	data->pm_state = PM_DEVICE_STATE_ACTIVE;

#ifdef CONFIG_PANEL_POWER_GPIO
	st77903_panel_init(dev);
#else
	//st77903_exit_sleep(dev->data);
	st77903_blanking_off(dev);
#endif /* CONFIG_PANEL_POWER_GPIO */

	st77903_pm_notify(data, PM_DEVICE_ACTION_LATE_RESUME);
	LOG_INF("panel active");
}

static int st77903_pm_late_resume(const struct device *dev)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;

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

static int st77903_pm_control(const struct device *dev, enum pm_device_action action)
{
	struct st77903_data *data = (struct st77903_data *)dev->data;
	int ret = 0;

	switch (action) {
	case PM_DEVICE_ACTION_EARLY_SUSPEND:
		ret = st77903_pm_early_suspend(dev);
		break;

	case PM_DEVICE_ACTION_LATE_RESUME:
		ret = st77903_pm_late_resume(dev);
		break;

	case PM_DEVICE_ACTION_TURN_OFF:
		st77903_pm_early_suspend(dev);
		data->pm_state = PM_DEVICE_STATE_OFF;
		LOG_INF("panel turn-off");
		break;

	case PM_DEVICE_ACTION_RESUME:
	case PM_DEVICE_ACTION_SUSPEND:
	case PM_DEVICE_ACTION_FORCE_SUSPEND:
	default:
		break;
	}

	return ret;
}
#endif /* CONFIG_PM_DEVICE */

static const struct display_driver_api st77903_api = {
	.blanking_on = st77903_blanking_on,
	.blanking_off = st77903_blanking_off,
	.write = st77903_write,
	.read = st77903_read,
	.get_framebuffer = st77903_get_framebuffer,
	.set_brightness = st77903_set_brightness,
	.set_contrast = st77903_set_contrast,
	.get_capabilities = st77903_get_capabilities,
	.set_pixel_format = st77903_set_pixel_format,
	.set_orientation = st77903_set_orientation,
	.register_callback = st77903_register_callback,
	.unregister_callback = st77903_unregister_callback,
};

static struct st77903_data st77903_data;

#if IS_ENABLED(CONFIG_PANEL)
DEVICE_DEFINE(st77903, CONFIG_LCD_DISPLAY_DEV_NAME, &st77903_init,
			st77903_pm_control, &st77903_data, NULL, POST_KERNEL,
			60, &st77903_api);
#endif
