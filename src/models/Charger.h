#ifndef CHARGER_H
#define CHARGER_H

#include "models/ProtectedSensor.h"

class Charger : public ProtectedSensor<ChargerData> {
public:
    Charger(const char* id, const char* name, SourceType source);
    
    const char* getId() const { return m_id; }
    const char* getName() const { return m_name; }

private:
    char m_id[16];
    char m_name[16];
    SourceType m_source;
};

#endif