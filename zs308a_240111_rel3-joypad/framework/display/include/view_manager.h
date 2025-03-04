/*
 * Copyright (c) 2019 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file view manager interface
 */

#ifndef __VIEW_MANGER_H__
#define __VIEW_MANGER_H__

/**
 * @defgroup ui_manager_apis app ui Manager APIs
 * @ingroup system_apis
 * @{
 */

#include <ui_region.h>
#include <sys/slist.h>
#include <input_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TRANSMIT_ALL_KEY_EVENT  0xFFFFFFFF

#define UI_VIEW_ANIM_SHIFT (10)
#define UI_VIEW_ANIM_RANGE (1 << UI_VIEW_ANIM_SHIFT)

struct ui_view_anim_cfg;

/**
 * @brief View ID
 *
 * view id enumeration
 **/
enum UI_VIEW_ID {
	VIEW_ID_ALL = 0,
	VIEW_ID_USER_OFFSET = 1,
};

#define VIEW_INVALID_ID VIEW_ID_ALL

/**
 * @brief Msgbox ID
 *
 * msgbox id enumeration
 **/
enum UI_MSGBOX_ID {
	MSGBOX_ID_ALL = 0,
	MSGBOX_ID_USER_OFFSET = 1,
};

#define MSGBOX_INVALID_ID MSGBOX_ID_ALL

/**
 * @brief UI Message ID
 *
 * UI message id enumeration
 **/
enum UI_MSG_ID {
	MSG_VIEW_NULL = 0,

	MSG_VIEW_CREATE,  /* creating view, MSG_VIEW_PRELOAD or MSG_VIEW_LAYOUT will be sent to view */
	MSG_VIEW_PRELOAD, /* preload view resource */
	MSG_VIEW_LAYOUT,  /* view layout inflate, can be called by the view itself if required */
	MSG_VIEW_DELETE,  /* deleting view */
	MSG_VIEW_PAINT,   /* update the view normally driven by data changed */
	MSG_VIEW_REFRESH, /* refresh view surface to display */
	MSG_VIEW_FOCUS,   /* view becomes focused */
	MSG_VIEW_DEFOCUS, /* view becomes de-focused */
	MSG_VIEW_PAUSE,   /* view surface becomes unreachable */
	MSG_VIEW_RESUME,  /* view surface becomes reachable */
	MSG_VIEW_UPDATE,  /* resource reload completed */
	MSG_VIEW_RESUME_DISPLAY, /* display unblank */
	MSG_VIEW_KEY,     /* key event, param is struct msg_view_key_data */

	MSG_VIEW_SET_HIDDEN, /* 14 */
	MSG_VIEW_SET_ORDER,
	MSG_VIEW_SET_POS,
	MSG_VIEW_SET_DRAG_ATTRIBUTE,

	MSG_VIEW_SET_CALLBACK, /* 18 */

	MSG_VIEW_SET_DRAW_PERIOD, /* set draw period multiple of vsync(te) period */

	MSG_VIEW_SCROLL_BEGIN,
	MSG_VIEW_SCROLL_END,

	MSG_VIEW_SET_BUF_COUNT,

	MSG_DISPLAY_POST, /* 23 */
	MSG_DISPLAY_RESUME,
	MSG_DISPLAY_LOCK,

	/* msgbox */
	MSG_MSGBOX_POPUP,  /* popup message box */
	MSG_MSGBOX_CLOSE,  /* close message box */
	MSG_MSGBOX_CANCEL, /* cancel message box, sine popup unavailable */
	MSG_MSGBOX_PAINT,  /* update message box */

	/* gesture setting */
	MSG_GESTURE_ENABLE, /* 30 */
	MSG_GESTURE_SET_SCROLL_DIR,
	MSG_GESTURE_LOCK_SCROLL,
	MSG_GESTURE_WAIT_RELEASE,

	/* user message offset for specific view */
	MSG_VIEW_USER_OFFSET = 128,
};

