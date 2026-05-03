#include "ui/views/View_Home.h"
#include "ui/UI_Style.h"
#include "StateService.h"

lv_obj_t* View_Home::label_summary = nullptr;

void View_Home::create(lv_obj_t* parent) {
    // Appliquer le fond de page
    lv_obj_set_style_bg_color(parent, UI_Style::current.bg, 0);

    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 600, 300);
    lv_obj_center(card);
    lv_obj_add_style(card, &UI_Style::style_card, 0); // Style Material

    lv_obj_t* title = lv_label_create(card);
    lv_label_set_text(title, "AIRSTREAM OS");
    lv_obj_add_style(title, &UI_Style::style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(title, UI_Style::current.text, 0); // Couleur thémée

    label_summary = lv_label_create(card);
    lv_obj_add_style(label_summary, &UI_Style::style_value, 0);
    lv_obj_set_style_text_color(label_summary, UI_Style::current.accent, 0); // Accent thémé[cite: 2]
    lv_obj_align(label_summary, LV_ALIGN_CENTER, 0, 20);
}

void View_Home::update() {
    if (!label_summary) return;

    Battery* sorted[MAX_BATTERIES];
    uint8_t n = StateService::instance().getBatteriesSorted(sorted, MAX_BATTERIES);
    if (n == 0) return;

    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f V", sorted[0]->get().voltage);
    lv_label_set_text(label_summary, buf);
}