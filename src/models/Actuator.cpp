#include "models/Actuator.h"


// Le constructeur initialise le nom et s'assure que l'état initial est OFF
Actuator::Actuator(const char* name) {
    strncpy(m_name, name, 16);
    
    ActuatorData initialData;
    initialData.requestedState = false;
    initialData.actualState = false;
    initialData.fault = false;
    
    set(initialData); // On initialise le ProtectedSensor
}

// La méthode toggle permet de changer l'état sans connaître l'état actuel
void Actuator::toggle() {
    ActuatorData d = get();
    d.requestedState = !d.requestedState;
    
    // Note : On ne change pas actualState ici. 
    // C'est le job du HAL (Core 0) de confirmer que le relais a cliqué.
    set(d);
}

// Permet au HAL de confirmer que l'action physique a eu lieu
void Actuator::confirmState(bool physicalState) {
    ActuatorData d = get();
    d.actualState = physicalState;
    set(d);
}