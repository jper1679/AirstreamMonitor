#ifndef ADS1115_NATIVE_H
#define ADS1115_NATIVE_H

#include <Arduino.h>
#include <Wire.h>

class ADS1115_Native {
public:
    ADS1115_Native(uint8_t address = 0x48) : _address(address) {}

    // On change la signature pour accepter un pointeur vers TwoWire (le type de &Wire)
    bool begin(TwoWire* wireInstance = &Wire);
    int16_t readADC_SingleEnded(uint8_t channel);

private:
    uint8_t _address;
    TwoWire* _wire; // On stocke l'instance du bus
};

#endif