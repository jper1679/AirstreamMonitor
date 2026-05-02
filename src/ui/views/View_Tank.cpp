#include "ui/views/View_Tanks.h"
#include "ui/UI_Style.h"
#include "StateService.h"

lv_obj_t* View_Tanks::bar_fresh = nullptr;
lv_obj_t* View_Tanks::bar_grey = nullptr;
lv_obj_t* View_Tanks::bar_black = nullptr;

void View_Tanks::create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, UI_Style::current.bg, 0);
    
    // Organisation automatique en ligne
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    createTankVisual(parent, "FRESH", UI_Style::current.accent);
    createTankVisual(parent, "GREY", lv_color_hex(0x757575));
    createTankVisual(parent, "BLACK", lv_color_hex(0x212121));
}

void View_Tanks::createTankVisual(lv_obj_t* parent, const char* name, lv_color_t color) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 150, 380);
    lv_obj_add_style(card, &UI_Style::style_card, 0);

    lv_obj_t* label = lv_label_create(card);
    lv_label_set_text(label, name);
    lv_obj_add_style(label, &UI_Style::style_title, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_t* bar = lv_bar_create(card);
    lv_obj_set_size(bar, 40, 280);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
    
    if (strcmp(name, "FRESH") == 0) bar_fresh = bar;
    else if (strcmp(name, "GREY") == 0) bar_grey = bar;
    else if (strcmp(name, "BLACK") == 0) bar_black = bar;
}

void View_Tanks::update() {
    if (!bar_fresh) return;
    auto tanks = StateService::instance().tanks;
    if (tanks.size() >= 3) {
        lv_bar_set_value(bar_fresh, (int)tanks[0]->get().levelPercent, LV_ANIM_ON);
        lv_bar_set_value(bar_grey, (int)tanks[1]->get().levelPercent, LV_ANIM_ON);
        lv_bar_set_value(bar_black, (int)tanks[2]->get().levelPercent, LV_ANIM_ON);
    }
}