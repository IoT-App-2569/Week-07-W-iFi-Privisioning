idf_component_register(SRCS "main.c"
                        PRIV_REQUIRES esp_wifi nvs_flash wifi_provisioning driver bt
                        INCLUDE_DIRS ".")
