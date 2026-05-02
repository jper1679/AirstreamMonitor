#include "ADS1115_Native.h"

bool ADS1115_Native::begin(TwoWire* wireInstance) {
    _wire = wireInstance;
    _wire->beginTransmission(_address);
    return (_wire->endTransmission() == 0);
}

int16_t ADS1115_Native::readADC_SingleEnded(uint8_t channel) {
    if (channel > 3) return 0;

    // Config: Start single-ended conversion, +/-6.144V, Single-shot mode
    uint16_t config = 0x8483; 
    config |= (channel << 12); // Positionne le canal

    _wire->beginTransmission(_address);
    _wire->write(0x01); // Config register
    _wire->write((uint8_t)(config >> 8));
    _wire->write((uint8_t)(config & 0xFF));
    _wire->endTransmission();

    delay(10); // Temps de conversion

    _wire->beginTransmission(_address);
    _wire->write(0x00); // Conversion register
    _wire->endTransmission();

    _wire->requestFrom(_address, (uint8_t)2);
    if (_wire->available() == 2) {
        return (_wire->read() << 8) | _wire->read();
    }
    return 0;
}