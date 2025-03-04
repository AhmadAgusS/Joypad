cd "..\Output\Objects\Debug\LVGL.libdisplay\Win32"
lib /OUT:libdisplay.lib bitmap_font_api.obj lz4.obj spress.obj lvgl_bitmap_font.obj lvgl_input_dev.obj lvgl_view.obj lvgl_virtual_display.obj graphic_buffer.obj ui_surface.obj gesture_manager.obj input_dispatcher.obj input_recorder.obj input_recorder_buffer.obj input_recorder_slide_fixedstep.obj input_recorder_stream.obj ui_service.obj view_animation.obj view_manager.obj view_manager_gui.obj libdisplay_version.obj
xcopy libdisplay.lib "..\..\..\..\..\libs\debug_win32\" /Y
echo "gen libdisplay done"