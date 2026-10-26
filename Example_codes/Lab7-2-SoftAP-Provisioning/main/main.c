
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_softap.h"

static const char *TAG = "LAB7_2_SOFTAP";

#define LED_PIN_WIFI_STA     GPIO_NUM_2
#define LED_PIN_SOFTAP_PROV  GPIO_NUM_5
#define PROV_POP_KEY         "abcd1234"

static volatile bool s_prov_active = false;
static volatile bool s_sta_connected = false;

/* ตั้งค่า LED ทั้งสองดวง */
static void led_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN_WIFI_STA) |
                        (1ULL << LED_PIN_SOFTAP_PROV),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(LED_PIN_WIFI_STA, 0);
    gpio_set_level(LED_PIN_SOFTAP_PROV, 0);
}

/* อัปเดต LED ตามสถานะของระบบ */
static void led_task(void *pvParameters)
{
    (void)pvParameters;

    while (1) {
        gpio_set_level(LED_PIN_WIFI_STA, s_sta_connected ? 1 : 0);
        gpio_set_level(LED_PIN_SOFTAP_PROV, s_prov_active ? 1 : 0);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* สร้างชื่อ SoftAP จาก MAC Address */
static void get_device_service_name(char *service_name, size_t max_len)
{
    uint8_t mac[6] = {0};

    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));

    snprintf(service_name, max_len, "PROV_%02X%02X%02X",
             mac[3], mac[4], mac[5]);
}

/* จัดการ Event จาก Provisioning, Wi-Fi และ IP */
static void event_handler(void *arg,
                          esp_event_base_t event_base,
                          int32_t event_id,
                          void *event_data)
{
    (void)arg;

    if (event_base == NETWORK_PROV_EVENT) {
        switch (event_id) {

        case NETWORK_PROV_START:
            ESP_LOGI(TAG, "[PROV] SoftAP Provisioning Started");
            s_prov_active = true;
            break;

        case NETWORK_PROV_WIFI_CRED_RECV: {
            wifi_sta_config_t *sta_cfg =
                (wifi_sta_config_t *)event_data;

            ESP_LOGI(TAG, "====================================");
            ESP_LOGI(TAG, "[PROV] Wi-Fi Credentials Received");
            ESP_LOGI(TAG, "Target SSID: %.*s",
                     (int)sizeof(sta_cfg->ssid),
                     (const char *)sta_cfg->ssid);
            ESP_LOGI(TAG, "====================================");
            break;
        }

        case NETWORK_PROV_WIFI_CRED_FAIL: {
            network_prov_wifi_sta_fail_reason_t *reason =
                (network_prov_wifi_sta_fail_reason_t *)event_data;

            if (reason != NULL) {
                if (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) {
                    ESP_LOGE(TAG, "[PROV] Wi-Fi authentication failed");
                } else {
                    ESP_LOGE(TAG, "[PROV] Wi-Fi access point not found");
                }
            } else {
                ESP_LOGE(TAG, "[PROV] Wi-Fi connection failed");
            }
            break;
        }

        case NETWORK_PROV_WIFI_CRED_SUCCESS:
            ESP_LOGI(TAG, "[PROV] Credentials accepted successfully");
            s_prov_active = false;
            break;

        case NETWORK_PROV_END:
            ESP_LOGI(TAG, "[PROV] Provisioning ended");
            s_prov_active = false;

            ESP_ERROR_CHECK(network_prov_mgr_deinit());
            break;

        default:
            break;
        }
    }

    else if (event_base == WIFI_EVENT) {
        switch (event_id) {

    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "[Wi-Fi] Station started");
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        s_sta_connected = false;
        ESP_LOGW(TAG, "[Wi-Fi] Station disconnected");

        if (!s_prov_active) {
        esp_err_t err = esp_wifi_connect();

        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Wi-Fi reconnect returned: %s",
                     esp_err_to_name(err));
        }
    }
    break;

        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "[SoftAP] Mobile connected to ESP32");
            break;

        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "[SoftAP] Mobile disconnected from ESP32");
            break;

        default:
            break;
        }
    }

    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "====================================");
        ESP_LOGI(TAG, "[ONLINE] Got IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "====================================");

        s_sta_connected = true;
    }

    else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {
        switch (event_id) {

        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "[SECURITY] Secure session established");
            break;

        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "[SECURITY] Invalid security parameters");
            break;

        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "[SECURITY] Incorrect PoP");
            break;

        default:
            break;
        }
    }
}