enum UI_CALLBACK_ID {
	UI_CB_MSGBOX,
	UI_CB_SCROLL,
	UI_CB_MONITOR,
	UI_CB_KEYEVENT,

	UI_NUM_CBS,
};

/**
 * @struct msg_view_key_data
 * @brief Structure hoding data of MSG_VIEW_KEY
 *
 * @var event key event
 * @var done the key event processing is done
 */
typedef struct msg_view_key_data {
	uint32_t event;
	bool done; /* the processing is done */
} msg_view_key_data_t;

/**
 * @typedef ui_view_proc
 * @brief Callback API to process view message
 * @param view_id view id
 * @param msg_id message id
 * @param msg_data message data
 *
 * @retval 0 on success else error codes
 */
typedef int (*ui_view_proc_t)(uint16_t view_id, uint8_t msg_id, void *msg_data);

typedef int (*ui_get_state_t)(void);
typedef bool (*ui_state_match_t)(uint32_t current_state, uint32_t match_state);

/**
 * @typedef ui_msgbox_popup_cb
 * @brief Callback API to popup/close the msgbox
 * @param msgbox_id msgbox id
 * @param msg_id message id
 * @param msg_data message data
 *                 For MSG_MSGBOX_POPUP/CLOSE, this is the view id to attached
 *                 For MSG_MSGBOX_PAINT, this is the exact message data passed to uisrv by MSG_MSGBOX_PAINT.
 * @param user_data user data, only passed for MSG_MSGBOX_POPUP/CANCEL
 *
 * @retval N/A
 */
typedef void (*ui_msgbox_popup_cb_t)(uint16_t msgbox_id, uint8_t msg_id, void *msg_data, void *user_data);

/**
 * @typedef ui_scroll_cb
 * @brief Callback API to notify view scroll result, should not block or call any GUI API.
 * @param view_id view id
 * @param msg_id view message id, MSG_VIEW_SCROLL_BEGIN or MSG_VIEW_SCROLL_END
 *
 * @retval N/A
 */
typedef void (*ui_scroll_cb_t)(uint16_t view_id, uint8_t msg_id);

/**
 * @typedef ui_monitor_cb
 * @brief Callback API to notify view state, like focus/de-focus, layout, delete
 * @param view_id view id
 * @param msg_id view message id
 * @param msg_data view message data
 *
 * @retval N/A
 */
typedef void (*ui_monitor_cb_t)(uint16_t view_id, uint8_t msg_id, void *msg_data);

/**
 * @typedef ui_keyevent_cb
 * @brief Callback API to notify (gesture) event
 * @param view_id the focused view when event takes place
 * @param event event id
 *
 * @retval N/A
 */
typedef void (*ui_keyevent_cb_t)(uint16_t view_id, uint32_t event);

/**
 * @typedef ui_view_anim_path
 * @brief Callback API for setting view dragging animation
 * @param scroll_throw_vect indicated the drag speed, it uses similar algorithm as LVGL.
 * @param cfg pointer to structure ui_view_anim_cfg filled by callback
 *
 * @retval N/A
 */
typedef void (*ui_view_drag_anim_cb_t)(uint16_t view_id,
		const ui_point_t *scroll_throw_vect, struct ui_view_anim_cfg *cfg);

/**
 * @typedef ui_view_anim_path
 * @brief Callback API to compute the view move position
 * @param elaps the ratio of elapsed time to the duration. range [0, UI_VIEW_ANIM_RANGE]
 *
 * @retval the interpolation value of move ratio, range [0, UI_VIEW_ANIM_RANGE]
 */
typedef int32_t (*ui_view_anim_path_cb_t)(int32_t elaps);

/* notify the user the animation has stopped */
typedef void (*ui_view_anim_stop_cb_t)(uint16_t view_id, const ui_point_t *pos);

