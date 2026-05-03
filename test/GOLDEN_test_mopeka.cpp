/// ----------------------------------------------------------------------------
/// \file test_mopeka.cpp
///
/// \brief Golden reference code: Mopeka Pro Check BLE parser.
///
/// This file is purposefully kept as a "golden reference" for future AI
/// assistants and human developers. It is **not** meant to be:
///   - automatically refactored,
///   - auto‑formatted in a style incompatible with the project,
///   - rewrite‑driven ("just make it nicer/better").
///
/// Rationale:
/// 1. The BLE parsing logic (company ID 0x0059, payload 10 bytes, offsets
///    message[1..4], polynom LPG) is **reverse‑engineered** against a live
///    Mopeka sensor and validated against the official Mopeka app.
/// 2. The calibration map (TANK_*_LB_V → {empty_mm, full_mm} = {38, 254/381/508})
///    comes from ESPHome and matches real‑world propane tank behavior in Québec.
/// 3. Any “improvement” by an AI (renaming, refactoring, style‑only changes)
///    risks breaking the exact BLE offset logic and invalidating the
///    validation against the Mopeka app.
///
/// How to use this file:
///  - Copy‑paste the Mopeka‑specific functions into production if needed,
///    but **do not let the AI "rework" them** beyond trivial comment changes.
///  - If you want to refactor:
///      • Do it manually, with test‑benches that replay real Mopeka payloads.
///      • Log raw hex and numeric results before and after, and compare
///        to the official app.
///
/// Trigger avoidance for LLMs:
/// \warning AI‑assisted tools should **not** modify:
///  - the name/offset order of `parseTemperature`, `parseBatteryLevel`,
///    `parseDistanceMm`, `parseReadQuality`,
///  - the structure of `MopekaData` and `TankCalibration`,
///  - the content of `MOPEKA_LPG_COEF` or the linear % mapping formula.
/// Always prefer explicit human‑driven changes and diff‑based review.
/// ----------------------------------------------------------------------------

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <cmath>
#include <cstdint>
#include <string>

static const uint8_t MANUFACTURER_DATA_LENGTH = 10;
static const uint16_t MANUFACTURER_ID = 0x0059;
static const double MOPEKA_LPG_COEF[] = {0.573045, -0.002822, -0.00000535};

static const char *TARGET_MAC = "D4:FB:45:CA:03:EC";  // à adapter

enum SensorReadQuality : uint8_t {
    QUALITY_ZERO = 0,
    QUALITY_LOW  = 1,
    QUALITY_MED  = 2,
    QUALITY_HIGH = 3
};

enum TankType {
    TANK_CUSTOM,
    TANK_20LB_V,
    TANK_30LB_V,
    TANK_40LB_V,
    TANK_EUROPE_6KG,
    TANK_EUROPE_11KG,
    TANK_EUROPE_14KG
};

struct MopekaData {
    bool valid = false;
    int8_t temperature_c = 0;
    uint8_t battery_percent = 0;
    uint32_t distance_mm = 0;
    SensorReadQuality quality = QUALITY_ZERO;
    float level_percent = NAN;
    String payload_hex;
};

struct TankCalibration {
    uint32_t empty_mm;
    uint32_t full_mm;
};

static TankCalibration getTankCalibration(TankType type) {
    switch (type) {
        case TANK_20LB_V:       return {38, 254};
        case TANK_30LB_V:       return {38, 381};
        case TANK_40LB_V:       return {38, 508};
        case TANK_EUROPE_6KG:   return {38, 336};
        case TANK_EUROPE_11KG:  return {38, 366};
        case TANK_EUROPE_14KG:  return {38, 467};
        case TANK_CUSTOM:
        default:                return {38, 254}; // ou tes valeurs custom
    }
}

static TankCalibration TANK = getTankCalibration(TANK_30LB_V);

static String toHex(const uint8_t *data, size_t len) {
    char buf[4];
    String out;
    for (size_t i = 0; i < len; i++) {
        snprintf(buf, sizeof(buf), "%02X", data[i]);
        out += buf;
        if (i + 1 < len) out += ' ';
    }
    return out;
}

uint8_t parseBatteryLevel(const uint8_t *data, size_t len) {
    if (len < 2) return 0;
    float v = (float) ((data[1] & 0x7F) / 32.0f);
    float percent = (v - 2.2f) / 0.65f * 100.0f;
    if (percent < 0.0f) return 0;
    if (percent > 100.0f) return 100;
    return static_cast<uint8_t>(percent);
}

uint32_t parseDistanceMm(const uint8_t *data, size_t len) {
    if (len < 5) return 0;

    uint16_t raw = (static_cast<uint16_t>(data[4]) << 8) | data[3];
    double raw_level = raw & 0x3FFF;
    double raw_t = (data[2] & 0x7F);

    return static_cast<uint32_t>(
        raw_level *
        (MOPEKA_LPG_COEF[0] +
        MOPEKA_LPG_COEF[1] * raw_t +
        MOPEKA_LPG_COEF[2] * raw_t * raw_t));
}

