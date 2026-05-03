#include <Arduino.h>
#include <NimBLEDevice.h>
#include "mbedtls/aes.h"

static const char* targetMac = "f5:81:ce:f9:b9:3a";

// Clé Victron (16 bytes)
static const uint8_t key[16] = {
    0xca, 0x9b, 0x40, 0xc4,
    0xad, 0x4a, 0x2c, 0x05,
    0x7b, 0x0c, 0x80, 0x52,
    0xa2, 0x81, 0xb9, 0x0f
};

static constexpr uint16_t VICTRON_MFG_ID = 0x02E1;
static constexpr uint8_t  RECORD_TYPE_DCDC = 0x04;

struct VictronDcdcData {
    bool valid = false;

    uint8_t recordType = 0;
    uint16_t nonce = 0;
    uint8_t keyCheck = 0;

    uint8_t deviceState = 0xFF;
    uint8_t chargerError = 0xFF;
    uint16_t inputVoltageRaw = 0xFFFF;
    int16_t outputVoltageRaw = 0x7FFF;
    uint32_t offReason = 0;

    float inputVoltage = NAN;
    float outputVoltage = NAN;
};

static const char* deviceStateToString(uint8_t state) {
    switch (state) {
        case 0:   return "Off";
        case 1:   return "Low power";
        case 2:   return "Fault";
        case 3:   return "Bulk";
        case 4:   return "Absorption";
        case 5:   return "Float";
        case 6:   return "Storage";
        case 7:   return "Equalize";
        case 9:   return "Inverting";
        case 11:  return "Power supply";
        case 246: return "Repeated absorption";
        case 247: return "Auto equalize / recharge";
        case 252: return "External control";
        default:  return "Unknown";
    }
}

static void printHex(const uint8_t* data, size_t len, const char* label) {
    Serial.print(label);
    for (size_t i = 0; i < len; i++) {
        Serial.printf("%02X ", data[i]);
    }
    Serial.println();
}

