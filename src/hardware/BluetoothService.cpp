#include "hardware/BluetoothService.h"
#include <Arduino.h>
#include "StateService.h"
#include "StorageService.h"

void BluetoothService::begin() {
    NimBLEDevice::init("Airstream-OS");
    
    pScan = NimBLEDevice::getScan();
    
    // NIMBLE : La méthode est setCallbacks
    pScan->setScanCallbacks(this); 
    
    pScan->setActiveScan(false); 
    pScan->setInterval(100);
    pScan->setWindow(99);
    
    // NIMBLE : start(duration, isContinue, restart)
    // 0 = scan infini, false = isContinue, true = restart[cite: 1]
    pScan->start(0, false, true); 
    
    Serial.println("[BLE] Service NimBLE démarré.");
}

void BluetoothService::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    std::string foundMac = advertisedDevice->getAddress().toString();
    for (auto& c : foundMac) c = tolower(c);

    if (!advertisedDevice->haveManufacturerData()) return;
    std::string raw = advertisedDevice->getManufacturerData();
    uint8_t* d   = (uint8_t*)raw.data();
    size_t   len = raw.length();

    AppConfig& cfg = StorageService::instance().config();

    // ── Propane (Mopeka) ──────────────────────────────────────────
    for (uint8_t i = 0; i < MAX_PROPANE; i++) {
        const PropaneSlot& slot = cfg.propane[i];
        if (!slot.enabled) continue;

        std::string slotMac = slot.mac;
        for (auto& c : slotMac) c = tolower(c);
        if (foundMac != slotMac) continue;

        Propane* propane = StateService::instance().propane_tanks[i];
        if (!propane || len < 5) return;

        PropaneData td;
        td.levelPercent = ((d[4] << 8) | d[3]) / 10.0f;
        td.temperature  = (float)((int8_t)d[2]);
        td.volumeLiters = (td.levelPercent / 100.0f) * slot.capacity_L;
        propane->set(td);  // ProtectedSensor<PropaneData>::set()

        Serial.printf("[BLE] Propane[%u] '%s' : %.1f%% %.1f°C\n",
            i, slot.name, td.levelPercent, td.temperature);
        return;
    }

    // ── Batteries (LiTime) ───────────────────────────────────────
    for (uint8_t i = 0; i < MAX_BATTERIES; i++) {
        const BatterySlot& slot = cfg.batteries[i];
        if (!slot.enabled) continue;

        std::string slotMac = slot.mac;
        for (auto& c : slotMac) c = tolower(c);
        if (foundMac != slotMac) continue;

        Battery* bat = StateService::instance().batteries[i];
        if (!bat || len < 8) return;

        // Parsing LiTime BLE manufacturer data
        BatteryData bd;
        bd.voltage = ((d[1] << 8) | d[0]) / 100.0f;
        bd.current = ((int16_t)((d[3] << 8) | d[2])) / 100.0f;
        bd.soc     = d[4];
        bd.temp    = (float)d[5] - 40.0f;
        bat->set(bd);  // ProtectedSensor<BatteryData>::set()

        Serial.printf("[BLE] Battery[%u] '%s' : %.2fV %.1fA %d%%\n",
            i, slot.name, bd.voltage, bd.current, (int)bd.soc);
        return;
    }

    // ── Chargeurs (Victron) ───────────────────────────────────────
    for (uint8_t i = 0; i < MAX_CHARGERS; i++) {
        const ChargerSlot& slot = cfg.chargers[i];
        if (!slot.enabled) continue;

        std::string slotMac = slot.mac;
        for (auto& c : slotMac) c = tolower(c);
        if (foundMac != slotMac) continue;

        Charger* charger = StateService::instance().chargers[i];
        if (!charger || len < 6) return;

        // Parsing Victron Extra Manufacturer Data (GATT Advertisement)
        ChargerData cd;
        cd.outputVoltage  = ((d[3] << 8) | d[2]) / 100.0f;
        cd.outputCurrent  = ((d[5] << 8) | d[4]) / 10.0f;
        cd.powerWatts     = cd.outputVoltage * cd.outputCurrent;
        cd.isActive       = cd.outputCurrent > 0.1f;
        charger->set(cd);  // ProtectedSensor<ChargerData>::set()

        Serial.printf("[BLE] Charger[%u] '%s' : %.1fV %.1fA %.0fW\n",
            i, slot.name, cd.outputVoltage, cd.outputCurrent, cd.powerWatts);
        return;
    }
}

void BluetoothService::handleMopeka(NimBLEAdvertisedDevice* device) {
    Serial.printf("[BLE] Mopeka détecté! RSSI: %d\n", device->getRSSI());
}

void BluetoothService::handleVictron(NimBLEAdvertisedDevice* device) {}
void BluetoothService::handleLiTime(NimBLEAdvertisedDevice* device) {}