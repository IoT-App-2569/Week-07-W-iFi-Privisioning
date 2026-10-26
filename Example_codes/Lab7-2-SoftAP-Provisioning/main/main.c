#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_softap.h"


static const char *TAG = "LAB7_2_SOFTAP";


/* =========================================================
 * GPIO Configuration
 * ========================================================= */

#define LED_PIN_WIFI_STA     GPIO_NUM_2    // LED 1
#define LED_PIN_SOFTAP_PROV  GPIO_NUM_5    // LED 3

/* Proof of Possession */
#define PROV_POP_KEY         "abcd1234"


/* =========================================================
 * QR Code Configuration
 * ========================================================= */

#define PROV_QR_VERSION      "v1"
#define PROV_TRANSPORT       "softap"
#define QRCODE_BASE_URL      "https://espressif.github.io/esp-jumpstart/qrcode.html"


/* =========================================================
 * LED Task
 * ========================================================= */

static void led_task(void *pvParameters)
{
    gpio_config_t io_conf = {
        .pin_bit_mask =
            (1ULL << LED_PIN_WIFI_STA) |
            (1ULL << LED_PIN_SOFTAP_PROV),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    /* Initial LED state */
    gpio_set_level(LED_PIN_WIFI_STA, 0);
    gpio_set_level(LED_PIN_SOFTAP_PROV, 0);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


/* =========================================================
 * Network Provisioning Event Handler
 * ========================================================= */

static void network_prov_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base != NETWORK_PROV_EVENT) {
        return;
    }

    switch (event_id) {

        /* ---------------------------------------------
         * Provisioning Started
         * --------------------------------------------- */
        case NETWORK_PROV_START:

            ESP_LOGI(
                TAG,
                "[PROV EVENT]: SoftAP Provisioning Started!"
            );

            /* LED 3 ON = waiting for provisioning */
            gpio_set_level(
                LED_PIN_SOFTAP_PROV,
                1
            );

            break;


        /* ---------------------------------------------
         * Wi-Fi Credentials Received
         * --------------------------------------------- */
        case NETWORK_PROV_WIFI_CRED_RECV:
        {
            wifi_sta_config_t *sta_cfg =
                (wifi_sta_config_t *)event_data;

            ESP_LOGI(
                TAG,
                "================================================="
            );

            ESP_LOGI(
                TAG,
                "[CREDENTIALS RECEIVED]"
            );

            ESP_LOGI(
                TAG,
                "  -> Target SSID     : %s",
                (const char *)sta_cfg->ssid
            );

            ESP_LOGI(
                TAG,
                "  -> Target Password : %s",
                (const char *)sta_cfg->password
            );

            ESP_LOGI(
                TAG,
                "================================================="
            );

            break;
        }


        /* ---------------------------------------------
         * Wi-Fi Credentials Failed
         * --------------------------------------------- */
        case NETWORK_PROV_WIFI_CRED_FAIL:
        {
            network_prov_wifi_sta_fail_reason_t *reason =
                (network_prov_wifi_sta_fail_reason_t *)event_data;

            if (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) {

                ESP_LOGE(
                    TAG,
                    "[ERROR]: Wi-Fi authentication failed!"
                );

                ESP_LOGE(
                    TAG,
                    "Reason: Incorrect SSID password"
                );

            } else {

                ESP_LOGE(
                    TAG,
                    "[ERROR]: Wi-Fi Access Point not found!"
                );
            }

            break;
        }


        /* ---------------------------------------------
         * Wi-Fi Credentials Success
         * --------------------------------------------- */
        case NETWORK_PROV_WIFI_CRED_SUCCESS:

            ESP_LOGI(
                TAG,
                "[SUCCESS]: Provisioning Completed Successfully!"
            );

            /* LED 3 OFF = provisioning finished */
            gpio_set_level(
                LED_PIN_SOFTAP_PROV,
                0
            );

            break;


        /* ---------------------------------------------
         * Provisioning End
         * --------------------------------------------- */
        case NETWORK_PROV_END:

            ESP_LOGI(
                TAG,
                "[PROV EVENT]: De-initializing Provisioning Manager"
            );

            /*
             * network_prov_mgr_deinit()
             * releases provisioning resources.
             */
            ESP_ERROR_CHECK(
                network_prov_mgr_deinit()
            );

            break;


        default:
            break;
    }
}


/* =========================================================
 * Wi-Fi Event Handler
 * ========================================================= */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base != WIFI_EVENT) {
        return;
    }

    switch (event_id) {

        /* ---------------------------------------------
         * Mobile Phone Connected to ESP32 SoftAP
         * --------------------------------------------- */
        case WIFI_EVENT_AP_STACONNECTED:

            ESP_LOGI(
                TAG,
                "[SOFTAP]: Mobile Phone connected to ESP32 SoftAP!"
            );

            break;


        /* ---------------------------------------------
         * Mobile Phone Disconnected
         * --------------------------------------------- */
        case WIFI_EVENT_AP_STADISCONNECTED:

            ESP_LOGW(
                TAG,
                "[SOFTAP]: Mobile Phone disconnected from ESP32 SoftAP"
            );

            break;


        /* ---------------------------------------------
         * Station Started
         * --------------------------------------------- */
        case WIFI_EVENT_STA_START:

            ESP_LOGI(
                TAG,
                "[STA]: Wi-Fi Station started"
            );

            break;


        /* ---------------------------------------------
         * Station Disconnected
         * --------------------------------------------- */
        case WIFI_EVENT_STA_DISCONNECTED:

            ESP_LOGW(
                TAG,
                "[STA]: Wi-Fi Station disconnected"
            );

            break;


        default:
            break;
    }
}


