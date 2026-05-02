#ifndef VIEW_PROPANE_H
#define VIEW_PROPANE_H

#include <lvgl.h>

class View_Propane { // Nom propre
public:
    static void create(lv_obj_t* parent);
    static void update();

private:
    static lv_obj_t* bar_propane_left;
    static lv_obj_t* bar_propane_right;
    static lv_obj_t* bar_propane_aux;
    
    static void createPropaneVisual(lv_obj_t* parent, const char* name, lv_color_t color, int x);
};

#endif