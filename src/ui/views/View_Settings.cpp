#include "View_Settings.h"
#include "../UI_Style.h"
#include "../UI_Engine.h"
#include <Arduino.h>

void View_Settings::create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, UI_Style::current.bg, 0);

    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 400, 80);
    lv_obj_center(card);
    lv_obj_add_style(card, &UI_Style::style_card, 0);

    lv_obj_t* label = lv_label_create(card);
    lv_label_set_text(label, "MODE SOMBRE");
    lv_obj_add_style(label, &UI_Style::style_title, 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t* sw = lv_switch_create(card);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(sw, UI_Style::current.accent, LV_PART_INDICATOR | LV_STATE_CHECKED);
    
    // État actuel
    if (UI_Style::isDark) lv_obj_add_state(sw, LV_STATE_CHECKED);

    lv_obj_add_event_cb(sw, theme_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void View_Settings::theme_event_cb(lv_event_t* e) {
    lv_obj_t* sw = lv_event_get_target(e);
    bool dark_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);

    if (xSemaphoreTakeRecursive(UI_Style::lvgl_mutex, pdMS_TO_TICKS(100))) {
        UI_Style::applyTheme(dark_mode);
        lv_obj_report_style_change(NULL); 
        lv_obj_invalidate(lv_scr_act()); 
        xSemaphoreGiveRecursive(UI_Style::lvgl_mutex);
    }
}

void View_Settings::update() {
    // Si on avait des éléments dynamiques, on les mettrait à jour ici
}