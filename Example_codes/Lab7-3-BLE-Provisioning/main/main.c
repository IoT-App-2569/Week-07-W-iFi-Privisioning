
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "protocomm.h"

static const char *TAG = "LAB7_4_CUSTOM";

#define LED_PIN_WIFI_STA GPIO_NUM_2
#define LED_PIN_BLE_PROV GPIO_NUM_4
#define PROV_POP_KEY     "abcd1234"
#define CUSTOM_ENDPOINT  "custom-data"

/* Custom Endpoint Handler */
static esp_err_t custom_prov_data_handler(
    uint32_t session_id,
    const uint8_t *inbuf,
    ssize_t inlen,
    uint8_t **outbuf,
    ssize_t *outlen,
    void *priv_data)
{
    (void)session_id;
    (void)priv_data;

    if (inbuf != NULL && inlen > 0) {
        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, "[CUSTOM DATA RECEIVED]: %.*s",
                 (int)inlen, (const char *)inbuf);
        ESP_LOGI(TAG, "========================================");
    }

    const char response[] = "ACK_FROM_ESP32_SUCCESS";

    *outbuf = (uint8_t *)strdup(response);

    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "Heap out of memory");
        return ESP_ERR_NO_MEM;
    }

    *outlen = strlen(response) + 1;

    ESP_LOGI(TAG, "[CUSTOM DATA RESPONSE]: %s", response);

    return ESP_OK;
}

/* Event Handler */
static void event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    (void)arg;

    if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {

        if (event_id == PROTOCOMM_SECURITY_SESSION_SETUP_OK) {
            ESP_LOGI(TAG,
                     "[SECURITY SUCCESS]: Valid PoP! Secured Session OK!");

        } else if (
            event_id ==
            PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH) {

            ESP_LOGE(TAG,
                     "[SECURITY ALERT]: INVALID PoP / Unauthorized Access!");
        }

    } else if (event_base == WIFI_PROV_EVENT) {

        if (event_id == WIFI_PROV_START) {
            ESP_LOGI(TAG, "[PROVISIONING]: BLE Provisioning Started");
            gpio_set_level(LED_PIN_BLE_PROV, 1);

        } else if (event_id == WIFI_PROV_CRED_RECV) {
            wifi_sta_config_t *sta_cfg =
                (wifi_sta_config_t *)event_data;

            ESP_LOGI(TAG, "[WIFI CREDENTIALS RECEIVED]");
            ESP_LOGI(TAG, "SSID: %s", (char *)sta_cfg->ssid);

        } else if (event_id == WIFI_PROV_CRED_SUCCESS) {
            ESP_LOGI(TAG, "[SUCCESS]: Provisioning Completed");
            gpio_set_level(LED_PIN_BLE_PROV, 0);

        } else if (event_id == WIFI_PROV_CRED_FAIL) {
            ESP_LOGE(TAG, "[ERROR]: Wi-Fi Provisioning Failed");

        } else if (event_id == WIFI_PROV_END) {
            ESP_LOGI(TAG, "[PROVISIONING]: De-initializing Manager");
            gpio_set_level(LED_PIN_BLE_PROV, 0);
            wifi_prov_mgr_deinit();
        }

    } else if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG,
                 "[ONLINE]: IP Address: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        gpio_set_level(LED_PIN_WIFI_STA, 1);
    }
}

void app_main(void)
{
    /* Configure status LEDs */
    gpio_config_t io_conf = {
        .pin_bit_mask =
            (1ULL << LED_PIN_WIFI_STA) |
            (1ULL << LED_PIN_BLE_PROV),
        .mode = GPIO_MODE_OUTPUT,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(LED_PIN_WIFI_STA, 0);
    gpio_set_level(LED_PIN_BLE_PROV, 0);

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /* Initialize networking */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register event handlers */
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_PROV_EVENT,
            ESP_EVENT_ANY_ID,
            event_handler,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            PROTOCOMM_SECURITY_SESSION_EVENT,
            ESP_EVENT_ANY_ID,
            event_handler,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            event_handler,
            NULL));

    /* Initialize Wi-Fi */
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Initialize BLE Provisioning Manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler =
            WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    /*
     * LAB TEST ONLY:
     * Clear previous Wi-Fi provisioning credentials.
     * Remove this line after completing the lab tests.
     */
    ESP_ERROR_CHECK(wifi_prov_mgr_reset_provisioning());

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {

        /* Generate BLE service name from MAC address */
        uint8_t mac[6];
        ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));

        char service_name[16];

        snprintf(
            service_name,
            sizeof(service_name),
            "PROV_%02X%02X%02X",
            mac[3], mac[4], mac[5]);

        /* Security 1 with Proof-of-Possession */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = PROV_POP_KEY;

        /* Create custom endpoint BEFORE starting provisioning */
        ESP_ERROR_CHECK(
            wifi_prov_mgr_endpoint_create(CUSTOM_ENDPOINT));

        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, "[LAB 7.4] Starting BLE Provisioning");
        ESP_LOGI(TAG, "Device name: %s", service_name);
        ESP_LOGI(TAG, "Security: 1");
        ESP_LOGI(TAG, "PoP: %s", pop);
        ESP_LOGI(TAG, "========================================");

        /* Start BLE provisioning */
        ESP_ERROR_CHECK(
            wifi_prov_mgr_start_provisioning(
                security,
                (const void *)pop,
                service_name,
                NULL));

        /* Register custom endpoint AFTER starting provisioning */
        ESP_ERROR_CHECK(
            wifi_prov_mgr_endpoint_register(
                CUSTOM_ENDPOINT,
                custom_prov_data_handler,
                NULL));

        ESP_LOGI(TAG, "[CUSTOM ENDPOINT READY]: %s",
                 CUSTOM_ENDPOINT);

        ESP_LOGI(
            TAG,
            "Payload JSON: "
            "{\"ver\":\"v1\",\"name\":\"%s\","
            "\"pop\":\"%s\",\"transport\":\"ble\"}",
            service_name,
            pop);

    } else {

        ESP_LOGI(TAG,
                 "Already provisioned! Starting Wi-Fi Station");

        ESP_ERROR_CHECK(wifi_prov_mgr_deinit());

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}
