#include <Arduino.h>
#include <NimBLEDevice.h>
#include <cmath>
#include <cstdint>
#include <string>
#include "mbedtls/aes.h"


void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== AIRSTREAM TEST : Unified Bluetooth ===");

    static const TankType TANK_TYPE = TANK_30LB_V;
    static const TankCalibration TANK = getTankCalibration(TANK_TYPE);

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEScan* pScan = NimBLEDevice::getScan();    // replace *scan par *pScan
    pScan->setScanCallbacks(new AdvertisedCallbacks(), false);
    pScan->setActiveScan(false);   // passive scan recommandé pour advertisements, était FALSE pour MOPEKA, TRUE pour VICTRON
    pScan->setInterval(100);
    pScan->setWindow(100);
    pScan->setDuplicateFilter(true);
    pScan->setMaxResults(0);
    pScan->start(0, false, true);
    Serial.println("\n=== Setup Done ===");
}

void loop() { // From Litime
    Serial.println("Connecting...");

    if (!connectToBattery()) {
        delay(5000);
        return;
    }

    dataReady = false;

    pWriteChar->writeValue(queryCommand, 8, true);

    uint32_t start = millis();
    while (!dataReady && millis() - start < 2000) {
        delay(10);
    }

    if (dataReady) {
        parseData(lastData);
    } else {
        Serial.println("Timeout");
    }

    disconnectBattery();

    Serial.println("Wait 15s...\n");
    delay(15000);
}

