#ifndef DATATYPES_H
#define DATATYPES_H

#include <Arduino.h>

// --- Status de validité (Le "Health Check" de tes composants) ---
enum DataValidity : uint8_t {
    DATA_INVALID = 0, // Pas de signal
    DATA_STALE   = 1, // Donnée ancienne (timeout)
    DATA_VALID   = 2  // Signal OK
};

struct ActuatorData {
    bool requestedState = false;
    bool actualState = false;
    bool fault = false;
};

// --- BOM : Batterie ---
struct BatteryData {
    float voltage = 0.0f;
    float current = 0.0f;
    float soc = 0.0f;      // State of Charge %
    float temp = 0.0f;
    uint32_t capacityAh = 100;
};

// --- BOM : Réservoir ---
struct TankData {
    float levelPercent = 0.0f;
    float volumeLiters = 0.0f;
    float temperature = 0.0f;
};

// --- BOM : Réservoir de Propane ---
struct PropaneData {
    float levelPercent = 0.0f;
    float volumeLiters = 0.0f;
    float temperature  = 0.0f;
    float batteryV     = 0.0f;  // Voltage pile du capteur Mopeka
};

// --- BOM : Chargeur (Solaire/DC-DC) ---
struct ChargerData {
    float inputVoltage = 0.0f;
    float outputVoltage = 0.0f;
    float outputCurrent = 0.0f;
    float powerWatts = 0.0f;
    bool isActive = false;
};

enum SourceType : uint8_t {
    SOURCE_NONE = 0,
    SOURCE_ADC  = 1,
    SOURCE_BLE  = 2,
    SOURCE_GPIO = 3  // Pour des capteurs de niveau haut/bas simples
};

#endif