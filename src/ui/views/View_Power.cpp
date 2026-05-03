#include "ui/views/View_Power.h"
#include "ui/UI_Style.h"
#include "StateService.h"

lv_obj_t* View_Power::label_voltage = nullptr;
lv_obj_t* View_Power::label_current = nullptr;
lv_obj_t* View_Power::arc_soc = nullptr;
lv_obj_t* View_Power::label_soc_text = nullptr;

void View_Power::create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, UI_Style::current.bg, 0);

    arc_soc = lv_arc_create(parent);
    lv_obj_set_size(arc_soc, 240, 240);
    lv_arc_set_rotation(arc_soc, 135);
    lv_arc_set_bg_angles(arc_soc, 0, 270);
    lv_obj_align(arc_soc, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_arc_color(arc_soc, UI_Style::current.accent, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_soc, UI_Style::current.border, LV_PART_MAIN);
    lv_obj_remove_style(arc_soc, NULL, LV_PART_KNOB);

    label_soc_text = lv_label_create(parent);
    lv_obj_set_style_text_color(label_soc_text, UI_Style::current.text, 0);
    lv_obj_set_style_text_font(label_soc_text, &lv_font_montserrat_40, 0);
    lv_obj_align_to(label_soc_text, arc_soc, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* info_cont = lv_obj_create(parent);
    lv_obj_set_size(info_cont, 600, 120);
    lv_obj_align(info_cont, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_style(info_cont, &UI_Style::style_card, 0);

    // Ajoutez ici la logique voltage/current simplifiée
}

void View_Power::update() {
    if (StateService::instance().batteryCount() == 0) return;

    Battery* sorted[MAX_BATTERIES];
    uint8_t n = StateService::instance().getBatteriesSorted(sorted, MAX_BATTERIES);
    if (n == 0) return;

    BatteryData data = sorted[0]->get();

    if (label_voltage) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2fV", data.voltage);
        lv_label_set_text(label_voltage, buf);
    }
    if (label_current) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1fA", data.current);
        lv_label_set_text(label_current, buf);
    }
    if (arc_soc && label_soc_text) {
        lv_arc_set_value(arc_soc, (int)data.soc);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", (int)data.soc);
        lv_label_set_text(label_soc_text, buf);
    }
}