#include "ui/UI_Engine.h"
#include <Arduino.h>
#include "ui/UI_Style.h"
#include "ui/lvgl_v8_port.h"
#include "ui/views/View_Home.h"
#include "ui/views/View_Tanks.h"
#include "ui/views/View_Power.h"
#include "ui/views/View_Propane.h"
#include "ui/views/View_Settings.h"

static TAMC_GT911* touch_ptr = nullptr;
extern SemaphoreHandle_t i2cMutex;
SemaphoreHandle_t UI_Engine::lvgl_mutex = NULL;

lv_obj_t* UI_Engine::tabview = nullptr;
lv_obj_t* UI_Engine::tab_home = nullptr;
lv_obj_t* UI_Engine::tab_tanks = nullptr;
lv_obj_t* UI_Engine::tab_propane = nullptr;
lv_obj_t* UI_Engine::tab_power = nullptr;
lv_obj_t* UI_Engine::tab_settings = nullptr;

UI_Engine::UI_Engine() {
    // Laisser vide ou initialiser des variables non-statiques ici
}

void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    if (!touch_ptr) return;

    if (xSemaphoreTake(i2cMutex, 0) == pdTRUE) {
        touch_ptr->read();
        if (touch_ptr->isTouched) {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = 1024 - touch_ptr->points[0].x;
            data->point.y = touch_ptr->points[0].y;
        } else {
            data->state = LV_INDEV_STATE_REL;
        }
        xSemaphoreGive(i2cMutex);
    }
}

void UI_Engine::begin(HardwareDisplay* driver) {
    if (lvgl_mutex == NULL) {
        lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    }
    if (!driver || !driver->begin()) {
        Serial.println(F("[DISPLAY] Échec du driver Hardware"));
        return;
    }

    // On récupère notre instance GT911 du driver
    touch_ptr = ((Waveshare5BDriver*)driver)->getGT911();

    // Init LVGL Port (Affichage)
    lvgl_port_init(driver->getLcd(), nullptr); 

    // Ajout manuel du tactile
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    UI_Style::init();

    setupSplashScreen();
    xTaskCreatePinnedToCore(guiTask, "gui_task", 8192, NULL, 1, NULL, 1);
}

void UI_Engine::switchToMainScreen(){
    Serial.println("[UI] Switching to Main Screen.");
    
    // VERROUILLE LVGL pendant la création des objets
    if (xSemaphoreTakeRecursive(UI_Engine::lvgl_mutex, portMAX_DELAY)) {
        setupTabs();
        xSemaphoreGiveRecursive(UI_Engine::lvgl_mutex);
    }
}

void UI_Engine::setupSplashScreen() {
    lv_obj_t * screen = lv_scr_act();
    
    // Un look un peu plus "Airstream" (Gris foncé/noir)
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);

    // Label de test
    lv_obj_t * label = lv_label_create(screen);
    lv_label_set_text(label, "AIRSTREAM OS - V1.0");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
}

void UI_Engine::guiTask(void *pvParameters) {
    while (1) {
        if (xSemaphoreTakeRecursive(UI_Engine::lvgl_mutex, portMAX_DELAY)) {
            lv_timer_handler(); 
            xSemaphoreGiveRecursive(UI_Engine::lvgl_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void UI_Engine::setupTabs() {
    // 1. Création (On s'assure que lv_scr_act() est prêt)
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 60);
    if (!tabview) return; // Sécurité anti-crash

    // 2. Configuration du contenu (Zone de swipe)
    lv_obj_t* tab_cont = lv_tabview_get_content(tabview);
    if (tab_cont) {
        lv_obj_clear_flag(tab_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(tab_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
        
        // On remet le callback DIRECTEMENT sur le tab_cont
        lv_obj_add_event_cb(tab_cont, [](lv_event_t * e) {
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
            if (dir == LV_DIR_NONE) return;

            lv_obj_t * tv = (lv_obj_t *)lv_event_get_user_data(e);
            uint16_t act = lv_tabview_get_tab_act(tv);
            const uint16_t max_tabs = 5;

            if(dir == LV_DIR_LEFT && act < (max_tabs - 1)) lv_tabview_set_act(tv, act + 1, LV_ANIM_ON);
            else if(dir == LV_DIR_RIGHT && act > 0) lv_tabview_set_act(tv, act - 1, LV_ANIM_ON);
        }, LV_EVENT_GESTURE, tabview); // On passe tabview en user_data pour éviter de le chercher
    }

    // 3. Styles de base (Sans fioritures pour tester la stabilité)
    lv_obj_set_style_bg_color(tabview, UI_Style::current.bg, 0);
    lv_obj_set_style_anim_time(tabview, 200, 0);

    // 4. Ajout des onglets
    tab_home     = lv_tabview_add_tab(tabview, "Home");
    tab_tanks    = lv_tabview_add_tab(tabview, "Tanks");
    tab_propane  = lv_tabview_add_tab(tabview, "Propane");
    tab_power    = lv_tabview_add_tab(tabview, "Power");
    tab_settings = lv_tabview_add_tab(tabview, "Settings");

    // 5. Création des vues (Vérifie que tes classes View ne crash pas en interne)
    if(tab_home)    View_Home::create(tab_home);
    if(tab_tanks)   View_Tanks::create(tab_tanks);
    if(tab_propane) View_Propane::create(tab_propane);
    if(tab_power)   View_Power::create(tab_power);
    if(tab_settings) View_Settings::create(tab_settings);
}

void UI_Engine::update() {
    if (tabview == nullptr) return;
    
    uint16_t active_tab = lv_tabview_get_tab_act(tabview);
    
    // On ne rafraîchit que ce qui est à l'écran
    switch(active_tab) {
        case 0: View_Home::update(); break;
        case 1: View_Tanks::update(); break;
        case 2: View_Propane::update(); break;
        case 3: View_Power::update(); break;
        case 4: View_Settings::update(); break;
    }
}

