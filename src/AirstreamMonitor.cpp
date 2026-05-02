#include <Arduino.h>
#include <SPI.h>  // Force l'inclusion pour le compilateur
#include <Wire.h> // Force l'inclusion pour le compilateur
#include "ui/UI_Engine.h"
#include "hardware/HardwareDisplay.h"
#include "hardware/BluetoothService.h"
#include "StateService.h"

SemaphoreHandle_t i2cMutex = NULL;
BluetoothService bleService;

// Simulation d'une tâche de fond (Core 0) qui lit les capteurs
void taskHardwareSim(void * pvParameters) {
    while(1) {
        // Simuler la lecture d'une batterie LiTime
        BatteryData fakeBattery;
        fakeBattery.voltage = 13.2f + ((float)random(0, 50) / 100.0f); // Fluctuation
        fakeBattery.current = -2.5f;
        fakeBattery.soc = 98.0f;
        
        // On injecte dans le StateService
        if (!StateService::instance().batteries.empty()) {
            StateService::instance().batteries[0]->set(fakeBattery);
        }

        // Simuler la lecture d'un réservoir ADC
        if (!StateService::instance().tanks.empty()) {
            // On fait monter l'eau lentement pour voir le changement
            static float level = 0.0f;
            level = (level >= 100.0f) ? 0.0f : level + 0.5f;
            StateService::instance().tanks[0]->updateLevel(level);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Lecture aux 500ms
    }
}

void setup() {
    i2cMutex = xSemaphoreCreateMutex();

    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== AIRSTREAM OS : BOOT SEQUENCE ===");

    // 1. On initialise l'I2C pour le CH422G (Backlight)
    Serial.println(F("[SETUP] Initialisation de l'I2C pour le Backlight..."));
    Wire.begin(8, 9, 100000);
    Wire.setTimeOut(50);

    // 2. On instancie le driver spécifique à ton Waveshare 5B
    //Serial.println(F("[SETUP] Initialisation du driver d'affichage..."));
    //Waveshare5BDriver* driver = new Waveshare5BDriver();

    // 3. On lance le DisplayManager avec ce driver
    // C'est ici qu'on passe l'argument manquant !
    //Serial.println(F("[SETUP] Démarrage du DisplayManager..."));
    //UI_Engine::instance().begin(driver);

    // 4. On démarre le BLE
    bleService.begin();

    // --- CONFIGURATION DYNAMIQUE (Simule le chargement du JSON) ---
    Serial.println("[SETUP] Initialisation des composants...");
    
    // Création des capteur Mopeka
    
    #define USE_MOPEKA_L 0
    #define USE_MOPEKA_R 1
    #define USE_MOPEKA_AUX 0
    #define MOPEKA_MAC_L  "XX:XX:XX:XX:XX:L1"
    #define MOPEKA_MAC_R  "D4:FB:45:CA:03:A5"
    #define MOPEKA_MAC_AUX  "D4:FB:45:CA:03:A5"

    // On crée une batterie "House" (Source BLE)
    Serial.println("[SETUP] Création de la batterie...");
    Battery* b = new Battery("LITIME_01", "House Battery", 200, SOURCE_BLE);
    StateService::instance().registerBattery(b);

    // On crée un réservoir "Fresh Water" (Source ADC)
    Serial.println("[SETUP] Création des réservoirs...");
    for (int i = 0; i < 3; i++) {
        char id[16];
        char name[16];
        snprintf(id, sizeof(id), "ADC_CH%d", i);
        snprintf(name, sizeof(name), "Tank %d", i+1);
        Tank* t = new Tank(id, name, 100, SOURCE_ADC);
        StateService::instance().registerTank(t);
    }

    // --- DÉMARRAGE DU CORE 0 (Hardware) ---
    Serial.println("[SETUP] Démarrage de la tâche de simulation hardware...");
    xTaskCreatePinnedToCore(taskHardwareSim, "HW_SIM", 4096, NULL, 1, NULL, 0);

    //UI_Engine::instance().switchToMainScreen();
    Serial.println("[SETUP] Système prêt.");
}

void loop() {
    // --- SIMULATION DE L'UI (Core 1) ---
    Serial.println("\n--- DASHBOARD ---");

    // Affichage des batteries
    for (auto b : StateService::instance().batteries) {
        BatteryData data = b->get();
        Serial.printf("%s: %.2fV | %.1f%% | %s\n", 
            b->getName(), data.voltage, data.soc, 
            (b->getStatus() == DATA_VALID ? "OK" : "TIMEOUT"));
    }

    // Affichage des réservoirs
    for (auto t : StateService::instance().tanks) {
        TankData data = t->get();
        Serial.printf("%s: %.1f%% (%.1f L)\n", 
            t->getName(), data.levelPercent, data.volumeLiters);
    }

    // Test de la Pompe
    ActuatorData pump = StateService::instance().waterPump.get();
    Serial.printf("POMPE EAU: %s\n", pump.requestedState ? "ON" : "OFF");

    // Simulation d'une interaction utilisateur (on toggle la pompe toutes les 10s)
    if (millis() % 10000 < 1000) {
        Serial.println(">>> UTILISATEUR: Appuie sur bouton POMPE");
        StateService::instance().waterPump.toggle();
    }

    delay(1000);
}