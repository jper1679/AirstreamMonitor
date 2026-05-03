#include <Arduino.h>
#include <NimBLEDevice.h>

static NimBLEAddress batteryAddress("C8:47:80:18:C1:17", BLE_ADDR_PUBLIC);

static NimBLEClient* pClient = nullptr;
static NimBLERemoteCharacteristic* pWriteChar = nullptr;
static NimBLERemoteCharacteristic* pNotifyChar = nullptr;

static bool dataReady = false;
static std::vector<uint8_t> lastData;

static uint8_t queryCommand[8] = {0x00, 0x00, 0x04, 0x01, 0x13, 0x55, 0xAA, 0x17};

enum BatteryPollState {
    BAT_IDLE,
    BAT_WAITING_DATA,
    BAT_DISCONNECTING,
    BAT_WAIT_RETRY_SHORT,
    BAT_WAIT_RETRY_LONG
};

BatteryPollState batteryState = BAT_IDLE;
uint32_t stateStartedAt = 0;

static constexpr uint32_t CONNECT_RETRY_DELAY_MS = 5000;
static constexpr uint32_t RESPONSE_TIMEOUT_MS    = 2000;
static constexpr uint32_t POLL_INTERVAL_MS       = 15000;

void changeBatteryState(BatteryPollState newState) {
    batteryState = newState;
    stateStartedAt = millis();
}

bool connectToBattery() {
    pClient = NimBLEDevice::createClient();

    if (!pClient->connect(batteryAddress)) {
        Serial.println("Connection failed");
        return false;
    }

    NimBLERemoteService* pService = pClient->getService("FFE0");
    if (!pService) {
        Serial.println("Service not found");
        pClient->disconnect();
        return false;
    }

    pWriteChar = pService->getCharacteristic("FFE2");
    pNotifyChar = pService->getCharacteristic("FFE1");

    if (!pWriteChar || !pNotifyChar) {
        Serial.println("Characteristics not found");
        pClient->disconnect();
        return false;
    }

    pNotifyChar->subscribe(true, [](NimBLERemoteCharacteristic*,
                                   uint8_t* data,
                                   size_t length,
                                   bool) {
        lastData.assign(data, data + length);
        dataReady = true;
    }, true);

    return true;
}

void disconnectBattery() {
    if (pClient && pClient->isConnected()) {
        pClient->disconnect();
    }
}

uint16_t getU16(const std::vector<uint8_t>& d, int i) {
    return d[i] | (d[i+1] << 8);
}

int16_t getS16(const std::vector<uint8_t>& d, int i) {
    return (int16_t)(d[i] | (d[i+1] << 8));
}

int32_t getS32(const std::vector<uint8_t>& d, int i) {
    return (int32_t)(
        d[i] |
        (d[i+1] << 8) |
        (d[i+2] << 16) |
        (d[i+3] << 24)
    );
}

void parseData(const std::vector<uint8_t>& d) {
    if (d.size() < 100) {
        Serial.println("Invalid packet");
        return;
    }

    float voltage = getS32(d, 12) / 1000.0;
    float current = getS32(d, 48) / 1000.0;
    float remainingAh = getU16(d, 62) / 100.0;
    float capacityAh = getU16(d, 64) / 100.0;

    int cellTemp = getS16(d, 52);
    int bmsTemp = getS16(d, 54);

    float soc = (remainingAh / capacityAh) * 100.0;

    Serial.println("========== BATTERY ==========");
    Serial.printf("Voltage: %.3f V\n", voltage);
    Serial.printf("Current: %.3f A\n", current);
    Serial.printf("Power: %.2f W\n", voltage * current);

    Serial.printf("SOC: %.1f %%\n", soc);
    Serial.printf("Remaining: %.2f / %.2f Ah\n", remainingAh, capacityAh);

    Serial.printf("Cell Temp: %d C\n", cellTemp);
    Serial.printf("BMS Temp: %d C\n", bmsTemp);

    // Cell voltages
    Serial.print("Cells: ");
    for (int i = 16; i < 48; i += 2) {
        uint16_t mv = getU16(d, i);
        if (mv != 0) {
            Serial.printf("%.3f ", mv / 1000.0);
        }
    }
    Serial.println();

    // Protection state
    uint32_t protection =
        d[76] |
        (d[77] << 8) |
        (d[78] << 16) |
        (d[79] << 24);

    Serial.printf("Protection: 0x%08X\n", protection);

    // Battery state
    uint16_t state = getU16(d, 88);
    Serial.printf("State: 0x%04X\n", state);

    Serial.println("=============================\n");
}

void batteryTask() {
    uint32_t now = millis();

    switch (batteryState) {
        case BAT_IDLE:
            Serial.println("Connecting...");
            if (connectToBattery()) {
                dataReady = false;
                pWriteChar->writeValue(queryCommand, 8, true);
                changeBatteryState(BAT_WAITING_DATA);
            } else {
                Serial.println("Connect failed");
                changeBatteryState(BAT_WAIT_RETRY_SHORT);
            }
            break;

        case BAT_WAITING_DATA:
            if (dataReady) {
                parseData(lastData);
                changeBatteryState(BAT_DISCONNECTING);
            } else if (now - stateStartedAt >= RESPONSE_TIMEOUT_MS) {
                Serial.println("Timeout");
                changeBatteryState(BAT_DISCONNECTING);
            }
            break;

        case BAT_DISCONNECTING:
            disconnectBattery();
            Serial.println("Wait 15s...\n");
            changeBatteryState(BAT_WAIT_RETRY_LONG);
            break;

        case BAT_WAIT_RETRY_SHORT:
            if (now - stateStartedAt >= CONNECT_RETRY_DELAY_MS) {
                changeBatteryState(BAT_IDLE);
            }
            break;

        case BAT_WAIT_RETRY_LONG:
            if (now - stateStartedAt >= POLL_INTERVAL_MS) {
                changeBatteryState(BAT_IDLE);
            }
            break;
    }
}

void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("");
}

void loop() {
    batteryTask();
    delay(1000);
}