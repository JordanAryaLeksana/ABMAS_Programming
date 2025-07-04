#include "gy2561.h"
#include "driver/i2c.h"



bool gy2561_init(gy2561_t *dev, i2c_port_t port, uint8_t address){
    dev->i2c_port = port;
    dev->i2c_address = address;
    return true;
}

bool gy2561_readlux(gy2561_t *dev, float *lux){
    uint8_t cmd = 0x20;
    esp_err_t err = i2c_master_write_to_device(dev->i2c_port, dev->i2c_address, &cmd, 1, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) return false;

    vTaskDelay(pdMS_TO_TICKS(180)); // delay pengukuran (max 180ms)

    uint8_t data[2];
    err = i2c_master_read_from_device(dev->i2c_port, dev->i2c_address, data, 2, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) return false;

    uint16_t raw = (data[0] << 8) | data[1];
    *lux = raw / 1.2f;
    return true;
}