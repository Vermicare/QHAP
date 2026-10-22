idf_component_register(SRCS "main.c" "software_sha256.c" INCLUDE_DIRS "." REQUIRES mbedtls esp_timer esp_hal_security esp_wifi esp_event nvs_flash esp_netif lwip)