static bool decryptVictronCtrPayload(
    const uint8_t* encrypted,
    size_t encryptedLen,
    uint16_t nonceLSB,
    uint8_t* output
) {
    if (!encrypted || !output || encryptedLen == 0) {
        return false;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // En CTR, on utilise la clé d'encryption.
    if (mbedtls_aes_setkey_enc(&aes, key, 128) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }

    uint8_t nonce_counter[16] = {0};
    uint8_t stream_block[16] = {0};
    size_t nc_off = 0;

    // Hypothèse Victron la plus plausible d'après le PDF:
    // nonce 2 bytes du header, ordre LSB tel que transmis.
    nonce_counter[0] = (uint8_t)(nonceLSB & 0xFF);
    nonce_counter[1] = (uint8_t)((nonceLSB >> 8) & 0xFF);

    // Le compteur CTR démarre à 0.
    if (mbedtls_aes_crypt_ctr(
            &aes,
            encryptedLen,
            &nc_off,
            nonce_counter,
            stream_block,
            encrypted,
            output) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }

    mbedtls_aes_free(&aes);
    return true;
}

static bool parseVictronDcdcRecord(
    const uint8_t* raw,
    size_t len,
    VictronDcdcData& out
) {
    out = VictronDcdcData();

    // Trame minimale observée:
    // [0..1]  Manufacturer ID = E1 02
    // [2..3]  Product Advertisement type = 10 00
    // [4..5]  Product ID (LE), ex A3D0
    // [6]     Victron record type = 0x04 (DC/DC)
    // [7..8]  Nonce/data counter (LSB)
    // [9]     Key byte 0
    // [10..]  Données chiffrées
    if (!raw || len < 20) {
        return false;
    }

    uint16_t mfg = (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
    if (mfg != VICTRON_MFG_ID) {
        return false;
    }

    // Product advertisement / beacon type = 0x0010
    uint16_t advType = (uint16_t)raw[2] | ((uint16_t)raw[3] << 8);
    if (advType != 0x0010) {
        Serial.printf("Unexpected advType: 0x%04X\n", advType);
        return false;
    }

    uint16_t productId = (uint16_t)raw[4] | ((uint16_t)raw[5] << 8);

    // Optionnel: filtre Orion Smart / Buck-Boost
    if (productId < 0xA3C0 || productId > 0xA3DF) {
        Serial.printf("Unexpected productId: 0x%04X\n", productId);
        return false;
    }

    out.recordType = raw[6];
    if (out.recordType != RECORD_TYPE_DCDC) {
        Serial.printf("Unexpected Victron recordType: 0x%02X\n", out.recordType);
        return false;
    }

    out.nonce = (uint16_t)raw[7] | ((uint16_t)raw[8] << 8);
    out.keyCheck = raw[9];

    if (out.keyCheck != key[0]) {
        Serial.printf("Key byte mismatch: packet=%02X expected=%02X\n", out.keyCheck, key[0]);
        return false;
    }

    const uint8_t* encrypted = raw + 10;
    size_t encryptedLen = len - 10;

    // Le PDF dit que les records actuels tiennent en 16 bytes ou moins
    if (encryptedLen == 0 || encryptedLen > 16) {
        Serial.printf("Unexpected encrypted length: %u\n", (unsigned)encryptedLen);
        return false;
    }

    uint8_t decrypted[16] = {0};
    if (!decryptVictronCtrPayload(encrypted, encryptedLen, out.nonce, decrypted)) {
        Serial.println("AES-CTR decrypt failed");
        return false;
    }

    printHex(raw, len, "ManufacturerData: ");
    Serial.printf("Adv type    : 0x%04X\n", advType);
    Serial.printf("Product ID  : 0x%04X\n", productId);
    Serial.printf("Record type : 0x%02X\n", out.recordType);
    Serial.printf("Nonce       : 0x%04X\n", out.nonce);
    Serial.printf("Key byte    : 0x%02X\n", out.keyCheck);
    printHex(encrypted, encryptedLen, "Encrypted: ");
    printHex(decrypted, encryptedLen, "Decrypted: ");

    // Layout DC/DC Converter selon le PDF:
    // byte 0  = Device state
    // byte 1  = Charger error
    // byte 2-3 = Input voltage, uint16, 0.01V
    // byte 4-5 = Output voltage, int16, 0.01V
    // byte 6-9 = Off reason, uint32
    if (encryptedLen < 10) {
        Serial.println("Encrypted payload too short for DC/DC layout");
        return false;
    }

    out.deviceState = decrypted[0];
    out.chargerError = decrypted[1];
    out.inputVoltageRaw = (uint16_t)decrypted[2] | ((uint16_t)decrypted[3] << 8);
    out.outputVoltageRaw = (int16_t)((uint16_t)decrypted[4] | ((uint16_t)decrypted[5] << 8));
    out.offReason =
        (uint32_t)decrypted[6] |
        ((uint32_t)decrypted[7] << 8) |
        ((uint32_t)decrypted[8] << 16) |
        ((uint32_t)decrypted[9] << 24);

    if (out.inputVoltageRaw != 0xFFFF) {
        out.inputVoltage = out.inputVoltageRaw / 100.0f;
    }

    if (out.outputVoltageRaw != 0x7FFF) {
        out.outputVoltage = out.outputVoltageRaw / 100.0f;
    }

    out.valid = true;
    return true;
}

static bool macMatches(const NimBLEAddress &addr, const char *targetMac) {
    String found = String(addr.toString().c_str());
    found.toUpperCase();

    String target = String(targetMac);
    target.toUpperCase();

    return found == target;
}

class AdvertisedCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* device) override {
        Serial.printf("SEEN: %s RSSI=%d hasMfg=%s name='%s'\n",
                        device->getAddress().toString().c_str(),
                        device->getRSSI(),
                        device->haveManufacturerData() ? "yes" : "no",
                        device->getName().c_str());
        if (!macMatches(device->getAddress(), targetMac)) return;

        Serial.println("MAC MATCH");

        if (!device->haveManufacturerData()) {
            return;
        }

        std::string data = device->getManufacturerData();
        const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());

        Serial.printf("SEEN: %s RSSI=%d hasMfg=yes name='%s'\n",
                      device->getAddress().toString().c_str(),
                      device->getRSSI(),
                      device->getName().c_str());

        printHex(raw, data.size(), "ManufacturerData: ");

        VictronDcdcData dcdc;
        if (!parseVictronDcdcRecord(raw, data.size(), dcdc)) {
            Serial.println("Not a valid Victron DC/DC extra manufacturer record.\n");
            return;
        }

        Serial.println("---- Victron DC/DC ----");
        Serial.printf("Record type : 0x%02X\n", dcdc.recordType);
        Serial.printf("Nonce       : 0x%04X\n", dcdc.nonce);
        Serial.printf("Key byte    : 0x%02X\n", dcdc.keyCheck);
        Serial.printf("State       : %s (%u)\n", deviceStateToString(dcdc.deviceState), dcdc.deviceState);
        Serial.printf("Error       : %u\n", dcdc.chargerError);

        if (!isnan(dcdc.inputVoltage)) {
            Serial.printf("Input V     : %.2f V\n", dcdc.inputVoltage);
        } else {
            Serial.println("Input V     : N/A");
        }

        if (!isnan(dcdc.outputVoltage)) {
            Serial.printf("Output V    : %.2f V\n", dcdc.outputVoltage);
        } else {
            Serial.println("Output V    : N/A");
        }

        Serial.printf("Off reason  : 0x%08lX\n", (unsigned long)dcdc.offReason);
        Serial.println("-----------------------\n");
    }
};

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Starting Victron BLE scan...");
    NimBLEDevice::init("");

    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new AdvertisedCallbacks(), false);
    pScan->setActiveScan(false);   // passive scan recommandé pour advertisements
    pScan->setInterval(100);
    pScan->setWindow(100);
    pScan->setMaxResults(0);
    pScan->start(0, false, true);
}

void loop() {
    delay(1000);
}