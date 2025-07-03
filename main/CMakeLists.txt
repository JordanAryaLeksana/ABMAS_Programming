idf_component_register(
  SRCS "main.c"
  INCLUDE_DIRS "."
  REQUIRES spi_flash esp_driver_uart rs485 loraRFM95 ds3231 esp_app_format
)
