#ifndef PROTECTEDSENSOR_H
#define PROTECTEDSENSOR_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "models/DataTypes.h"

template <typename T>
class ProtectedSensor {
protected:
    T m_data;
    DataValidity m_validity;
    SemaphoreHandle_t m_mutex;
    uint32_t m_lastUpdate;

public:
    ProtectedSensor() : m_validity(DATA_INVALID), m_lastUpdate(0) {
        m_mutex = xSemaphoreCreateMutex();
    }

    // "L'entrée" (Core 0 - Hardware)
    void set(const T& newData) {
        if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            m_data = newData;
            m_validity = DATA_VALID;
            m_lastUpdate = millis();
            xSemaphoreGive(m_mutex);
        }
    }

    // "La sortie" (Core 1 - Display)
    T get() {
        T temp{};
        if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            temp = m_data;
            xSemaphoreGive(m_mutex);
        }
        return temp;
    }

    DataValidity getStatus() {
        // Optionnel : si pas d'update depuis 30s, on passe en STALE
        if (millis() - m_lastUpdate > 30000) m_validity = DATA_STALE;
        return m_validity;
    }
};

#endif