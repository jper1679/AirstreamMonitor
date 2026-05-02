#ifndef HARDWAREDISPLAY_H
#define HARDWAREDISPLAY_H

#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <TAMC_GT911.h>

class HardwareDisplay {
public:
    virtual bool begin() = 0;
    virtual ESP_PanelLcd* getLcd() = 0;
    virtual ESP_PanelLcdTouch* getTouch() = 0;
};

class Waveshare5BDriver : public HardwareDisplay {
public:
    bool begin() override;
    ESP_PanelLcd* getLcd() override { return _lcd; }
    ESP_PanelLcdTouch* getTouch() override { return nullptr; }
    TAMC_GT911* getGT911() { return _ts; }

private:
    ESP_PanelBus_RGB* _bus = nullptr;
    ESP_PanelLcd* _lcd = nullptr;
    TAMC_GT911* _ts = nullptr;
};

#endif