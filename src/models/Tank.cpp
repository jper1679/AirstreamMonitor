#include "models/Tank.h"

// Constructeur : On initialise l'ID, le nom, la capacité et la source
Tank::Tank(const char* id, const char* name, float capacityLiters, SourceType source) 
    : m_capacityLiters(capacityLiters), m_source(source) {
    strncpy(m_id, id, 16);
    strncpy(m_name, name, 16);
}

// C'est ici que la magie opère pour l'affichage
// On transforme le % en Litres avant de stocker dans le ProtectedSensor
void Tank::updateLevel(float percent) {
    TankData d;
    
    // Protection contre les valeurs aberrantes (ex: capteur qui saute)
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    d.levelPercent = percent;
    d.volumeLiters = (percent / 100.0f) * m_capacityLiters;
    d.temperature = 0.0f; // Optionnel : pourrait être ajouté si le capteur le supporte

    // Envoi sécurisé au StateManager via le template ProtectedSensor
    set(d);
}