enum UI_VIEW_TYPE {
	UI_VIEW_Unknown = 0,
	UI_VIEW_LVGL,
	UI_VIEW_USER, /* user defined */

	NUM_VIEW_TYPES,
};

enum UI_VIEW_CREATE_FLAGS {
	UI_CREATE_FLAG_NO_PRELOAD = (1 << 0), /* ignore resource preload when created */
	UI_CREATE_FLAG_SHOW = (1 << 1), /* show the view by default */
	UI_CREATE_FLAG_NO_FB = (1 << 4), /* initial no surface buffer */
	UI_CREATE_FLAG_POST_ON_PAINT = (1 << 5), /* only post the view to display when painted */
};

enum UI_VIEW_FLAGS {
	UI_FLAG_HIDDEN       = (1 << 0),
	UI_FLAG_FOCUSED      = (1 << 1), /* user observable focus status */
	UI_FLAG_PAUSED       = (1 << 2),

	UI_FLAG_INFLATED     = (1 << 3), /* layout inflated */
	UI_FLAG_PAINTED      = (1 << 4), /* first paint finished after layout */

	UI_FLAG_DELETING     = (1 << 7), /* view is deleting */
};

enum UI_VIEW_REFRESH_FLAGS {
	/* flags for view refresh */
	UI_REFR_FLAG_MOVED         = (1 << 0), /* refresh due to position changed */
	UI_REFR_FLAG_CHANGED       = (1 << 1), /* refresh due to content changed */
	UI_REFR_FLAG_FIRST_CHANGED = (1 << 2), /* first refresh in frame due to content changed */
	UI_REFR_FLAG_LAST_CHANGED  = (1 << 3), /* last refresh in frame due to content changed */
};

/* DOWN/UP/LEFT/RIGHT indicates the drop/move direction */
enum UI_VIEW_DRAG_ATTRIBUTE {
	UI_DRAG_NONE      = 0,
	UI_DRAG_DROPDOWN  = (1 << 0),
	UI_DRAG_DROPUP    = (1 << 1),
	UI_DRAG_DROPLEFT  = (1 << 2),
	UI_DRAG_DROPRIGHT = (1 << 3),
	UI_DRAG_MOVEDOWN  = (1 << 4),
	UI_DRAG_MOVEUP    = (1 << 5),
	UI_DRAG_MOVELEFT  = (1 << 6),
	UI_DRAG_MOVERIGHT = (1 << 7),
};

typedef struct {
    /** key value, which key is pressed */
	uint32_t key_val;

    /** key type, which key type of the pressed key */
	uint32_t key_type;

    /** app state, the state of app service to handle the message */
	uint32_t app_state;

    /** app msg, the message of app service will be send */
	uint32_t app_msg;

    /** key policy */
	uint32_t key_policy;
} ui_key_map_t;

typedef struct view_data {
	/* gui data */
	//void *context; /* gui context */
	void *display; /* gui display */
	void *surface; /* gui surface to draw on */

	/* custom data */
	const void *presenter; /* view presenter passed by app */
	void *user_data; /* application defined data */
} view_data_t;

typedef struct {
	ui_region_t     region;
	ui_view_proc_t  view_proc;
	ui_get_state_t  view_get_state;
	/** state match function */
	ui_state_match_t view_state_match;
	const ui_key_map_t	*view_key_map;
	void		*app_id;
	uint16_t	flags;
	uint16_t	order;
} ui_view_info_t;

/* app entry structure */
typedef struct view_entry {
	/** app id */
	const char *app_id;
	/** proc function of view */
	ui_view_proc_t  proc;
	/** get state function of view  */
	ui_get_state_t  get_state;
	/**key map of view */
	const ui_key_map_t	*key_map;
	/**view id */
	uint16_t id;
	/**default order */
	uint8_t default_order;
	/**view type */
	uint8_t type;
	/** view width */
	uint16_t width;
	/** view height */
	uint16_t height;
} view_entry_t;

