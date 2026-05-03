#include "StateService.h"
#include "StorageService.h"
#include <Arduino.h>

// ── Destructeur ───────────────────────────────────────────────────

StateService::~StateService() {
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) { delete batteries[i];     batteries[i]     = nullptr; }
    for (uint8_t i = 0; i < MAX_TANKS;     i++) { delete tanks[i];         tanks[i]         = nullptr; }
    for (uint8_t i = 0; i < MAX_PROPANE;   i++) { delete propane_tanks[i]; propane_tanks[i] = nullptr; }
    for (uint8_t i = 0; i < MAX_CHARGERS;  i++) { delete chargers[i];      chargers[i]      = nullptr; }
}

// ── Enregistrement ────────────────────────────────────────────────

bool StateService::registerBattery(uint8_t slot, Battery* b) {
    if (slot >= MAX_BATTERIES || b == nullptr) return false;
    if (batteries[slot] != nullptr) {
        Serial.printf("[STATE] WARN: slot batterie %u déjà occupé, remplacement\n", slot);
        delete batteries[slot];
    }
    batteries[slot] = b;
    return true;
}

bool StateService::registerTank(uint8_t slot, Tank* t) {
    if (slot >= MAX_TANKS || t == nullptr) return false;
    if (tanks[slot] != nullptr) {
        Serial.printf("[STATE] WARN: slot tank %u déjà occupé, remplacement\n", slot);
        delete tanks[slot];
    }
    tanks[slot] = t;
    return true;
}

bool StateService::registerPropane(uint8_t slot, Propane* p) {
    if (slot >= MAX_PROPANE || p == nullptr) return false;
    if (propane_tanks[slot] != nullptr) {
        Serial.printf("[STATE] WARN: slot propane %u déjà occupé, remplacement\n", slot);
        delete propane_tanks[slot];
    }
    propane_tanks[slot] = p;
    return true;
}

bool StateService::registerCharger(uint8_t slot, Charger* c) {
    if (slot >= MAX_CHARGERS || c == nullptr) return false;
    if (chargers[slot] != nullptr) {
        Serial.printf("[STATE] WARN: slot chargeur %u déjà occupé, remplacement\n", slot);
        delete chargers[slot];
    }
    chargers[slot] = c;
    return true;
}

// ── Compteurs ─────────────────────────────────────────────────────

uint8_t StateService::batteryCount() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) if (batteries[i]) n++;
    return n;
}

uint8_t StateService::tankCount() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_TANKS; i++) if (tanks[i]) n++;
    return n;
}

uint8_t StateService::propaneCount() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_PROPANE; i++) if (propane_tanks[i]) n++;
    return n;
}

uint8_t StateService::chargerCount() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_CHARGERS; i++) if (chargers[i]) n++;
    return n;
}

// ── Lookup UI triée par ui_position ───────────────────────────────
//
// Algorithme : insertion sort sur maxOut éléments max.
// On évite heap_caps_malloc / std::sort pour rester frugal en RAM.

template<typename T>
static uint8_t sortedByUiPos(
    T* const src[], uint8_t srcSize,
    const uint8_t* uiPositions,         // uiPositions[i] = ui_position du slot i
    T* out[], uint8_t maxOut)
{
    uint8_t count = 0;

    // Collecte les pointeurs non-nuls
    T*      tmp[srcSize];
    uint8_t pos[srcSize];
    for (uint8_t i = 0; i < srcSize; i++) {
        if (!src[i]) continue;
        tmp[count] = src[i];
        pos[count] = uiPositions[i];
        count++;
    }
    if (count > maxOut) count = maxOut;

    // Tri par insertion sur le petit tableau
    for (uint8_t i = 1; i < count; i++) {
        T*      keyPtr = tmp[i];
        uint8_t keyPos = pos[i];
        int8_t  j = i - 1;
        while (j >= 0 && pos[j] > keyPos) {
            tmp[j+1] = tmp[j];
            pos[j+1] = pos[j];
            j--;
        }
        tmp[j+1] = keyPtr;
        pos[j+1] = keyPos;
    }

    for (uint8_t i = 0; i < count; i++) out[i] = tmp[i];
    return count;
}

uint8_t StateService::getBatteriesSorted(Battery* out[], uint8_t maxOut) const {
    const AppConfig& cfg = StorageService::instance().config();
    uint8_t pos[MAX_BATTERIES];
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) pos[i] = cfg.batteries[i].ui_position;
    return sortedByUiPos<Battery>(batteries, MAX_BATTERIES, pos, out, maxOut);
}

uint8_t StateService::getTanksSorted(Tank* out[], uint8_t maxOut) const {
    const AppConfig& cfg = StorageService::instance().config();
    uint8_t pos[MAX_TANKS];
    for (uint8_t i = 0; i < MAX_TANKS; i++) pos[i] = cfg.tanks[i].ui_position;
    return sortedByUiPos<Tank>(tanks, MAX_TANKS, pos, out, maxOut);
}

uint8_t StateService::getPropaneSorted(Propane* out[], uint8_t maxOut) const {
    const AppConfig& cfg = StorageService::instance().config();
    uint8_t pos[MAX_PROPANE];
    for (uint8_t i = 0; i < MAX_PROPANE; i++) pos[i] = cfg.propane[i].ui_position;
    return sortedByUiPos<Propane>(propane_tanks, MAX_PROPANE, pos, out, maxOut);
}

uint8_t StateService::getChargersSorted(Charger* out[], uint8_t maxOut) const {
    const AppConfig& cfg = StorageService::instance().config();
    uint8_t pos[MAX_CHARGERS];
    for (uint8_t i = 0; i < MAX_CHARGERS; i++) pos[i] = cfg.chargers[i].ui_position;
    return sortedByUiPos<Charger>(chargers, MAX_CHARGERS, pos, out, maxOut);
}