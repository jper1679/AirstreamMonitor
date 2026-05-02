/**
 * @file lv_conf.h
 * Configuration pour LVGL 8.4.0 - Airstream Monitor
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* IMPORTANT: Active le contenu du fichier */
#if 1 

/*====================
   COLOR SETTINGS
 *====================*/

/* 16-bit RGB565 pour ton écran Waveshare */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* En v8, on utilise LV_MEM_CUSTOM pour laisser l'ESP32 gérer la RAM */
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
    /* Taille de la pile interne si on n'utilise pas malloc personnalisé */
    #define LV_MEM_SIZE (64U * 1024U)
    #define LV_MEM_ADR 0
#else
    /* Si tu veux utiliser la PSRAM pour LVGL lui-même */
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif

/*====================
   HAL SETTINGS
 *====================*/

#define LV_TICK_CUSTOM 0

/*====================
   DEFAULTS
 *====================*/

#define LV_DPI_DEF 130
#define LV_DISP_DEF_REFR_PERIOD 30      /* ms */
#define LV_INDEV_DEF_READ_PERIOD 30     /* ms */

/*====================
   FEATURES
 *====================*/

#define LV_USE_ANIMATION 1
#define LV_USE_SHADOW 1
#define LV_USE_OUTLINE 1
#define LV_USE_OPACITY 1
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/*====================
   FONTS
 *====================*/
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_FONT_DEFAULT &lv_font_montserrat_24


/*====================
   WIDGETS
 *====================*/

#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1
#define LV_USE_KEYBOARD   1
#define LV_USE_SPINBOX    1

/* Widgets Extra (pour tes jauges de tanks/batteries) */
#define LV_USE_METER      1
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    1
#define LV_USE_FLEX       1
#define LV_USE_GRID       1

#endif /*#if 1*/

#endif /*LV_CONF_H*/