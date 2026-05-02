#include "StateService.h"

/**
 * Constructeur du StateService
 * Initialise les services de base comme la pompe à eau.
 * Les listes de batteries et réservoirs restent vides jusqu'à ce que
 * le HAL ou le ConfigLoader ne les peuplent.
 */
StateService::StateService() : waterPump("Pompe Eau") {
    // Note: Dans une version future, nous appellerons ici loadFromConfig()
    // pour charger les noms et types depuis la mémoire Flash (LittleFS).
}

/**
 * Destructeur du StateService
 * Très important pour un système commercial : on libère proprement 
 * la mémoire de chaque objet créé dynamiquement avec 'new'.
 */
StateService::~StateService() {
    // Libération des batteries
    for (auto b : batteries) {
        if (b != nullptr) {
            delete b;
        }
    }
    batteries.clear();

    // Libération des réservoirs
    for (auto t : tanks) {
        if (t != nullptr) {
            delete t;
        }
    }
    tanks.clear();

    // Libération des chargeurs
    for (auto c : chargers) {
        if (c != nullptr) {
            delete c;
        }
    }
    chargers.clear();
}