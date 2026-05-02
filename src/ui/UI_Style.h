#ifndef UI_STYLE_H
#define UI_STYLE_H

#include <lvgl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct ThemeColors {
    lv_color_t bg;
    lv_color_t tabs;
    lv_color_t card;
    lv_color_t border;
    lv_color_t text;
    lv_color_t text_dim;
    lv_color_t accent;
    
    lv_color_t tank_fresh;
    lv_color_t tank_grey;
    lv_color_t tank_black;
    lv_color_t tank_propane_left;
    lv_color_t tank_propane_right;
    lv_color_t tank_propane_aux;
};

class UI_Style {
public:
    static void init();
    static void applyTheme(bool dark); // ÉTAIT MANQUANT
    
    static lv_style_t style_card;
    static lv_style_t style_title;
    static lv_style_t style_value;

    static ThemeColors current;
    static SemaphoreHandle_t lvgl_mutex;
    static bool isDark; // ÉTAIT MANQUANT
};

#endif