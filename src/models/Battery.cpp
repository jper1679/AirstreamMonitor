#include "models/Battery.h"

Battery::Battery(const char* id, const char* name, uint32_t cap, SourceType source) 
    : m_capacityAh(cap), m_source(source) {
    strncpy(m_id, id, 16);
    strncpy(m_name, name, 16);
}

float Battery::getWatts() {
    BatteryData d = get(); // Utilise le GET protégé du template
    return d.voltage * d.current;
}

bool Battery::isCharging() {
    return get().current > 0.1f;
}