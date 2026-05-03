#include <Arduino.h>
#include <SPI.h>  // Force l'inclusion pour le compilateur
#include <Wire.h> // Force l'inclusion pour le compilateur
#include "ui/UI_Engine.h"
#include "hardware/HardwareDisplay.h"
#include "hardware/BluetoothService.h"
#include "StateService.h"
#include "StorageService.h"

SemaphoreHandle_t i2cMutex = NULL;
BluetoothService bleService;

// Simulation d'une tâche de fond (Core 0) qui lit les capteurs
void taskHardwareSim(void * pvParameters) {
    while(1) {
        BatteryData fakeBattery;
        fakeBattery.voltage = 13.2f + ((float)random(0, 50) / 100.0f);
        fakeBattery.current = -2.5f;
        fakeBattery.soc     = 98.0f;

        Battery* b = StateService::instance().batteries[0];
        if (b) b->set(fakeBattery);

        static float level = 0.0f;
        level = (level >= 100.0f) ? 0.0f : level + 0.5f;

        Tank* t = StateService::instance().tanks[0];
        if (t) {
            TankData td;
            td.levelPercent = level;
            td.volumeLiters = (level / 100.0f) * 100.0f;
            td.temperature  = 0.0f;
            t->set(td);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void setup() {
    i2cMutex = xSemaphoreCreateMutex();
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n=== AIRSTREAM OS : BOOT SEQUENCE ===");

    // On initialise l'I2C pour le CH422G (Backlight)
    Serial.println(F("[SETUP] Initialisation de l'I2C pour le Backlight..."));
    Wire.begin(8, 9, 100000);
    Wire.setTimeOut(50);

    // On démarre le BLE
    bleService.begin();

    // On instancie le driver spécifique à ton Waveshare 5B
    Serial.println(F("[SETUP] Initialisation du driver d'affichage..."));
    Waveshare5BDriver* driver = new Waveshare5BDriver();

    // On lance le DisplayManager avec ce driver
    // C'est ici qu'on passe l'argument manquant !
    Serial.println(F("[SETUP] Démarrage du DisplayManager..."));
    UI_Engine::instance().begin(driver);

    StorageService::instance().begin();
    StorageService::instance().load();
    AppConfig& cfg = StorageService::instance().config();

    

    Serial.println("[SETUP] Initialisation des composants...");
    // Instancier uniquement les slots activés, dans l'ordre ui_position
    // (tri optionnel si l'ordre UI diverge de l'ordre des slots)
    
    // Batteries LiTime
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) {
        auto& s = cfg.batteries[i];
        if (!s.enabled) continue;
        Battery* b = new Battery(s.name, s.name, s.capacity_Ah, SOURCE_BLE);
        StateService::instance().registerBattery(i, b);
    }

    // Réservoirs d'eau (ADC)
    for (uint8_t i = 0; i < MAX_TANKS; i++) {
        auto& s = cfg.tanks[i];
        if (!s.enabled) continue;
        Tank* t = new Tank(s.name, s.name, s.capacity_L, SOURCE_ADC);
        StateService::instance().registerTank(i, t);
    }

    // Réservoirs de propane (Mopeka)
    for (uint8_t i = 0; i < MAX_PROPANE; i++) {
        auto& s = cfg.propane[i];
        if (!s.enabled) continue;
        Propane* p = new Propane(s.name, s.name, s.capacity_L, SOURCE_BLE);
        StateService::instance().registerPropane(i, p);
    }

    // Chargeurs (Victron)
    for (uint8_t i = 0; i < MAX_CHARGERS; i++) {
        auto& s = cfg.chargers[i];
        if (!s.enabled) continue;
        Charger* c = new Charger(s.name, s.name, SOURCE_BLE);
        StateService::instance().registerCharger(i, c);
    }




    // --- DÉMARRAGE DU CORE 0 (Hardware) ---
    Serial.println("[SETUP] Démarrage de la tâche de simulation hardware...");
    xTaskCreatePinnedToCore(taskHardwareSim, "HW_SIM", 4096, NULL, 1, NULL, 0);

    //UI_Engine::instance().switchToMainScreen();
    Serial.println("[SETUP] Système prêt.");
}

void loop() {
    Serial.println("\n--- DASHBOARD ---");

    // Batteries
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) {
        Battery* b = StateService::instance().batteries[i];
        if (!b) continue;
        BatteryData data = b->get();
        Serial.printf("Battery[%u] %s: %.2fV | %.1f%% | %s\n",
            i, b->getName(), data.voltage, data.soc,
            b->getStatus() == DATA_VALID ? "OK" : "TIMEOUT");
    }

    // Tanks eau
    for (uint8_t i = 0; i < MAX_TANKS; i++) {
        Tank* t = StateService::instance().tanks[i];
        if (!t) continue;
        TankData data = t->get();
        Serial.printf("Tank[%u] %s: %.1f%% (%.1fL)\n",
            i, t->getName(), data.levelPercent, data.volumeLiters);
    }

    // Propane
    for (uint8_t i = 0; i < MAX_PROPANE; i++) {
        Propane* p = StateService::instance().propane_tanks[i];
        if (!p) continue;
        PropaneData data = p->get();
        Serial.printf("Propane[%u] %s: %.1f%% (%.1fL)\n",
            i, t->getName(), data.levelPercent, data.volumeLiters);
    }

    // Pompe
    ActuatorData pump = StateService::instance().waterPump.get();
    Serial.printf("POMPE EAU: %s\n", pump.requestedState ? "ON" : "OFF");

    if (millis() % 10000 < 1000) {
        Serial.println(">>> UTILISATEUR: Appuie sur bouton POMPE");
        StateService::instance().waterPump.toggle();
    }

    delay(1000);
}