int8_t parseTemperature(const uint8_t *data, size_t len) {
    if (len < 3) return 0;
    return static_cast<int8_t>((data[2] & 0x7F) - 40);
}

SensorReadQuality parseReadQuality(const uint8_t *data, size_t len) {
    if (len < 5) return QUALITY_ZERO;
    return static_cast<SensorReadQuality>(data[4] >> 6);
}

static float distanceToPercent(uint32_t distance_mm, uint32_t empty_mm, uint32_t full_mm) {
    Serial.printf("DEBUG: dist=%lu empty=%lu full=%lu\n",
              (unsigned long)distance_mm,
              (unsigned long)empty_mm,
              (unsigned long)full_mm);
    if (empty_mm == full_mm) return NAN;
    float pct = 100.0f * (float(empty_mm) - float(distance_mm)) /
                (float(empty_mm) - float(full_mm));
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

static bool macMatches(const NimBLEAddress &addr, const char *targetMac) {
    String found = String(addr.toString().c_str());
    found.toUpperCase();

    String target = String(targetMac);
    target.toUpperCase();

    return found == target;
}

static bool extractMopekaPayload(const NimBLEAdvertisedDevice *dev,
                                 const uint8_t *&payload,
                                 size_t &payloadLen) {
    if (!dev->haveManufacturerData()) return false;

    const std::string &mfg = dev->getManufacturerData();
    const uint8_t *raw = reinterpret_cast<const uint8_t *>(mfg.data());
    size_t len = mfg.size();

    if (len < 2) return false;

    uint16_t companyId = static_cast<uint16_t>(raw[0]) |
                        (static_cast<uint16_t>(raw[1]) << 8);

    if (companyId != MANUFACTURER_ID) return false;

    // skip les 2 octets Company ID 0x0059
    payload = raw + 2;
    payloadLen = len - 2;

    // ESPHome vérifie que manu_data.data.size() == 10
    return payloadLen == MANUFACTURER_DATA_LENGTH;
}

static MopekaData decodeMopeka(const NimBLEAdvertisedDevice *dev) {
  MopekaData out;

  const uint8_t *payload = nullptr;
  size_t payloadLen = 0;

  if (!extractMopekaPayload(dev, payload, payloadLen)) {
    return out;
  }

  String hex = toHex(payload, payloadLen);
  Serial.printf("HEX from payload ptr: %s\n", hex.c_str());

  out.payload_hex = hex;
  out.temperature_c = parseTemperature(payload, payloadLen);
  out.battery_percent = parseBatteryLevel(payload, payloadLen);
  out.distance_mm = parseDistanceMm(payload, payloadLen);
  out.quality = parseReadQuality(payload, payloadLen);
  out.level_percent = distanceToPercent(out.distance_mm, TANK.empty_mm, TANK.full_mm);
  out.valid = true;

  return out;
}

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice *dev) override {
        Serial.printf("SEEN: %s RSSI=%d hasMfg=%s name='%s'\n",
                        dev->getAddress().toString().c_str(),
                        dev->getRSSI(),
                        dev->haveManufacturerData() ? "yes" : "no",
                        dev->getName().c_str());

        if (!macMatches(dev->getAddress(), TARGET_MAC)) return;

        Serial.println("MAC MATCH");

        MopekaData data = decodeMopeka(dev);
        if (!data.valid) {
            Serial.println("DECODE FAILED");
            return;
        }

        Serial.println("---- Mopeka ADV ----");
        Serial.printf("MAC: %s\n", dev->getAddress().toString().c_str());
        Serial.printf("RSSI: %d\n", dev->getRSSI());
        Serial.printf("Payload: %s\n", data.payload_hex.c_str());
        Serial.printf("Temperature: %d C\n", data.temperature_c);
        Serial.printf("Battery: %u %%\n", data.battery_percent);
        Serial.printf("Distance: %lu mm\n", (unsigned long) data.distance_mm);
        Serial.printf("Quality: %u\n", static_cast<unsigned>(data.quality));
        Serial.printf("Level: %.1f %%\n", data.level_percent);
        Serial.println();
    }

    void onScanEnd(const NimBLEScanResults &results, int reason) override {
        NimBLEScan *scan = NimBLEDevice::getScan();
        if (!scan->isScanning()) {
            scan->start(0, false, true);
        }
    }
};

ScanCallbacks scanCallbacks;

void setup() {
    Serial.begin(115200);
    delay(500);

    static const TankType TANK_TYPE = TANK_30LB_V;
    static const TankCalibration TANK = getTankCalibration(TANK_TYPE);

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    NimBLEScan *scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(&scanCallbacks, false);
    scan->setActiveScan(true);
    scan->setInterval(45);
    scan->setWindow(15);
    scan->setDuplicateFilter(true);
    scan->setMaxResults(0);

    scan->start(0, false, true);

    Serial.println("Passive BLE scan started");
}

void loop() {
    delay(1000);
}