#ifndef VIEW_HOME_H
#define VIEW_HOME_H

#include <lvgl.h>

class View_Home {
public:
    static void create(lv_obj_t* parent);
    static void update();
private:
    static lv_obj_t* label_summary;
};

#endif