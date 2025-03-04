/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file ui manager interface
 */

#ifndef __UI_MANGER_H__
#define __UI_MANGER_H__

#include <msg_manager.h>
#include <view_manager.h>
#include <input_manager.h>

#ifdef CONFIG_SEG_LED_MANAGER
#include <seg_led_manager.h>
#endif
#ifdef CONFIG_LED_MANAGER
#include <led_manager.h>
#endif

//#include "led_display.h"
/**
 * @defgroup ui_manager_apis app ui Manager APIs
 * @ingroup system_apis
 * @{
 */

/**
 * @cond INTERNAL_HIDDEN
 */
/**
 * @brief dispatch key event to target view
 *
 * This routine dispatch key event to target view, ui manager will found the target
 * view by view_key_map
 *
 * @param key_event value of key event
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_dispatch_key_event(uint32_t key_event);

/**
 * @brief send ui message to target view
 *
 * This routine send ui message to target view which mark by view id
 *
 * @param view_id id of view
 * @param msg_id id of msg_id
 * @param msg_data param of message
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_send_async(uint16_t view_id, uint8_t msg_id, uint32_t msg_data);

/**
 * @brief send ui message to target view with optional callback
 *
 * This routine send ui message to target view which mark by view id
 *
 * @param view_id id of view
 * @param msg_id id of msg_id
 * @param msg_data param of message
 * @param msg_cb callback of message. "msg_data" is stored in the field "value" of
 *             the 1st "struct app_msg *" param of msg_cb.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_send_async2(uint16_t view_id, uint8_t msg_id, uint32_t msg_data, MSG_CALLBAK msg_cb);

/**
 * @brief send ui message synchronized to target view
 *
 * This routine send ui message to target view which mark by view id
 *
 * @param view_id id of view
 * @param msg_id id of msg_id
 * @param msg_data param of message
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_send_sync(uint16_t view_id, uint8_t msg_id, uint32_t msg_data);

/**
 * @brief send ui message synchronized to target view
 *
 * This routine send ui message to target view which mark by view id
 *
 * @param view_id id of view
 * @param msg_id id of msg_id
 * @param msg_data param of message
 * @param msg_cb callback of message. "msg_data" is stored in the field "value" of
 *             the 1st "struct app_msg *" param of msg_cb.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_send_sync2(uint16_t view_id, uint8_t msg_id, uint32_t msg_data, MSG_CALLBAK msg_cb);

/**
 * @brief wait previous sent messages received and replied
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
static inline int ui_message_wait_reply(void)
{
	return ui_manager_send_sync(VIEW_INVALID_ID, MSG_VIEW_NULL, 0);
}

#ifndef CONFIG_UI_SERVICE
/**
 * @brief dispatch ui message to target view
 *
 * This routine dispatch ui message to target view which mark by view id
 * if the target view not found in view list ,return failed. otherwise target
 * view will process this ui message.
 *
 * @param view_id id of view
 * @param msg_id id of msg_id
 * @param msg_data param of message
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_message_dispatch(uint16_t view_id, uint8_t msg_id, uint32_t msg_data);
#endif

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @brief Create a new view
 *
 * This routine Create a new view. After view created, it is hidden by default.
 *
 * @param view_id init view id
 * @param presenter view presenter
 * @param flags create flags, see enum UI_VIEW_CREATE_FLAGS
 *
 * @retval 0 on invoiked success else negative errno code.
 */
#ifdef CONFIG_UI_SERVICE
int ui_view_create(uint16_t view_id, const void *presenter, uint8_t flags);
#else
int ui_view_create(uint16_t view_id, ui_view_info_t *info);
#endif

/**
 * @brief Inflate view layout
 *
 * This routine inflate view layout, called by view itself if required.
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_layout(uint16_t view_id);

/**
 * @brief update view
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_update(uint16_t view_id);

/**
 * @brief destory view
 *
 * This routine destory view, delete form ui manager view list
 * and release all resource for view.
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_delete(uint16_t view_id);

/**
 * @brief show view
 *
 * This routine show view, show form ui manager view list.
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_show(uint16_t view_id);

/**
 * @brief hide view
 *
 * This routine hide view, hide form ui manager view list.
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_hide(uint16_t view_id);

/**
 * @brief set view order
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_set_order(uint16_t view_id, uint16_t order);

/**
 * @brief set view drag attribute
 *
 * This routine set view drag attribute. If drag_attribute > 0, it implicitly make the view
 * visible, just like view_set_hidden(view_id, false) called.
 *

 * @param view_id id of view
 * @param drag_attribute drag attribute, see enum UI_VIEW_DRAG_ATTRIBUTE
 * @param keep_pos if true, keep the positon unchanged, else move the view to the drag position
 *
 * @retval 0 on success else negative errno code.
 */
int ui_view_set_drag_attribute(uint16_t view_id, uint8_t drag_attribute, bool keep_pos);

/**
 * @brief paint view
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_paint(uint16_t view_id);

/**
 * @brief refresh view
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_refresh(uint16_t view_id);

/**
 * @cond INTERNAL_HIDDEN
 */

/**
 * @brief pause view
 *
 * After view become paused, the view surface may become unreachable.
 * View is not paused by default after created.
 * A paused view is always hidden regardless of the hidden attribute.
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_pause(uint16_t view_id);

/**
 * @brief resume view
 *
 * After view resumed, the view surface becomes reachable.
 *
 * @param view_id id of view
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_resume(uint16_t view_id);

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @brief set view position
 *
 * @param view_id id of view
 * @param x new coordinate X
 * @param y new coordinate Y
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_set_pos(uint16_t view_id, int16_t x, int16_t y);

/**
 * @brief send user message to view
 *
 * @param view_id id of view
 * @param msg_id user message id, must not less than MSG_VIEW_USER_OFFSET
 * @param msg_data user message data
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_view_send_user_msg(uint16_t view_id, uint8_t msg_id, void *msg_data);

/**
 * @brief popup a message box
 *
 * This routine will lock the gesture, just like ui_gesture_lock_scroll() is called.
 * If the registered popup callback failed, should unlock
 * gesture by ui_gesture_unlock_scroll().
 *
 * @param msgbox_id user defined message box id
 * @param user_data user data
 *
 * @retval 0 on invoiked success else negative code.
 */