void app_main(void)
{
    /* 1. Initialize NVS */
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(ret);
    }

    /* 2. Initialize LED */
    led_init();

    /* 3. Start LED status task */
    BaseType_t task_result = xTaskCreate(
        led_task,
        "led_task",
        2048,
        NULL,
        5,
        NULL
    );

    if (task_result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LED task");
        abort();
    }

    /* 4. Initialize network interfaces and event loop */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* 5. Register event handlers */
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            NETWORK_PROV_EVENT,
            ESP_EVENT_ANY_ID,
            event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            PROTOCOMM_SECURITY_SESSION_EVENT,
            ESP_EVENT_ANY_ID,
            event_handler,
            NULL
        )
    );

    /* 6. Create default STA and SoftAP network interfaces */
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    /* 7. Initialize Wi-Fi */
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    /* 8. Configure SoftAP provisioning manager */
    network_prov_mgr_config_t prov_cfg = {
        .scheme = network_prov_scheme_softap,
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE,
    };

    ESP_ERROR_CHECK(network_prov_mgr_init(prov_cfg));

    /* 9. Check whether Wi-Fi has already been provisioned */
    bool provisioned = false;

    ESP_ERROR_CHECK(
        network_prov_mgr_is_wifi_provisioned(&provisioned)
    );

    if (!provisioned) {
        /* 10. Create the SoftAP service name */
        char service_name[16] = {0};
        get_device_service_name(service_name, sizeof(service_name));

        /* 11. Configure Security 1 and Proof of Possession */
        network_prov_security_t security = NETWORK_PROV_SECURITY_1;
        const char *pop = PROV_POP_KEY;

        network_prov_security1_params_t *sec_params =
            (network_prov_security1_params_t *)pop;

        const char *service_key = NULL;

        ESP_LOGI(TAG, "====================================");
        ESP_LOGI(TAG, "Starting SoftAP Provisioning");
        ESP_LOGI(TAG, "SoftAP SSID: %s", service_name);
        ESP_LOGI(TAG, "Security: 1");
        ESP_LOGI(TAG, "PoP: %s", PROV_POP_KEY);
        ESP_LOGI(TAG, "====================================");

        /* 12. Start provisioning */
        ESP_ERROR_CHECK(
            network_prov_mgr_start_provisioning(
                security,
                (const void *)sec_params,
                service_name,
                service_key
            )
        );

        /* 13. Print QR-code payload and URL */
        ESP_LOGI(TAG, "QR Payload:");
        ESP_LOGI(TAG,
                 "{\"ver\":\"v1\",\"name\":\"%s\","
                 "\"pop\":\"%s\",\"transport\":\"softap\"}",
                 service_name, pop);

        ESP_LOGI(TAG, "QR Code URL:");
        ESP_LOGI(TAG,
                 "https://espressif.github.io/esp-jumpstart/"
                 "qrcode.html?data=%%7B%%22ver%%22%%3A%%22v1%%22"
                 "%%2C%%22name%%22%%3A%%22%s%%22"
                 "%%2C%%22pop%%22%%3A%%22%s%%22"
                 "%%2C%%22transport%%22%%3A%%22softap%%22%%7D",
                 service_name, pop);

        ESP_LOGI(TAG, "Connect your phone to the SoftAP above");
    } else {
        /* 14. Already provisioned: use Wi-Fi Station mode */
        ESP_LOGI(TAG, "Already provisioned; starting Wi-Fi STA");

        ESP_ERROR_CHECK(network_prov_mgr_deinit());

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}
