#ifndef BATTERY_H
#define BATTERY_H

#include "models/ProtectedSensor.h"

class Battery : public ProtectedSensor<BatteryData> {
public:
    // Ajoute SourceType source à la fin
    Battery(const char* id, const char* name, uint32_t cap, SourceType source);
    
    const char* getId() const { return m_id; }
    const char* getName() const { return m_name; }
    float getWatts();
    bool isCharging();

private:
    char m_id[16];
    char m_name[16];
    uint32_t m_capacityAh;
    SourceType m_source; // Assure-toi que c'est là aussi
};

#endif