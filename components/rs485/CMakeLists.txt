idf_component_register(SRCS "rs485.c"
                        PRIV_REQUIRES spi_flash esp_driver_uart driver
                        INCLUDE_DIRS "include")