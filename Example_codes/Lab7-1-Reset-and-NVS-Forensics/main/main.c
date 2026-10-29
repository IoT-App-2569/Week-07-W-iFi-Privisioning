#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

/* ---- เปลี่ยนจาก wifi_provisioning -> network_provisioning (ESP-IDF 6.0) ---- */
#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_softap.h"

static const char *TAG = "LAB7_1_RESET";

#define FACTORY_RESET_BUTTON_GPIO  GPIO_NUM_18   // ปุ่ม Factory Reset (Active-Low ต่อลง GND)
#define LED_PIN_WIFI_STA           GPIO_NUM_2    // LED 1: สถานะ Wi-Fi Station (Onboard/External)
#define GRACE_PERIOD_SEC           5             // เวลารอให้ผู้ใช้กดปุ่มหลังบอร์ดบูต (สำหรับทดสอบสะดวกขึ้น)

typedef enum {
    LED_STA_OFF = 0,
    LED_STA_CONNECTED,       // Heartbeat: ติด 200ms ในทุก 1 วินาที
    LED_STA_DISCONNECTED     // Alert: ติด 200ms ดับ 200ms
} led_mode_t;

static volatile led_mode_t g_led_mode = LED_STA_OFF;

/* FreeRTOS Background Task สำหรับกระพริบไฟ LED 1 */
static void led_status_task(void *pvParameters)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN_WIFI_STA),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    uint32_t tick = 0;
    while (1) {
        if (g_led_mode == LED_STA_CONNECTED) {
            // Heartbeat: ติด 200ms / ดับ 800ms
            gpio_set_level(LED_PIN_WIFI_STA, (tick % 10 < 2) ? 1 : 0);
        } else if (g_led_mode == LED_STA_DISCONNECTED) {
            // Alert: ติด 200ms / ดับ 200ms
            gpio_set_level(LED_PIN_WIFI_STA, (tick % 4 < 2) ? 1 : 0);
        } else {
            gpio_set_level(LED_PIN_WIFI_STA, 0);
        }
        tick++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ตรวจสอบการกดปุ่ม GPIO 18 ค้างไว้ 3 วินาที (Active Low) */
static bool check_factory_reset_button(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << FACTORY_RESET_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Waiting %d seconds for button press... (press and hold GPIO 18 now)", GRACE_PERIOD_SEC);

    /* ช่วงรอให้ผู้ใช้กดปุ่มลง (Grace Period) เผื่อเวลาให้ผู้ใช้อ่าน Log แล้วค่อยกด
       ระหว่างนี้ถ้ายังไม่กดปุ่ม (ค่ายังเป็น HIGH) จะวนรอไปเรื่อยๆ จนครบเวลา */
    int grace_count = 0;
    bool button_pressed = false;
    while (grace_count < (GRACE_PERIOD_SEC * 10)) {
        if (gpio_get_level(FACTORY_RESET_BUTTON_GPIO) == 0) {
            button_pressed = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        grace_count++;
    }

    if (!button_pressed) {
        ESP_LOGI(TAG, "No button press detected. Continuing normal boot.");
        return false;
    }

    ESP_LOGI(TAG, "Button pressed! Now hold it for 3 seconds to confirm Factory Reset...");
    int hold_count = 0;
    while (gpio_get_level(FACTORY_RESET_BUTTON_GPIO) == 0) {
        vTaskDelay(pdMS_TO_TICKS(100));
        hold_count++;
        if (hold_count % 10 == 0) {
            ESP_LOGI(TAG, "Holding button... %d/3 seconds", hold_count / 10);
        }
        if (hold_count >= 30) {
            ESP_LOGW(TAG, "=================================================");
            ESP_LOGW(TAG, ">>> FACTORY RESET TRIGGERED! ERASING NVS FLASH <<<");
            ESP_LOGW(TAG, "=================================================");
            return true;
        }
    }
    ESP_LOGW(TAG, "Button released before 3 seconds. Factory reset cancelled.");
    return false;
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        g_led_mode = LED_STA_DISCONNECTED;
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi Disconnected. Connecting again...");
        g_led_mode = LED_STA_DISCONNECTED;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "=================================================");
        ESP_LOGI(TAG, "[ONLINE]: Connected with IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "=================================================");
        g_led_mode = LED_STA_CONNECTED;
    }
}

void app_main(void)
{
    xTaskCreate(led_status_task, "led_task", 2048, NULL, 5, NULL);

    // 1. ตรวจสอบการกดปุ่ม GPIO 18 ทางกายภาพ
    if (check_factory_reset_button()) {
        ESP_LOGW(TAG, "[FORENSIC]: User requested Flash Erase!");
        ESP_ERROR_CHECK(nvs_flash_erase());
    }

    // 2. เริ่มต้นระบบ NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 3. เริ่มต้น Network Interface และ Event Loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // 4. ตรวจสอบสถานะว่าเคย Provision มาก่อนหรือไม่
    // ---- เปลี่ยนชื่อ type/struct/ฟังก์ชันจาก wifi_prov_xx -> network_prov_xx ----
    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_softap, // default scheme
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE
    };
    ESP_ERROR_CHECK(network_prov_mgr_init(config));

    bool provisioned = false;
    // wifi_prov_mgr_is_provisioned -> network_prov_mgr_is_wifi_provisioned
    ESP_ERROR_CHECK(network_prov_mgr_is_wifi_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGW(TAG, "--------------------------------------------------");
        ESP_LOGW(TAG, "[STATUS]: Device is NOT provisioned (NVS is empty)");
        ESP_LOGW(TAG, "Ready for Provisioning Lab 7-2 (SoftAP) or 7-3 (BLE)!");
        ESP_LOGW(TAG, "--------------------------------------------------");
        network_prov_mgr_deinit();
    } else {
        ESP_LOGI(TAG, "--------------------------------------------------");
        ESP_LOGI(TAG, "[STATUS]: Already provisioned! Starting Wi-Fi Station");
        ESP_LOGI(TAG, "--------------------------------------------------");
        network_prov_mgr_deinit();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}