#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define FACTORY_RESET_BUTTON_GPIO GPIO_NUM_18
#define FACTORY_RESET_HOLD_TIME_MS 3000
#define FACTORY_RESET_CHECK_INTERVAL_MS 100

static const char *TAG_FACTORY_RESET = "FACTORY_RESET";

/* ประกาศฟังก์ชันก่อน app_main */
static bool check_factory_reset_button(void);

/**
 * ตรวจสอบว่ากดปุ่ม Factory Reset ค้างไว้ 3 วินาทีหรือไม่
 *
 * GPIO 18 ต่อกับปุ่มกดภายนอก และอีกด้านต่อ GND
 * HIGH = ไม่กดปุ่ม
 * LOW  = กดปุ่ม
 */
static bool check_factory_reset_button(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << FACTORY_RESET_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_config(&io_conf);

    if (err != ESP_OK) {
        ESP_LOGE(TAG_FACTORY_RESET,
                 "GPIO configuration failed: %s",
                 esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG_FACTORY_RESET,
             "Hold GPIO 18 button for 3 seconds to reset");

    /* ไม่กดปุ่ม ไม่ต้อง Reset */
    if (gpio_get_level(FACTORY_RESET_BUTTON_GPIO) != 0) {
        ESP_LOGI(TAG_FACTORY_RESET, "Button not pressed");
        return false;
    }

    int elapsed_ms = 0;

    while (elapsed_ms < FACTORY_RESET_HOLD_TIME_MS) {
        vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_CHECK_INTERVAL_MS));
        elapsed_ms += FACTORY_RESET_CHECK_INTERVAL_MS;

        /* ปล่อยปุ่มก่อนครบ 3 วินาที ยกเลิกการ Reset */
        if (gpio_get_level(FACTORY_RESET_BUTTON_GPIO) != 0) {
            ESP_LOGI(TAG_FACTORY_RESET,
                     "Button released; Factory Reset cancelled");
            return false;
        }

        if (elapsed_ms % 1000 == 0) {
            ESP_LOGI(TAG_FACTORY_RESET,
                     "Holding button... %d/3 seconds",
                     elapsed_ms / 1000);
        }
    }

    ESP_LOGW(TAG_FACTORY_RESET, "FACTORY RESET TRIGGERED");

    return true;
}

/**
 * ฟังก์ชันหลักของ ESP-IDF
 */
void app_main(void)
{
    esp_err_t ret;

    /* ตรวจสอบปุ่มก่อน Initialize NVS */
    if (check_factory_reset_button()) {
        ESP_LOGW(TAG_FACTORY_RESET, "Erasing NVS...");

        ret = nvs_flash_erase();

        if (ret != ESP_OK) {
            ESP_LOGE(TAG_FACTORY_RESET,
                     "Failed to erase NVS: %s",
                     esp_err_to_name(ret));
            return;
        }

        ESP_LOGW(TAG_FACTORY_RESET, "NVS erased successfully");
    }

    /* Initialize NVS */
    ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_LOGW(TAG_FACTORY_RESET,
                 "NVS initialization requires erase; erasing NVS");

        ret = nvs_flash_erase();

        if (ret != ESP_OK) {
            ESP_LOGE(TAG_FACTORY_RESET,
                     "Failed to erase NVS: %s",
                     esp_err_to_name(ret));
            return;
        }

        ret = nvs_flash_init();
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG_FACTORY_RESET,
                 "NVS initialization failed: %s",
                 esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG_FACTORY_RESET, "NVS initialized successfully");

    /*
     * TODO:
     * นำโค้ด Wi-Fi Provisioning เดิมของ Lab7-1
     * มาใส่ต่อจากตรงนี้
     
     * ต้องนำโค้ดเดิมที่ตรวจสอบ Credentials
     * และเริ่ม Wi-Fi Provisioning กลับมาให้ครบ
     */
}