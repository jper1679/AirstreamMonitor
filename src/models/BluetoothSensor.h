#ifndef BLUETOOTH_SENSOR_H
#define BLUETOOTH_SENSOR_H

#include <Arduino.h>
#include <string>

enum class BLEConnType {
    PASSIVE_SCAN,   // Mopeka, Victron
    ACTIVE_CONNECT  // LiTime
};

class BluetoothSensorBase {
public:
    std::string macAddress;
    BLEConnType connectionType;
    std::string encryptionKey; 

    BluetoothSensorBase(std::string mac, BLEConnType type, std::string key = "") 
        : macAddress(mac), connectionType(type), encryptionKey(key) {
        // Normalisation de la MAC pour faciliter le filtrage
        for(auto & c : macAddress) c = tolower(c);
    }

    virtual void updateFromBle(uint8_t* data, size_t len) = 0;
};

#endif