int ui_msgbox_popup(uint16_t msgbox_id, void *user_data);

/**
 * @brief close a message box
 *
 * @param msgbox_id user defined message box id.
 * @param bsync synchronous flag
 *
 * @retval 0 on invoiked success else negative code.
 */
int ui_msgbox_close(uint16_t msgbox_id, bool bsync);

/**
 * @brief Update a message box
 *
 * @param msgbox_id user defined message box id.
 * @param reason paint reason
 *
 * @retval 0 on invoiked success else negative code.
 */
int ui_msgbox_paint(uint16_t msgbox_id, uint32_t reason);

/**
 * @brief ui manager init funcion
 *
 * This routine calls init ui manager ,called by main
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_init(void);

/**
 * @brief ui manager deinit funcion
 *
 * This routine calls deinit ui manager ,called by main
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_exit(void);

/**
 * @brief ui manager lock display post temporarily
 *
 * The post will be locked and display will never update.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_lock_display(void);

/**
 * @brief ui manager unlock display post
 *
 * The display can be updated.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_manager_unlock_display(void);

/**
 * @brief ui manager set gui draw period
 *
 * This routine set gui draw invoking period which is multiple of vsync(te)
 * period. By default, the multiple is 1.
 *
 * @param multiple period multiple of vsync(te) period
 *
 * @retval 0 on succsess else negative code
 */
int ui_manager_set_draw_period(uint8_t multiple);

/**
 * @brief ui manager register msgbox popup callback
 *
 * @param callback function called when a msgbox popup
 *
 * @retval 0 on succsess else negative code
 */
int ui_manager_set_msgbox_popup_callback(ui_msgbox_popup_cb_t callback);

/**
 * @brief ui manager register scroll result notify callback
 *
 * @param callback function called when view state changed takes place
 *
 * @retval 0 on succsess else negative code
 */
int ui_manager_set_scroll_callback(ui_scroll_cb_t callback);

/**
 * @brief ui manager register state notify callback
 *
 * The possible notify states is MSG_VIEW_FOCUS, MSG_VIEW_DEFOCUS, MSG_VIEW_PAUSE, MSG_VIEW_RESUME
 *
 * @param callback function called when view state changed takes place
 *
 * @retval 0 on succsess else negative code
 */
int ui_manager_set_monitor_callback(ui_monitor_cb_t callback);

/**
 * @brief ui manager register gesture/key event default callback
 *
 * The registered callback will only handle the key events passed by
 * ui_manager_dispatch_key_event() (also CONFIG_UISRV_KEY_EVENT_BYPASS
 * is not configured.) and the (inner) gesture events.
 *
 * The event will only passed to the callback if the event is not processed
 * completed by focused view.
 *
 * @param callback function called when gesture/key event take place
 *
 * @retval 0 on succsess else negative code
 */
int ui_manager_set_keyevent_callback(ui_keyevent_cb_t callback);

/**
 * @brief Set surface max buffer count
 *
 * Set surface buffer count for focused view.
 *
 * @param buf_count buffer count, must not exceed CONFIG_SURFACE_MAX_BUFFER_COUNT
 *
 * @retval 0 on invoiked success else negative errno code.
 */
int ui_manager_set_max_buffer_count(uint8_t buf_count);

/* deprecated API */
#define ui_manager_gesture_set_enabled(en) ui_gesture_set_enabled(en)
#define ui_manager_gesture_set_dir(dir) ui_gesture_set_scroll_dir(dir)
#define ui_manager_gesture_lock_scroll ui_gesture_lock_scroll
#define ui_manager_gesture_unlock_scroll ui_gesture_unlock_scroll
#define ui_manager_gesture_wait_release ui_gesture_wait_release

/**
 * @brief ui manager enable/disable gesture detection
 *
 * This routine enable/disable gesture detection. By default, gesture is enabled.
 *
 * @param en enabled flag
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_gesture_set_enabled(bool en);

/**
 * @brief ui manager set gesture direction
 *
 * This routine filter the gestures. By default, all are filtered in.
 *
 * @param dir gesture dir filter flags, like (1 << GESTURE_DROP_DOWN) | (1 << GESTURE_DROP_UP)
 *                 only the gestures filtered will be detected.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_gesture_set_scroll_dir(uint8_t dir);

/**
 * @brief ui manager lock gesture scroll temporarily
 *
 * The scrolling will be stopped and locked.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_gesture_lock_scroll(void);

/**
 * @brief ui manager lock gesture scroll
 *
 * The scrolling will be stopped and locked.
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_gesture_unlock_scroll(void);

/**
 * @brief Ignore gesture detection until the next release
 *
 * @return 0 if invoked succsess.
 * @return others if invoked failed.
 */
int ui_gesture_wait_release(void);

/**
 * @brief Determine if code is running at ui thread context.
 *
 * This routine allows the caller to customize its actions, depending on
 * whether it is running in ui thread context.
 *
 * @funcprops \isr_ok
 *
 * @return false if invoked by a thread.
 * @return true if invoked by an ISR.
 */
bool is_in_ui_thread(void);

/**
 * @} end defgroup system_apis
 */
#endif
