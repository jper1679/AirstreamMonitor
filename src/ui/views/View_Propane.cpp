#include "ui/views/View_Propane.h"
#include "ui/UI_Style.h"
#include "StateService.h"

lv_obj_t* View_Propane::bar_propane_left = nullptr;
lv_obj_t* View_Propane::bar_propane_right = nullptr;
lv_obj_t* View_Propane::bar_propane_aux = nullptr;

void View_Propane::create(lv_obj_t* parent) {
    createPropaneVisual(parent, "PROPANE_LEFT", UI_Style::current.tank_propane_left, 650);
    createPropaneVisual(parent, "PROPANE_RIGHT", UI_Style::current.tank_propane_right, 850);
    createPropaneVisual(parent, "PROPANE_AUX", UI_Style::current.tank_propane_aux, 1050);
}

void View_Propane::createPropaneVisual(lv_obj_t* parent, const char* name, lv_color_t color, int x) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, name);
    lv_obj_set_pos(label, x + 20, 20);
    
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_size(bar, 60, 300);
    lv_obj_set_pos(bar, x, 50);
    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
    
    if (strcmp(name, "LEFT") == 0) bar_propane_left = bar;
    else if (strcmp(name, "RIGHT") == 0) bar_propane_right = bar;
    else if (strcmp(name, "AUX") == 0) bar_propane_aux = bar;
}

void View_Propane::update() {
    if (!bar_propane_left) return;

    Propane* sorted[MAX_PROPANE];
    uint8_t n = StateService::instance().getPropaneSorted(sorted, MAX_PROPANE);

    if (n > 0) lv_bar_set_value(bar_propane_left,  (int)sorted[0]->get().levelPercent, LV_ANIM_ON);
    if (n > 1) lv_bar_set_value(bar_propane_right, (int)sorted[1]->get().levelPercent, LV_ANIM_ON);
    if (n > 2) lv_bar_set_value(bar_propane_aux,   (int)sorted[2]->get().levelPercent, LV_ANIM_ON);
}