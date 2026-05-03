#include "StorageService.h"

const char* StorageService::CONFIG_PATH = "/config.json";

StorageService::StorageService() {
    m_mutex = xSemaphoreCreateMutex();
}

bool StorageService::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println(F("[STORAGE] Erreur: impossible de monter LittleFS"));
        return false;
    }
    m_ready = true;
    Serial.println(F("[STORAGE] LittleFS monté."));
    return true;
}

bool StorageService::load() {
    if (!m_ready) return false;

    if (!LittleFS.exists(CONFIG_PATH)) {
        Serial.println(F("[STORAGE] Pas de config → valeurs par défaut"));
        return save();
    }

    File f = LittleFS.open(CONFIG_PATH, "r");
    if (!f) {
        Serial.println(F("[STORAGE] Erreur ouverture config.json"));
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.printf("[STORAGE] JSON invalide (%s) → reset\n", err.c_str());
        return reset();
    }

    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        deserialize(doc);
        xSemaphoreGive(m_mutex);
    }

    Serial.printf("[STORAGE] Config chargée — %u bat / %u tanks / %u propane / %u chargeurs\n",
        m_config.activeBatteries(), m_config.activeTanks(),
        m_config.activePropane(),   m_config.activeChargers());
    return true;
}

bool StorageService::save() {
    if (!m_ready) return false;

    File f = LittleFS.open(CONFIG_PATH, "w");
    if (!f) {
        Serial.println(F("[STORAGE] Erreur écriture config.json"));
        return false;
    }

    JsonDocument doc;
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        serialize(doc);
        xSemaphoreGive(m_mutex);
    }

    serializeJsonPretty(doc, f);
    f.close();
    Serial.println(F("[STORAGE] Config sauvegardée."));
    return true;
}

bool StorageService::reset() {
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        m_config = AppConfig{};
        xSemaphoreGive(m_mutex);
    }
    return save();
}

bool StorageService::lock(int timeout_ms) {
    return xSemaphoreTake(m_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void StorageService::unlock() {
    xSemaphoreGive(m_mutex);
}

// ─────────────────────────────────────────────
//  Sérialisation
// ─────────────────────────────────────────────

void StorageService::serialize(JsonDocument& doc) {
    doc["dark_mode"]      = m_config.dark_mode;
    doc["wifi"]["ssid"]   = m_config.wifi_ssid;
    doc["wifi"]["pass"]   = m_config.wifi_password;

    // Batteries
    JsonArray bArr = doc["batteries"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) {
        const BatterySlot& s = m_config.batteries[i];
        JsonObject o = bArr.add<JsonObject>();
        o["enabled"]     = s.enabled;
        o["ui_pos"]      = s.ui_position;
        o["name"]        = s.name;
        o["mac"]         = s.mac;
        o["capacity_Ah"] = s.capacity_Ah;
    }

    // Tanks eau
    JsonArray tArr = doc["tanks"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_TANKS; i++) {
        const TankSlot& s = m_config.tanks[i];
        JsonObject o = tArr.add<JsonObject>();
        o["enabled"]     = s.enabled;
        o["ui_pos"]      = s.ui_position;
        o["name"]        = s.name;
        o["adc_ch"]      = s.adc_channel;
        o["capacity_L"]  = s.capacity_L;
    }

    // Propane
    JsonArray pArr = doc["propane"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_PROPANE; i++) {
        const PropaneSlot& s = m_config.propane[i];
        JsonObject o = pArr.add<JsonObject>();
        o["enabled"]    = s.enabled;
        o["ui_pos"]     = s.ui_position;
        o["name"]       = s.name;
        o["mac"]        = s.mac;
        o["capacity_L"] = s.capacity_L;
    }

    // Chargeurs
    JsonArray cArr = doc["chargers"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_CHARGERS; i++) {
        const ChargerSlot& s = m_config.chargers[i];
        JsonObject o = cArr.add<JsonObject>();
        o["enabled"] = s.enabled;
        o["ui_pos"]  = s.ui_position;
        o["name"]    = s.name;
        o["mac"]     = s.mac;
        o["type"]    = static_cast<uint8_t>(s.type);
    }
}

// ─────────────────────────────────────────────
//  Désérialisation
// ─────────────────────────────────────────────

void StorageService::deserialize(JsonDocument& doc) {
    m_config.dark_mode = doc["dark_mode"] | true;
    strlcpy(m_config.wifi_ssid,      doc["wifi"]["ssid"] | "", sizeof(m_config.wifi_ssid));
    strlcpy(m_config.wifi_password,  doc["wifi"]["pass"] | "", sizeof(m_config.wifi_password));

    // Batteries
    m_config.battery_count = 0;
    JsonArray bArr = doc["batteries"].as<JsonArray>();
    for (uint8_t i = 0; i < MAX_BATTERIES && i < bArr.size(); i++) {
        BatterySlot& s  = m_config.batteries[i];
        JsonObject   o  = bArr[i].as<JsonObject>();
        s.enabled       = o["enabled"]     | false;
        s.ui_position   = o["ui_pos"]      | i;
        s.capacity_Ah   = o["capacity_Ah"] | 100;
        strlcpy(s.name, o["name"] | "", sizeof(s.name));
        strlcpy(s.mac,  o["mac"]  | "", sizeof(s.mac));
        if (s.enabled) m_config.battery_count++;
    }

    // Tanks eau
    m_config.tank_count = 0;
    JsonArray tArr = doc["tanks"].as<JsonArray>();
    for (uint8_t i = 0; i < MAX_TANKS && i < tArr.size(); i++) {
        TankSlot&  s  = m_config.tanks[i];
        JsonObject o  = tArr[i].as<JsonObject>();
        s.enabled     = o["enabled"]    | false;
        s.ui_position = o["ui_pos"]     | i;
        s.adc_channel = o["adc_ch"]     | i;
        s.capacity_L  = o["capacity_L"] | 100.0f;
        strlcpy(s.name, o["name"] | "", sizeof(s.name));
        if (s.enabled) m_config.tank_count++;
    }

    // Propane
    m_config.propane_count = 0;
    JsonArray pArr = doc["propane"].as<JsonArray>();
    for (uint8_t i = 0; i < MAX_PROPANE && i < pArr.size(); i++) {
        PropaneSlot& s  = m_config.propane[i];
        JsonObject   o  = pArr[i].as<JsonObject>();
        s.enabled       = o["enabled"]    | false;
        s.ui_position   = o["ui_pos"]     | i;
        s.capacity_L    = o["capacity_L"] | 20.0f;
        strlcpy(s.name, o["name"] | "", sizeof(s.name));
        strlcpy(s.mac,  o["mac"]  | "", sizeof(s.mac));
        if (s.enabled) m_config.propane_count++;
    }

    // Chargeurs
    m_config.charger_count = 0;
    JsonArray cArr = doc["chargers"].as<JsonArray>();
    for (uint8_t i = 0; i < MAX_CHARGERS && i < cArr.size(); i++) {
        ChargerSlot& s  = m_config.chargers[i];
        JsonObject   o  = cArr[i].as<JsonObject>();
        s.enabled       = o["enabled"] | false;
        s.ui_position   = o["ui_pos"]  | i;
        s.type          = static_cast<ChargerType>(o["type"] | 0);
        strlcpy(s.name, o["name"] | "", sizeof(s.name));
        strlcpy(s.mac,  o["mac"]  | "", sizeof(s.mac));
        if (s.enabled) m_config.charger_count++;
    }
}