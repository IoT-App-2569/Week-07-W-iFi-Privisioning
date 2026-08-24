#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_softap.h"
#include "network_provisioning/network_config.h"

static const char *TAG = "LAB7_2_SOFTAP";

#define FACTORY_RESET_BUTTON_GPIO  GPIO_NUM_18
#define LED_STA_PIN                GPIO_NUM_2  
#define LED_PROV_PIN               GPIO_NUM_5  

static void configure_leds(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_STA_PIN) | (1ULL << LED_PROV_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_STA_PIN, 0);
    gpio_set_level(LED_PROV_PIN, 0);
}

static void check_factory_reset(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << FACTORY_RESET_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    if (gpio_get_level(FACTORY_RESET_BUTTON_GPIO) == 0) {
        ESP_LOGW(TAG, "Hardware Reset Button Pressed! Erasing NVS...");
        nvs_flash_erase();
    }
}

// อัปเดต Event Handler ให้รองรับ _WIFI_ ของ v6.0
static void prov_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == NETWORK_PROV_EVENT) { 
        switch (event_id) {
            case NETWORK_PROV_START:
                ESP_LOGI(TAG, "Provisioning Started! (SoftAP Mode)");
                break;
            case NETWORK_PROV_WIFI_CRED_RECV: { // อัปเดตเป็น NETWORK_PROV_WIFI_CRED_RECV
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials");
                ESP_LOGI(TAG, "\tSSID     : %s", (const char *) wifi_sta_cfg->ssid);
                ESP_LOGI(TAG, "\tPassword : %s", (const char *) wifi_sta_cfg->password);
                break;
            }
            case NETWORK_PROV_WIFI_CRED_FAIL: { // อัปเดตเป็น NETWORK_PROV_WIFI_CRED_FAIL
                network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed! Reason: %s",
                         (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) ? "Wi-Fi Password Incorrect" : "AP Not Found");
                break;
            }
            case NETWORK_PROV_WIFI_CRED_SUCCESS: // อัปเดตเป็น NETWORK_PROV_WIFI_CRED_SUCCESS
                ESP_LOGI(TAG, "Provisioning successful!");
                break;
            case NETWORK_PROV_END:
                ESP_LOGI(TAG, "Provisioning End. Closing SoftAP.");
                network_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_AP_STACONNECTED) {
            ESP_LOGI(TAG, "SoftAP transport: Smartphone Connected!");
            gpio_set_level(LED_PROV_PIN, 1);
        } else if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGW(TAG, "Wi-Fi Disconnected. Connecting again...");
            gpio_set_level(LED_STA_PIN, 0);
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        gpio_set_level(LED_STA_PIN, 1); 
    }
}

void app_main(void)
{
    configure_leds();
    check_factory_reset();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &prov_event_handler, NULL));

    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_softap,
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE
    };
    ESP_ERROR_CHECK(network_prov_mgr_init(config));

    bool provisioned = false;
    ESP_ERROR_CHECK(network_prov_mgr_is_wifi_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting Provisioning (SoftAP)...");
        
        char service_name[12];
        uint8_t eth_mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
        snprintf(service_name, sizeof(service_name), "PROV_%02X%02X%02X", eth_mac[3], eth_mac[4], eth_mac[5]);

// 1. กำหนดรูปแบบความปลอดภัยเป็น SECURITY_2
        network_prov_security_t security = NETWORK_PROV_SECURITY_2; 
        const char *pop = "abcd1234";

        // 2. ใช้โครงสร้าง Sec2 แบบระบุ PoP โดยตรงผ่าน Sec Params (ปล่อย Salt/Verifier ให้ระบบสร้างให้อัตโนมัติด้วยโครงสร้างมาตรฐาน v6.0)
        // สำหรับ ESP-IDF v6.0.2 หากใช้ Security 2 เราต้องส่งพารามิเตอร์แบบกำหนดค่าว่างเพื่อให้ระบบสร้างตัวสร้างคีย์ชั่วคราว
// 2. กำหนดโครงสร้างพารามิเตอร์สำหรับ Security 2 
        network_prov_security2_params_t sec2_params = {
            .salt = (const char *)"salt_string_123", 
            .salt_len = 15,
            .verifier = NULL,
            .verifier_len = 0
        };

        const void *sec_params = (const void *)&sec2_params;

        // 3. QR Payload ที่ถูกต้อง (ตรงกับลิงก์ที่เราเปิด)
        char payload[180];
        snprintf(payload, sizeof(payload), "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"security\":2,\"transport\":\"softap\"}", service_name, pop);
        
        ESP_LOGI(TAG, "=======================================================");
        ESP_LOGI(TAG, "Scan this QR code from the provisioning application.");
        ESP_LOGI(TAG, "https://espressif.github.io/esp-jumpstart/qrcode.html?data=%s", payload);
        ESP_LOGI(TAG, "=======================================================");

        // 4. เริ่มต้น Provisioning ด้วย Security 2 และพารามิเตอร์ที่เติม Salt แล้ว
        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, &sec2_params, service_name, NULL));
        
        gpio_set_level(LED_PROV_PIN, 1); 
        // network_prov_mgr_endpoint_create("custom-data");
    } else {
        ESP_LOGI(TAG, "Already provisioned! Starting Wi-Fi Station.");
        network_prov_mgr_deinit();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}