/* =========================================================
 * IP Event Handler
 * ========================================================= */

static void ip_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP
    ) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(
            TAG,
            "================================================="
        );

        ESP_LOGI(
            TAG,
            "[ONLINE]: Got IP: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        ESP_LOGI(
            TAG,
            "================================================="
        );

        /*
         * LED 1 ON = ESP32 connected to target Wi-Fi
         */
        gpio_set_level(
            LED_PIN_WIFI_STA,
            1
        );
    }
}


/* =========================================================
 * app_main
 * ========================================================= */

void app_main(void)
{
    /* -----------------------------------------------------
     * Initialize NVS
     * ----------------------------------------------------- */

    esp_err_t ret = nvs_flash_init();

    if (
        ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND
    ) {

        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        ESP_ERROR_CHECK(
            nvs_flash_init()
        );
    }


    /* -----------------------------------------------------
     * Initialize TCP/IP stack
     * ----------------------------------------------------- */

    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    /* -----------------------------------------------------
     * Create default event loop
     * ----------------------------------------------------- */

    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );


    /* -----------------------------------------------------
     * Register Event Handlers
     * ----------------------------------------------------- */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            NETWORK_PROV_EVENT,
            ESP_EVENT_ANY_ID,
            &network_prov_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &ip_event_handler,
            NULL
        )
    );


    /* -----------------------------------------------------
     * Create Wi-Fi Interfaces
     * ----------------------------------------------------- */

    esp_netif_create_default_wifi_sta();

    esp_netif_create_default_wifi_ap();


    /* -----------------------------------------------------
     * Initialize Wi-Fi
     * ----------------------------------------------------- */

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    /* -----------------------------------------------------
     * Start LED Task
     * ----------------------------------------------------- */

    xTaskCreate(
        led_task,
        "led_task",
        2048,
        NULL,
        5,
        NULL
    );


    /* -----------------------------------------------------
     * Configure Network Provisioning
     * ----------------------------------------------------- */

    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_softap,
        .scheme_event_handler =
            NETWORK_PROV_EVENT_HANDLER_NONE
    };


    /* -----------------------------------------------------
     * Initialize Provisioning Manager
     * ----------------------------------------------------- */

    ESP_ERROR_CHECK(
        network_prov_mgr_init(config)
    );


    /* -----------------------------------------------------
     * Check whether device is already provisioned
     * ----------------------------------------------------- */

    bool provisioned = false;

    ESP_ERROR_CHECK(
        network_prov_mgr_is_wifi_provisioned(
            &provisioned
        )
    );


    /* =====================================================
     * NOT PROVISIONED
     * ===================================================== */

    if (!provisioned) {

        /* -------------------------------------------------
         * Generate SoftAP SSID from MAC Address
         * ------------------------------------------------- */

        uint8_t mac[6];

        ESP_ERROR_CHECK(
            esp_wifi_get_mac(
                WIFI_IF_STA,
                mac
            )
        );

        char service_name[16];

        snprintf(
            service_name,
            sizeof(service_name),
            "PROV_%02X%02X%02X",
            mac[3],
            mac[4],
            mac[5]
        );


        ESP_LOGI(
            TAG,
            "================================================="
        );

        ESP_LOGI(
            TAG,
            "Starting SoftAP Provisioning"
        );

        ESP_LOGI(
            TAG,
            "SSID : %s",
            service_name
        );

        ESP_LOGI(
            TAG,
            "PoP  : %s",
            PROV_POP_KEY
        );

        ESP_LOGI(
            TAG,
            "================================================="
        );


        /* -------------------------------------------------
         * Security 1
         * ------------------------------------------------- */

        network_prov_security_t security =
            NETWORK_PROV_SECURITY_1;

        network_prov_security1_params_t *sec_params =
            (network_prov_security1_params_t *)PROV_POP_KEY;


        /* -------------------------------------------------
         * Start Provisioning
         * ------------------------------------------------- */

        ESP_ERROR_CHECK(
            network_prov_mgr_start_provisioning(
                security,
                sec_params,
                service_name,
                NULL
            )
        );


        /* -------------------------------------------------
         * Print QR Code URL
         * ------------------------------------------------- */

        ESP_LOGI(
            TAG,
            "--------------------------------------------------"
        );

        ESP_LOGI(
            TAG,
            "[QR CODE URL]:"
        );

        ESP_LOGI(
            TAG,
            "%s?data={\"ver\":\"%s\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
            QRCODE_BASE_URL,
            PROV_QR_VERSION,
            service_name,
            PROV_POP_KEY,
            PROV_TRANSPORT
        );


        /* -------------------------------------------------
         * Print JSON Payload
         * ------------------------------------------------- */

        ESP_LOGI(
            TAG,
            "[Payload JSON]"
        );

        ESP_LOGI(
            TAG,
            "{\"ver\":\"%s\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
            PROV_QR_VERSION,
            service_name,
            PROV_POP_KEY,
            PROV_TRANSPORT
        );

        ESP_LOGI(
            TAG,
            "--------------------------------------------------"
        );

    }


    /* =====================================================
     * ALREADY PROVISIONED
     * ===================================================== */

    else {

        ESP_LOGI(
            TAG,
            "Already provisioned!"
        );

        ESP_LOGI(
            TAG,
            "Starting Wi-Fi Station..."
        );


        /*
         * Provisioning manager will be stopped after
         * successful provisioning.
         */

        ESP_ERROR_CHECK(
            esp_wifi_set_mode(WIFI_MODE_STA)
        );

        ESP_ERROR_CHECK(
            esp_wifi_start()
        );
    }


    /* -----------------------------------------------------
     * Keep app_main alive
     * ----------------------------------------------------- */

    while (1) {

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}