/**
 * @brief Statically define and initialize view entry for view.
 *
 * The view entry define statically,
 *
 * Each view must define the view entry info so that the system wants
 * to find view to knoe the corresponding information
 *
 * @param app_id Name of the app.
 * @param view_proc view proc function.
 * @param view_get_state view get state function.
 * @param view_key_map key map of view .
 * @param view_id view id of view .
 * @param default_order default order of view .
 * @param view_w view width .
 * @param view_h view height.
 */
#ifdef CONFIG_SIMULATOR
#  define VIEW_ENTRY_ATTR
#else
#  define VIEW_ENTRY_ATTR __attribute__((__section__(".view_entry")))
#endif

#define VIEW_DEFINE(app_name, view_proc, view_get_state, view_key_map,\
		view_id, order, view_type, view_w, view_h)	\
	const struct view_entry __view_entry_##app_name##view_id	\
		VIEW_ENTRY_ATTR = {	\
		.app_id = #app_name,							\
		.proc = view_proc,								\
		.get_state = view_get_state,					\
		.key_map = view_key_map,		    			\
		.id = view_id,									\
		.default_order = order,							\
		.type = view_type,								\
		.width = view_w,								\
		.height = view_h,								\
	}

typedef struct {
	sys_snode_t node;

#ifdef CONFIG_UI_SERVICE
	const view_entry_t *entry;
	ui_region_t region;
	uint8_t flags;
	uint8_t refr_flags;
	uint8_t create_flags;
	uint8_t order;
	uint8_t drag_attr;
	/*
	 * internal focus status
	 * modified both by uisrv and display-workq (byte-write is atomic)
	 */
	uint8_t focused;
	ui_view_drag_anim_cb_t drag_anim_cb;

	view_data_t data;
	void *snapshot; /* pointer to the snapshot of the surface buffer */
#else
	void   *app_id;
	ui_view_info_t  info;
	uint16_t   view_id;
#endif
} ui_view_context_t;

enum UI_VIEW_ANIMATION_STATE {
	UI_ANIM_NONE,
	UI_ANIM_START,
	UI_ANIM_RUNNING,
	UI_ANIM_STOP,
};

/* DOWN/UP/LEFT/RIGHT indicates the slide direction */
enum UI_VIEW_SLIDE_ANIMATION_TYPE {
	UI_ANIM_SLIDE_IN_DOWN = 1,
	UI_ANIM_SLIDE_IN_UP,
	UI_ANIM_SLIDE_IN_RIGHT,
	UI_ANIM_SLIDE_IN_LEFT,
	UI_ANIM_SLIDE_OUT_UP,
	UI_ANIM_SLIDE_OUT_DOWN,
	UI_ANIM_SLIDE_OUT_LEFT,
	UI_ANIM_SLIDE_OUT_RIGHT,
};

typedef struct ui_view_anim_cfg {
	ui_point_t start;
	ui_point_t stop;
	uint16_t duration;
	ui_view_anim_path_cb_t path_cb;
	ui_view_anim_stop_cb_t stop_cb;
} ui_view_anim_cfg_t;

typedef struct {
	uint8_t state;
	uint8_t is_slide : 1;

	uint16_t view_id;
	uint16_t last_view_id;
	ui_point_t last_view_offset;

	uint32_t start_time;
	uint16_t elapsed;

	ui_view_anim_cfg_t cfg;
} ui_view_animation_t;

/**
 * @brief get view data
 *
 * This routine get view data.
 *
 * @param view_id id of view
 *
 * @return view data on success else NULL
 */
view_data_t *view_get_data(uint16_t view_id);

/**
 * @brief get view display
 *
 * This routine get view display.
 *
 * @param data view data
 *
 * @return view display on success else NULL
 */
static inline void *view_get_display(view_data_t *data)
{
	return data->display;
}

/**
 * @brief get view surface
 *
 * This routine get view gui surface.
 *
 * @param data view data
 *
 * @return view surface on success else NULL
 */
