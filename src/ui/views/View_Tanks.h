#ifndef VIEW_TANKS_H
#define VIEW_TANKS_H

#include <lvgl.h>

class View_Tanks { // Nom propre
public:
    static void create(lv_obj_t* parent);
    static void update();

private:
    static void createTankVisual(lv_obj_t* parent, const char* name, lv_color_t color);
    static lv_obj_t* bar_fresh;
    static lv_obj_t* bar_grey;
    static lv_obj_t* bar_black;
};

#endif