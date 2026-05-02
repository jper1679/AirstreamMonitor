#include <Arduino.h>
#include <unity.h>
#include "models/Tank.h"

void test_tank_initialization() {
    Tank tank("T_FRESH", "Eau Propre", 100.0f, SOURCE_ADC);
    
    TEST_ASSERT_EQUAL_STRING("Eau Propre", tank.getName());
    // Au départ, sans update, les données doivent être à zéro ou par défaut
    TEST_ASSERT_EQUAL_UINT8(DATA_INVALID, tank.getStatus());
}

void test_tank_update_logic() {
    // Réservoir de 200 Litres
    Tank tank("T_GREY", "Eaux Grises", 200.0f, SOURCE_ADC);
    
    // On simule une lecture à 50%
    tank.updateLevel(50.0f);
    
    TankData data = tank.get();
    TEST_ASSERT_EQUAL_FLOAT(50.0f, data.levelPercent);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, data.volumeLiters); // 50% de 200L
    TEST_ASSERT_EQUAL_UINT8(DATA_VALID, tank.getStatus());
}

void test_tank_bounds_protection() {
    Tank tank("T_BLACK", "Eaux Noires", 80.0f, SOURCE_ADC);
    
    // Test valeur négative
    tank.updateLevel(-10.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, tank.get().levelPercent);
    
    // Test valeur > 100
    tank.updateLevel(110.0f);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, tank.get().levelPercent);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, tank.get().volumeLiters);
}

void test_tank_precision() {
    // Test avec des valeurs flottantes précises
    Tank tank("T_PRECISION", "Test", 75.5f, SOURCE_ADC);
    
    tank.updateLevel(33.33f);
    float expectedVolume = (33.33f / 100.0f) * 75.5f;
    
    TEST_ASSERT_EQUAL_FLOAT(expectedVolume, tank.get().volumeLiters);
}

void setup() {
    // Délai pour laisser le temps au port série de s'initialiser sur l'ESP32
    delay(2000);

    UNITY_BEGIN();
    
    RUN_TEST(test_tank_initialization);
    RUN_TEST(test_tank_update_logic);
    RUN_TEST(test_tank_bounds_protection);
    RUN_TEST(test_tank_precision);
    
    UNITY_END();
}

void loop() {
    // Rien ici
}