static inline void *view_get_surface(view_data_t *data)
{
	return data->surface;
}

/**
 * @brief get view data
 *
 * This routine get view data.
 *
 * @param view_id id of view
 *
 * @return view data on success else NULL
 */
static inline const void *view_get_presenter(view_data_t *data)
{
	return data->presenter;
}

/**
 * @cond INTERNAL_HIDDEN
 */

/**
 * @brief enable view refresh to display or not
 *
 * This routine enable view refresh to display or not
 *
 * @param view_id id of view
 * @param enabled enable or not
 *
 * @retval 0 on succsess else negative errno code.
 */
int view_set_refresh_en(uint16_t view_id, bool enabled);

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @brief query view is hidden
 *
 * @param view_id id of view
 *
 * @retval the query result
 */
bool view_is_hidden(uint16_t view_id);

/**
 * @brief query view is visible (even partially) now or not
 *
 * A view to be visible, must not be hidden, and also partially at least on the screen.
 *
 * @param view_id id of view
 *
 * @retval the query result
 */
bool view_is_visible(uint16_t view_id);

/**
 * @brief Query view is focused or not
 *
 * @param view_id id of view
 *
 * @return query result.
 */
bool view_is_focused(uint16_t view_id);

/**
 * @brief Query view is focused or not
 *
 * @param view_id id of view
 *
 * @return query result.
 */
bool view_is_paused(uint16_t view_id);

/**
 * @brief Query view is layout-inflated or not
 *
 * @param view_id id of view
 *
 * @return query result.
 */
bool view_is_inflated(uint16_t view_id);

/**
 * @brief get x coord of position of view
 *
 * @param view_id id of view
 *
 * @retval x coord
 */
int16_t view_get_x(uint16_t view_id);

/**
 * @brief get y coord of position of view
 *
 * @param view_id id of view
 *
 * @retval y coord
 */
int16_t view_get_y(uint16_t view_id);

/**
 * @brief get position of view
 *
 * @param view_id id of view
 * @param x pointer to store x coordinate
 * @param y pointer to store y coordinate
 *
 * @retval 0 on success else negative code
 */
int view_get_pos(uint16_t view_id, int16_t *x, int16_t *y);

/**
 * @brief get width of view
 *
 * @param view_id id of view
 *
 * @retval width on success else negative code
 */
int16_t view_get_width(uint16_t view_id);

/**
 * @brief get height of view
 *
 * @param view_id id of view
 *
 * @retval height on success else negative code
 */
int16_t view_get_height(uint16_t view_id);

/**
 * @brief get position of view
 *
 * @param view_id id of view
 * @param x pointer to store x coordinate
 * @param y pointer to store y coordinate
 *
 * @retval 0 on success else negative code
 */
int view_get_region(uint16_t view_id, ui_region_t *region);

/**
 * @brief set position of view
 *
 * It will stop the gesture scrolling before setting position.
 *
 * @param view_id id of view
 * @param x new x coordinate
 * @param y new y coordinate
 *
 * @retval 0 on success else negative code
 */
int view_set_pos(uint16_t view_id, int16_t x, int16_t y);

/**
 * @brief set position of view during gesture scrolling
 *
 * @param view_id id of view
 * @param x new x coordinate
 * @param y new y coordinate
 *
 * @retval 0 on success else negative code
 */
int view_set_drag_pos(uint16_t view_id, int16_t x, int16_t y);

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
 * @retval 0 on succsess else negative errno code.
 */
int view_set_drag_attribute(uint16_t view_id, uint8_t drag_attribute, bool keep_pos);

/**
 * @brief get view drag attribute
 *
 * @param view_id id of view
 *
 * @retval drag attribute if view exist else negative code
 */
uint8_t view_get_drag_attribute(uint16_t view_id);

