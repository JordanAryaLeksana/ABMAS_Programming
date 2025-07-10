idf_component_register(
  SRCS "main.c" 
  INCLUDE_DIRS "."
  REQUIRES spi_flash esp_driver_uart rs485 esp_app_format bme280 gy2561 ds3231 lora esp-modbus
)
