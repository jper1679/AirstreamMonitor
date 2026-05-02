#ifndef VIEW_POWER_H
#define VIEW_POWER_H

#include <lvgl.h>
#include "StateService.h"

class View_Power {
public:
    static void create(lv_obj_t* parent);
    static void update();

private:
    static lv_obj_t* label_voltage;
    static lv_obj_t* label_current;
    static lv_obj_t* arc_soc;
    static lv_obj_t* label_soc_text;
    
    static void createPowerCard(lv_obj_t* parent, const char* title, int y_pos);
};

#endif