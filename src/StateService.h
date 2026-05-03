#ifndef STATESERVICE_H
#define STATESERVICE_H

#include "models/Battery.h"
#include "models/Tank.h"
#include "models/Charger.h"
#include "models/Propane.h"
#include "models/Actuator.h"
#include "StorageService.h"  // Pour MAX_*

class StateService {
public:
    static StateService& instance() {
        static StateService inst;
        return inst;
    }

    // ── Arrays indexés (index = slot config) ──────────────────────
    Battery* batteries[MAX_BATTERIES]   = {nullptr};
    Tank*    tanks[MAX_TANKS]           = {nullptr};
    Propane* propane_tanks[MAX_PROPANE] = {nullptr};
    Charger* chargers[MAX_CHARGERS]     = {nullptr};

    // ── Actuateurs ────────────────────────────────────────────────
    Actuator waterPump;

    // ── Enregistrement (appelé depuis setup()) ────────────────────
    bool registerBattery(uint8_t slot, Battery* b);
    bool registerTank(uint8_t slot, Tank* t);
    bool registerPropane(uint8_t slot, Propane* t);
    bool registerCharger(uint8_t slot, Charger* c);

    // ── Compteurs rapides ─────────────────────────────────────────
    uint8_t batteryCount()  const;
    uint8_t tankCount()     const;
    uint8_t propaneCount()  const;
    uint8_t chargerCount()  const;

    // ── Lookup UI : retourne les slots actifs triés par ui_position ──
    // Le appelant passe un buffer pré-alloué de taille MAX_*
    // Retourne le nombre d'éléments remplis.
    uint8_t getBatteriesSorted(Battery*  out[], uint8_t maxOut) const;
    uint8_t getTanksSorted    (Tank*     out[], uint8_t maxOut) const;
    uint8_t getPropaneSorted  (Propane*  out[], uint8_t maxOut) const;
    uint8_t getChargersSorted (Charger*  out[], uint8_t maxOut) const;

private:
    StateService() : waterPump("Pompe Eau") {}
    ~StateService();

    // Interdit la copie
    StateService(const StateService&)            = delete;
    StateService& operator=(const StateService&) = delete;
};

#endif