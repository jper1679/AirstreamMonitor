#ifndef MOPEKA_SENSOR_H
#define MOPEKA_SENSOR_H

#include "BluetoothSensor.h"
#include "models/ProtectedSensor.h"

struct MopekaData {
    float tankLevel;    // En pourcentage
    float temperature;  // En Celsius
    float batteryV;     // Voltage du capteur[cite: 1]
    bool isPresent;     //[cite: 1]
};

class MopekaSensor : public BluetoothSensorBase, public ProtectedSensor<float> {
public:
    MopekaData data;

    MopekaSensor(std::string mac, std::string name) 
        : BluetoothSensorBase(mac, BLEConnType::PASSIVE_SCAN),
          ProtectedSensor<float>(name, 0.0f) {
        data = {0};
    }

    // Version membre de ton parseur pour mettre à jour l'objet interne[cite: 1]
    void updateFromBle(uint8_t* rawData, size_t len) override {
        if (len < 10) return; // Sécurité sur la taille du paquet[cite: 1]

        // --- Logique de décodage Mopeka (Exemple de calcul) ---
        // Battery Voltage (Byte 0 & 1)[cite: 1]
        data.batteryV = (rawData[0] & 0x7F) / 32.0f; 
        
        // Temperature (Byte 2)[cite: 1]
        data.temperature = (float)((int8_t)rawData[2]); 

        // Tank Level (Byte 3 & 4)[cite: 1]
        // Note: Le calcul dépend du type de gaz et de la taille du réservoir[cite: 1]
        float rawReading = (float)((rawData[4] << 8) | rawData[3]);
        data.tankLevel = rawReading / 10.0f; 

        data.isPresent = true;

        // Mise à jour de la valeur principale pour l'UI (ProtectedSensor)[cite: 1]
        this->updateValue(data.tankLevel);
    }
};

#endif