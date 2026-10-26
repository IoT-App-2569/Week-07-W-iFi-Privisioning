idf_component_register(
    SRCS "main.c"
    PRIV_REQUIRES
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        network_provisioning
        driver
    INCLUDE_DIRS "."
)