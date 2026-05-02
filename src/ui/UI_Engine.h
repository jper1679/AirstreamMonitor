#ifndef UI_ENGINE_H
#define UI_ENGINE_H

#include "Hardware/HardwareDisplay.h"
#include "ui/lvgl_v8_port.h"

class UI_Engine {
public:
    static SemaphoreHandle_t lvgl_mutex;
    
    static UI_Engine& instance() {
        static UI_Engine inst;
        return inst;
    }
    static void guiTask(void * pvParameters);
    void begin(HardwareDisplay* driver);
    void switchToMainScreen();
    static void update();
    
private:
    UI_Engine();
    void setupSplashScreen();
    
    static lv_obj_t* tabview;
    static lv_obj_t* tab_home;
    static lv_obj_t* tab_tanks;
    static lv_obj_t* tab_propane;
    static lv_obj_t* tab_power;
    static lv_obj_t* tab_settings;
    
    void setupTabs();

};

#endif