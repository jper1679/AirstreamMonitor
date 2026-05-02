#include "models/Charger.h"

Charger::Charger(const char* id, const char* name, SourceType source) 
    : m_source(source) {
    strncpy(m_id, id, 16);
    strncpy(m_name, name, 16);
}