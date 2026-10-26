#include <stdio.h>
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

void app_main(void) {
    wifi_prov_mgr_config_t config = { .scheme = wifi_prov_scheme_softap };
    wifi_prov_mgr_init(config);
    printf("SoftAP Provisioning HTTP endpoints initialized.\n");
}
