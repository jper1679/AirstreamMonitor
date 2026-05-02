#ifndef TANK_H
#define TANK_H

#include "models/ProtectedSensor.h"

class Tank : public ProtectedSensor<TankData> {
public:
    Tank(const char* id, const char* name, float capacityLiters, SourceType source);
    
    const char* getName() const { return m_name; }
    
    // Met à jour le niveau et calcule le volume automatiquement
    void updateLevel(float percent);

private:
    char m_id[16];
    char m_name[16];
    float m_capacityLiters;
    SourceType m_source;
};

#endif