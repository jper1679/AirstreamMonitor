#include "hardware/HardwareDisplay.h"
#include <Arduino.h>
#include <Wire.h>

extern SemaphoreHandle_t i2cMutex;

bool Waveshare5BDriver::begin() {
    Serial.println(F("[HAL] Initialisation Waveshare 5B RGB..."));

    // 1. Configuration des timings (Tes paramètres validés)
    static esp_lcd_rgb_panel_config_t rgb_config;
    memset(&rgb_config, 0, sizeof(rgb_config));
    
    rgb_config.clk_src = (lcd_clock_source_t)0;
    rgb_config.timings.pclk_hz = 16000000;
    rgb_config.timings.h_res = 1024;
    rgb_config.timings.v_res = 600;
    rgb_config.timings.hsync_pulse_width = 30;
    rgb_config.timings.hsync_back_porch = 145;
    rgb_config.timings.hsync_front_porch = 170;
    rgb_config.timings.vsync_pulse_width = 2;
    rgb_config.timings.vsync_back_porch = 20;
    rgb_config.timings.vsync_front_porch = 12;
    rgb_config.timings.flags.pclk_active_neg = 1;
    rgb_config.data_width = 16;
    
    // Pins DATA et CTRL
    rgb_config.pclk_gpio_num = 7;
    rgb_config.hsync_gpio_num = 46;
    rgb_config.vsync_gpio_num = 3;
    rgb_config.de_gpio_num = 5;
    rgb_config.disp_gpio_num = -1;
    
    int data_pins[] = {14, 38, 18, 17, 10, 39, 0, 45, 48, 47, 21, 1, 2, 42, 41, 40};
    for (int i = 0; i < 16; i++) rgb_config.data_gpio_nums[i] = data_pins[i];
    
    rgb_config.flags.fb_in_psram = 1; // Indispensable pour la fluidité en 1024x600

    // 2. Création du bus et du LCD
    _bus = new ESP_PanelBus_RGB(&rgb_config);
    _lcd = new ESP_PanelLcd_ST7262(_bus);
    _lcd->init();
    _lcd->reset();
    _lcd->begin();
    Serial.println(F("[HAL] LCD Initialisé"));

    // 3. Réveil du Backlight via I2C (CH422G)
    // On utilise le mutex pour éviter de clasher avec d'autres capteurs sur le bus
    /*if (i2cMutex != NULL && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        
        // Config du CH422G en mode sortie
        Wire.beginTransmission(0x24);
        Wire.write(0x01);
        Wire.endTransmission();

        // Allumage RST + DISP + BL (Backlight)
        // On le fait 2 fois pour être certain (ton ancienne méthode stable)
        for(int i=0; i<2; i++) {
            Wire.beginTransmission(0x38);
            Wire.write(0x0E);
            Wire.endTransmission();
            delay(10);
        }

        xSemaphoreGive(i2cMutex);
        Serial.println(F("[HAL] Backlight ON"));
    } else {
        Serial.println(F("[HAL] Erreur Mutex I2C - Backlight peut-être éteint"));
    }*/

    // 4. Initialisation du Tactile (GT911)
    _ts = new TAMC_GT911(8, 9, -1, -1, 1024, 600);
    _ts->begin();
    _ts->setRotation(ROTATION_NORMAL);
    Serial.println(F("[HAL] Tactile GT911 prêt."));

    return true;
}