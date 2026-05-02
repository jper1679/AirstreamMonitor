#pragma once
#include "lvgl.h"
#include "ESP_Panel_Library.h"

#ifdef __cplusplus
extern "C" {
#endif

    bool lvgl_port_init(ESP_PanelLcd *lcd, ESP_PanelLcdTouch *tp);
    bool lvgl_port_lock(int timeout_ms);
    void lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif