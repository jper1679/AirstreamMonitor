#include "hardware/BluetoothService.h"
#include <Arduino.h>

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
    for(auto & c : foundMac) c = tolower(c); // Normalisation[cite: 1]

    // On suppose que tankMain est une instance de MopekaSensor accessible[cite: 1]
    if (foundMac == tankMain.macAddress) {
        if (advertisedDevice->haveManufacturerData()) {
            std::string raw = advertisedDevice->getManufacturerData();
            tankMain.updateFromBle((uint8_t*)raw.data(), raw.length());
            
            // Debug pour confirmer la température et le niveau[cite: 1]
            Serial.printf("[BLE] %s : %.1f%% (%0.1f°C)\n", 
                          tankMain.getName().c_str(), 
                          tankMain.getValue(), 
                          tankMain.data.temperature);
        }
    }
}

void BluetoothService::handleMopeka(NimBLEAdvertisedDevice* device) {
    Serial.printf("[BLE] Mopeka détecté! RSSI: %d\n", device->getRSSI());
}

void BluetoothService::handleVictron(NimBLEAdvertisedDevice* device) {}
void BluetoothService::handleLiTime(NimBLEAdvertisedDevice* device) {}