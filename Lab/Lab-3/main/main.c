#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_bt.h>

#include <network_provisioning/manager.h>
#include <network_provisioning/scheme_ble.h>

static const char *TAG = "LAB7_3_BLE";

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to AP...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

void app_main(void)
{
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize TCP/IP and Event Loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // 3. Initialize Wi-Fi Stack in Station mode
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 4. Initialize Network Provisioning Manager for v6.0 (กำหนดค่าคอนฟิกเริ่มต้นให้ถูกต้อง)
    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_ble,
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BT
    };
    ESP_ERROR_CHECK(network_prov_mgr_init(config));

    bool provisioned = false;
    ESP_ERROR_CHECK(network_prov_mgr_is_wifi_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting BLE provisioning...");

        network_prov_security_t security = NETWORK_PROV_SECURITY_2;
        const char *pop = "abcd1234";

        char service_name[12];
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        snprintf(service_name, sizeof(service_name), "PROV_%02X%02X", mac[4], mac[5]);

        // เริ่มต้น Provisioning ผ่าน Scheme BLE
        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, (const void *)pop, service_name, NULL));
        
        ESP_LOGI(TAG, "BLE Provisioning started. Service Name: %s", service_name);
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi station...");
        network_prov_mgr_deinit();
        esp_wifi_start();
    }

    // 5. รอจนกว่าจะเชื่อมต่อ Wi-Fi สำเร็จ
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    
    ESP_LOGI(TAG, "Provisioning successful & Wi-Fi connected!");

    // ปลดปล่อยหน่วยความจำ Bluetooth และ BTDM คืนสู่ระบบ (RAM Optimization)
    ESP_ERROR_CHECK(esp_bt_mem_release(ESP_BT_MODE_BLE));
    ESP_LOGI(TAG, "wifi_prov_scheme_ble: BT memory released");
    ESP_LOGI(TAG, "wifi_prov_scheme_ble: BTDM memory released");
}