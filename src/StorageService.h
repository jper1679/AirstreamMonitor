#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "models/DataTypes.h"

// ─────────────────────────────────────────────
//  Limites matérielles — modifier ici seulement
// ─────────────────────────────────────────────
static constexpr uint8_t MAX_BATTERIES = 4;
static constexpr uint8_t MAX_TANKS     = 4;
static constexpr uint8_t MAX_PROPANE   = 3;
static constexpr uint8_t MAX_CHARGERS  = 4;

// ─────────────────────────────────────────────
//  Structs de configuration par slot
// ─────────────────────────────────────────────

struct BatterySlot {
    bool     enabled       = false;
    uint8_t  ui_position   = 0;         // Ordre d'affichage (0 = premier)
    char     name[16]      = "";
    char     mac[18]       = "";         // Adresse BLE
    uint32_t capacity_Ah   = 100;
};

struct TankSlot {
    bool     enabled          = false;
    uint8_t  ui_position      = 0;
    char     name[16]         = "";
    uint8_t  adc_channel      = 0;       // Canal physique ADS1115 (0–3)
    float    capacity_L       = 100.0f;
};

struct PropaneSlot {
    bool     enabled          = false;
    uint8_t  ui_position      = 0;
    char     name[16]         = "";
    char     mac[18]          = "";      // Adresse BLE Mopeka
    float    capacity_L       = 20.0f;
};

enum class ChargerType : uint8_t {
    UNKNOWN   = 0,
    ORION_TR  = 1,   // Victron Orion Smart Tr  (BLE passif)
    MULTIPLUS = 2,   // Victron Multiplus II     (BLE passif)
    MPPT      = 3,   // Victron SmartSolar MPPT  (BLE passif)
};

struct ChargerSlot {
    bool        enabled       = false;
    uint8_t     ui_position   = 0;
    char        name[24]      = "";
    char        mac[18]       = "";
    ChargerType type          = ChargerType::UNKNOWN;
};

// ─────────────────────────────────────────────
//  Config globale
// ─────────────────────────────────────────────

struct AppConfig {
    // UI
    bool dark_mode = true;

    // Réseau
    char wifi_ssid[64]     = "";
    char wifi_password[64] = "";

    // Slots (arrays de taille fixe, count = nombre actifs)
    uint8_t     battery_count = 0;
    BatterySlot batteries[MAX_BATTERIES];

    uint8_t     tank_count = 0;
    TankSlot    tanks[MAX_TANKS];

    uint8_t     propane_count = 0;
    PropaneSlot propane[MAX_PROPANE];

    uint8_t     charger_count = 0;
    ChargerSlot chargers[MAX_CHARGERS];

    // Helpers — retournent le nombre de slots actifs
    uint8_t activeBatteries() const {
        uint8_t n = 0;
        for (uint8_t i = 0; i < MAX_BATTERIES; i++) if (batteries[i].enabled) n++;
        return n;
    }
    uint8_t activeTanks() const {
        uint8_t n = 0;
        for (uint8_t i = 0; i < MAX_TANKS; i++) if (tanks[i].enabled) n++;
        return n;
    }
    uint8_t activePropane() const {
        uint8_t n = 0;
        for (uint8_t i = 0; i < MAX_PROPANE; i++) if (propane[i].enabled) n++;
        return n;
    }
    uint8_t activeChargers() const {
        uint8_t n = 0;
        for (uint8_t i = 0; i < MAX_CHARGERS; i++) if (chargers[i].enabled) n++;
        return n;
    }
};

// ─────────────────────────────────────────────
//  Service
// ─────────────────────────────────────────────

class StorageService {
public:
    static StorageService& instance() {
        static StorageService inst;
        return inst;
    }

    bool begin();
    bool load();
    bool save();
    bool reset();

    AppConfig& config() { return m_config; }

    // Pour modifications depuis l'UI :  lock() → modifie → unlock() → save()
    bool lock(int timeout_ms = 200);
    void unlock();

private:
    StorageService();

    static const char* CONFIG_PATH;
    AppConfig          m_config;
    SemaphoreHandle_t  m_mutex;
    bool               m_ready = false;

    void serialize(JsonDocument& doc);
    void deserialize(JsonDocument& doc);
};

#endif