#include "UI_Style.h"

SemaphoreHandle_t UI_Style::lvgl_mutex = NULL;

ThemeColors UI_Style::current;
lv_style_t UI_Style::style_card;
lv_style_t UI_Style::style_title;
lv_style_t UI_Style::style_value;
bool UI_Style::isDark = true;

void UI_Style::applyTheme(bool dark) {
    isDark = dark;
    if (dark) { // VRAI MODE SOMBRE
        current.bg       = lv_color_hex(0x121212); // Fond très sombre

        current.card     = lv_color_hex(0x1E1E1E); // Cartes légèrement plus claires
        current.border   = lv_color_hex(0x333333);
        current.text     = lv_color_hex(0xFFFFFF);
        current.text_dim = lv_color_hex(0x888888);
        current.accent   = lv_color_hex(0x03DAC6); // Teal Material
    } else { // VRAI MODE CLAIR
        current.bg       = lv_color_hex(0xF0F2F5);
        
        current.card     = lv_color_hex(0xFFFFFF);
        current.border   = lv_color_hex(0xD1D5DB);
        current.text     = lv_color_hex(0x111827);
        current.text_dim = lv_color_hex(0x6B7280);
        current.accent   = lv_color_hex(0x2563EB); // Bleu Royal
    }
}

void UI_Style::init() {
    if (lvgl_mutex == NULL) {
        lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    }
    applyTheme(true); // Maintenant déclaré dans le .h, donc ça va marcher
    
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, current.card); // Utilise 'card'
    lv_style_set_radius(&style_card, 12);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, current.border);

    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, current.text);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_18);

    lv_style_init(&style_value);
    lv_style_set_text_color(&style_value, current.accent);
    lv_style_set_text_font(&style_value, &lv_font_montserrat_24);
}