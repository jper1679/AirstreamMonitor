#ifndef BLUETOOTH_SERVICE_H
#define BLUETOOTH_SERVICE_H

#include <NimBLEDevice.h>

// Héritage obligatoire pour les callbacks
class BluetoothService : public NimBLEScanCallbacks {
public:
    void begin();
    
    // Signature exacte de NimBLE : onResult
    void onResult(NimBLEAdvertisedDevice* advertisedDevice);

private:
    NimBLEScan* pScan = nullptr; 
    
    void handleMopeka(NimBLEAdvertisedDevice* device);
    void handleVictron(NimBLEAdvertisedDevice* device);
    void handleLiTime(NimBLEAdvertisedDevice* device);
};

#endif