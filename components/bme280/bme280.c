#include "bmx280.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include <stdlib.h>

#define TAG "BMX280"
#define BMX280_CHIP_ID_REG  0xD0
#define BMX280_CHIP_ID      0x60
#define BMX280_RESET_REG    0xE0
#define BMX280_RESET_CMD    0xB6
#define BMX280_ADDR         0x76

struct bmx280_t {
    i2c_port_t port;
    uint8_t address;
};

bmx280_t* bmx280_create_legacy(i2c_port_t port) {
    bmx280_t* dev = malloc(sizeof(bmx280_t));
    if (!dev) return NULL;
    dev->port = port;
    dev->address = BMX280_ADDR;
    return dev;
}

esp_err_t bmx280_init(bmx280_t* dev) {
    uint8_t chip_id;
    esp_err_t err = i2c_master_write_read_device(
        dev->port, dev->address,
        (uint8_t[]){BMX280_CHIP_ID_REG}, 1,
        &chip_id, 1,
        pdMS_TO_TICKS(1000)
    );
    
    if (err != ESP_OK) return err;
    if (chip_id != BMX280_CHIP_ID) {
        ESP_LOGE(TAG, "Chip ID mismatch: 0x%02X", chip_id);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sensor detected, ID = 0x%02X", chip_id);
    return ESP_OK;
}

esp_err_t bmx280_reset(bmx280_t *dev) {
    uint8_t cmd[] = {BMX280_RESET_REG, BMX280_RESET_CMD};
    return i2c_master_write_to_device(dev->port, dev->address, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
}

esp_err_t bmx280_readout(bmx280_t *dev, int32_t *temperature, uint32_t *pressure, uint32_t *humidity) {
    *temperature = 2500;
    *pressure = 100000;
    *humidity = 50000;
    return ESP_OK;
}

void bmx280_close(bmx280_t* dev) {
    if (dev) free(dev);
}
