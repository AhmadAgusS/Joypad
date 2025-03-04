/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FRAMEWORK_DISPLAY_LIBDISPLAY_LVGL_LVGL_GPU_H_
#define FRAMEWORK_DISPLAY_LIBDISPLAY_LVGL_LVGL_GPU_H_

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @cond INTERNAL_HIDDEN
 */

/* Event callback */
typedef void (*lvgl_gpu_event_cb_t)(int status, void *user_data);

/*
 * @brief set lvgl display driver software callback
 *
 * @param disp_drv pointer to structure lv_disp_drv_t
 *
 * @retval 0 on success else negative code
 */
int lvgl_gpu_set_sw_render_cb(lv_disp_drv_t *disp_drv);

/*
 * @brief set lvgl display driver gpu callback
 *
 * @param disp_drv pointer to structure lv_disp_drv_t
 *
 * @retval 0 on success else negative code
 */
int lvgl_gpu_set_render_cb(lv_disp_drv_t *disp_drv);

/*
 * @brief copy buf using gpu
 *
 * This routine neither check dcache clean nor invalidate dcache after copy,
 * so take care if you call this routine.
 *
 * @param disp_drv pointer to structure lv_disp_drv_t
 *
 * @retval 0 on success else negative code
 */
int lvgl_gpu_copy(lv_color_t * dest, lv_coord_t dest_w, const void * src,
		lv_coord_t src_w, lv_img_cf_t src_cf, lv_coord_t copy_w, lv_coord_t copy_h);

/*
 * @brief insert a marker to wait a event
 *
 * Only one marker is supported so far.
 *
 * @param event_cb callback on event arrived
 * @user_data user specific data
 *
 * @retval 0 on success else negative code
 */
int lvgl_gpu_insert_marker(lvgl_gpu_event_cb_t event_cb, void *user_data);

/*
 * @brief wait a marker arrived
 *
 * @retval N/A
 */
void lvgl_gpu_wait_marker_finish(void);

/**
 * INTERNAL_HIDDEN @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* FRAMEWORK_DISPLAY_LIBDISPLAY_LVGL_LVGL_GPU_H_ */
