#ifndef CONFIG_H
#define CONFIG_H

// Activer/désactiver les modules
#define USE_ADC        1
#define USE_MOPEKA     1
#define USE_LITIME     1
#define USE_VICTRON    1
#define USE_WEB        0
#define USE_WIFI       0
#define USE_DISPLAY    1

// Module ADC
#define ADC_AVG_SAMPLES 1 //Was 10
#define ADC_READ_INTERVAL 5000

// Module Wifi
#define WIFI_SSID           "zone"
#define WIFI_PASSWORD       "fleurdelys"
#define WIFI_AP_NAME        "Airstream-Monitor"
#define WIFI_AP_PASSWORD    "fleurdelys"

// --- Adresses MAC Bluetooth (Hardcoded) ---

// Batteries LiTime
#define USE_LITIME_1 1
#define USE_LITIME_2 0
#define LITIME_MAC_1  "C8:47:80:18:C1:17"
#define LITIME_MAC_2  "AA:BB:CC:DD:EE:02"

// Capteurs Propane Mopeka
#define USE_MOPEKA_L 0
#define USE_MOPEKA_R 1
#define USE_MOPEKA_AUX 0
#define MOPEKA_MAC_L  "XX:XX:XX:XX:XX:L1"
#define MOPEKA_MAC_R  "D4:FB:45:CA:03:EC"
#define MOPEKA_MAC_AUX  "XX:XX:XX:XX:XX:L2"

// Chargeur Victron Orion
#define USE_VICTRON_1 1
#define VICTRON_MAC   "F5:81:CE:F9:B9:3A"

#define MAX_PROPANE 3
#define MAX_TANKS 3
#define MAX_BATTERIES 2
#define MAX_CHARGERS 1

#endif