#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "models/ProtectedSensor.h"


class Actuator : public ProtectedSensor<ActuatorData> {
public:
    Actuator(const char* name);

    void toggle();
    void confirmState(bool physicalState);

private:
    char m_name[16];
};

#endif