#include <Arduino.h>
#include <Wire.h>
#include "ADS1115_Native.h"
#include "models/Tank.h"
#include "StateService.h"
#include "StorageService.h"

ADS1115_Native adc;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== AIRSTREAM OS : ADC TEST ===");

    // Initialize I2C and ADC
    Wire.begin(8, 9, 100000);
    if (!adc.begin(&Wire)) {
        Serial.println("[ADC] Failed to initialize ADS1115!");
    } else {
        Serial.println("[ADC] ADS1115 initialized successfully.");
    }

    // Configure fake tank for testing
    AppConfig& cfg = StorageService::instance().config();
    cfg.tanks[0].enabled = true;
    cfg.tanks[0].adc_channel = 0;
    strncpy(cfg.tanks[0].name, "Fresh Water", 16);
    cfg.tanks[0].capacity_L = 100.0f;

    // Register Tank in StateService
    Tank* t = new Tank("tank0", cfg.tanks[0].name, cfg.tanks[0].capacity_L, SOURCE_ADC);
    StateService::instance().registerTank(0, t);

    Serial.println("[ADC] Setup complete. Starting reads...");
}

void loop() {
    // Read raw value from ADS1115 channel 0
    int16_t raw_val = adc.readADC_SingleEnded(0);
    
    // Simulate mapping to percentage (assuming 0-26000 range for test)
    // You will need to calibrate this mapping based on your actual sensor
    float percent = (raw_val / 26000.0f) * 100.0f;
    if (percent > 100.0f) percent = 100.0f;
    if (percent < 0.0f) percent = 0.0f;

    // Update the domain model
    Tank* tank = StateService::instance().tanks[0];
    if (tank) {
        tank->updateLevel(percent);
    }

    // Read back from StateService to verify
    TankData data = tank->get();
    
    Serial.printf("[ADC] Raw: %6d | Mapped: %5.1f%% | Model Vol: %5.1fL\n", 
                  raw_val, percent, data.volumeLiters);
                  
    delay(1000);
}
