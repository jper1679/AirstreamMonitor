#include "ESP_Panel_Library.h"
#include "lvgl_v8_port.h"
#include "esp_timer.h"
#include "Arduino.h"

#define LCD_WIDTH  1024
#define LCD_HEIGHT 600

static SemaphoreHandle_t lvgl_mux = nullptr;
static TaskHandle_t lvgl_task_handle = nullptr;
static esp_timer_handle_t lvgl_tick_timer = NULL;
static void *lvgl_buf[2] = {nullptr, nullptr};

static void flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    ESP_PanelLcd *lcd = (ESP_PanelLcd *)drv->user_data;
    int w = area->x2 - area->x1 + 1;
    int h = area->y2 - area->y1 + 1;
    lcd->drawBitmap(area->x1, area->y1, w, h, (const uint8_t *)color_map);
    lv_disp_flush_ready(drv);
}

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    ESP_PanelLcdTouch *tp = (ESP_PanelLcdTouch *)indev_drv->user_data;
    
    tp->readData();
    // La v1.1.1 utilise l'objet TouchPoint
    auto point = tp->getPoint(0); 

    // On vérifie si les coordonnées sont valides (souvent -1 ou 0 si pas de touche)
    if (point.x >= 0 && point.x < LCD_WIDTH && point.y >= 0 && point.y < LCD_HEIGHT) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static lv_disp_t *display_init(ESP_PanelLcd *lcd) {
    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_drv_t disp_drv;

    // On récupère le bus et on le caste en RGB
    auto bus = static_cast<ESP_PanelBus_RGB *>(lcd->getBus());
    auto bus_config = bus->getRGBConfig();

    lvgl_buf[0] = nullptr;
    lvgl_buf[1] = nullptr;

    // Sécurité : si fb_list est vide, on alloue en PSRAM (fallback)
    if (lvgl_buf[0] == nullptr) {
        lvgl_buf[0] = heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    }

    uint32_t buffer_size = LCD_WIDTH * LCD_HEIGHT;
    lv_disp_draw_buf_init(&disp_buf, lvgl_buf[0], lvgl_buf[1], buffer_size);
    
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = flush_callback;
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = (void *)lcd;

    return lv_disp_drv_register(&disp_drv);
}

static lv_indev_t *indev_init(ESP_PanelLcdTouch *tp) {
    static lv_indev_drv_t indev_drv_tp;
    lv_indev_drv_init(&indev_drv_tp);
    indev_drv_tp.type = LV_INDEV_TYPE_POINTER;
    indev_drv_tp.read_cb = touchpad_read;
    indev_drv_tp.user_data = (void *)tp;
    return lv_indev_drv_register(&indev_drv_tp);
}

bool lvgl_port_lock(int timeout_ms) {
    if (!lvgl_mux) return false;
    TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE);
}

void lvgl_port_unlock(void) {
    if (lvgl_mux) xSemaphoreGiveRecursive(lvgl_mux);
}

static void tick_increment(void *arg) { lv_tick_inc(5); }

static void lvgl_port_task(void *arg) {
    while (1) {
        if (lvgl_port_lock(-1)) {
            lv_timer_handler();
            lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool lvgl_port_init(ESP_PanelLcd *lcd, ESP_PanelLcdTouch *tp) {
    lv_init();
    
    const esp_timer_create_args_t tick_args = { .callback = &tick_increment, .name = "LVGL tick" };
    esp_timer_create(&tick_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, 5000);

    display_init(lcd);
    if (tp != nullptr) indev_init(tp);

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    xTaskCreatePinnedToCore(lvgl_port_task, "lvgl", 8192, NULL, 5, &lvgl_task_handle, 1);
    
    return true;
}