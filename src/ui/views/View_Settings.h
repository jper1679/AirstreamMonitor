#ifndef VIEW_SETTINGS_H
#define VIEW_SETTINGS_H

#include <lvgl.h>

class UI_Engine;

class View_Settings {
public:
    static void create(lv_obj_t* parent);
    static void update();
private:
    static void theme_event_cb(lv_event_t* e);
};

#endif