/**
 * @brief query view has move attribute (DRAG_ATTRIBUTE_MOVExx)
 *
 * @param view_id id of view
 *
 * @retval the query result
 */
bool view_has_move_attribute(uint16_t view_id);

/**
 * @brief set drag callback of view
 *
 * The callback will be called when the single dragged finished. During the callback,
 * the user can start the animation to some other position.
 *
 * @param view_id id of view
 * @param drag_cb callback for dragging one view, not involved in view switching
 *
 * @retval 0 on success else negative code
 */
int view_set_drag_anim_cb(uint16_t view_id, ui_view_drag_anim_cb_t drag_cb);

/**
 * @brief get diplay X resolution
 *
 * @retval X resolution in pixels
 */
int16_t view_manager_get_disp_xres(void);

/**
 * @brief get diplay Y resolution
 *
 * @retval Y resolution in pixels
 */
int16_t view_manager_get_disp_yres(void);

/**
 * @brief get dragged view
 *
 * This routine provide get dragged view.
 *
 * @param gesture gesture, see enum GESTURE_TYPE
 * @param towards_screen store the drag direction, whether dragged close to screen or away from screen.
 *
 * @retval view id.
 */
uint16_t view_manager_get_draggable_view(uint8_t gesture, bool *towards_screen);

/**
 * @brief get focused view
 *
 * This routine provide get focused view
 *
 * @retval view id of focused view
 */
uint16_t view_manager_get_focused_view(void);

/**
 * @brief dump the view informations to the console.
 *
 * @retval N/A.
 */
void view_manager_dump(void);

/**
 * @brief refocus view pre animation
 *
 * @param view_id the view that will be focused.
 *
 * @retval N/A
 */
void view_manager_pre_anim_refocus(uint16_t view_id);

/**
 * @brief refocus view post animation
 *
 * @retval N/A
 */
void view_manager_post_anim_refocus(void);

/**
 * @brief fill the start and stop points of the slide animation
 *
 * @param view_id the view to take the animation
 * @param cfg animation config
 * @param animation_type animation type, see  enum UI_VIEW_ANIMATION_TYPE
 *
 * @retval 0 on success else negative code.
 */
int view_manager_get_slide_animation_config(uint16_t view_id,
		ui_view_anim_cfg_t *cfg, uint8_t animation_type);

/**
 * @brief start the view animation on view switching
 *
 * @param view_id the view to take the animation
 * @param last_view_id the related view which will fade out if exists
 * @param animation_type animation type, see  enum UI_VIEW_ANIMATION_TYPE
 * @param cfg animation config
 *
 * @retval 0 on success else negative code.
 */
int view_manager_slide_animation_start(uint16_t view_id,
		uint16_t last_view_id, uint8_t animation_type, ui_view_anim_cfg_t *cfg);

/**
 * @brief fill the start and stop points of the drag animation
 *
 * @param view_id the view to take the animation
 * @param cfg animation config
 * @param runtime pointer to structure input_dev_runtime_t
 *
 * @retval 0 on success else negative code.
 */
int view_manager_get_drag_animation_config(uint16_t view_id,
		ui_view_anim_cfg_t *cfg, input_dev_runtime_t *runtime);

/**
 * @brief start the view animation caused on single long view dragging
 *
 * @param view_id the view to take the animation
 * @param cfg animation config
 *
 * @retval 0 on success else negative code.
 */
int view_manager_drag_animation_start(uint16_t view_id, ui_view_anim_cfg_t *cfg);

/**
 * @brief get current view id
 *
 * @param N/A
 *
 * @retval id id of current opertation view
 */
uint16_t view_manager_get_current_view_id(void);

/**
 * @brief get view entry
 *
 * @param view_id view id
 *
 * @retval pointer to view entry
 */
view_entry_t * view_manager_get_view_entry(uint16_t view_id);

#ifdef __cplusplus
}
#endif

/**
 * @} end defgroup system_apis
 */
#endif /* __VIEW_MANGER_H__ */
