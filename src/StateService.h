#ifndef STATESERVICE_H
#define STATESERVICE_H

#include <vector>
#include <cstring>
#include "models/Battery.h"
#include "models/Tank.h"
#include "models/Charger.h"
#include "models/Actuator.h"

class StateService {
public:
    static StateService& instance() {
        static StateService inst;
        return inst;
    }

    // Listes dynamiques
    std::vector<Battery*> batteries;
    std::vector<Tank*> tanks;
    std::vector<Tank*> propane_tanks;
    std::vector<Charger*> chargers;

    // L'objet Pompe
    Actuator waterPump; 

    // Méthodes d'enregistrement
    void registerBattery(Battery* b) { batteries.push_back(b); }
    void registerTank(Tank* t) { tanks.push_back(t); }

private:
    StateService();  // Défini dans le .cpp
    ~StateService(); // Défini dans le .cpp
};

#endif