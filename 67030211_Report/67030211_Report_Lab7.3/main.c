#include <stdio.h>
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

void app_main(void) {
    wifi_prov_mgr_config_t config = { .scheme = wifi_prov_scheme_ble };
    wifi_prov_mgr_init(config);
    printf("BLE GATT Provisioning Service registered (UUID: 0000ffff-...)\n");
}
