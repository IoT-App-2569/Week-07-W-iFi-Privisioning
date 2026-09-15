#pragma once
#include "wifi_provisioning/manager.h"

#define PROV_QR_VERSION         "v1"
#define PROV_TRANSPORT_SOFTAP   "softap"
#define QRCODE_BASE_URL         "https://espressif.github.io/esp-jumpstart/qrcode.html"

void wifi_init_sta(void);
esp_err_t get_device_service_name(char *service_name, size_t max);
void wifi_prov_print_qr(const char *name, const char *pop, const char *transport);