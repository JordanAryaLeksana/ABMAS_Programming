
#include "ds3231.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define TAG "DS3231"

#define DS3231_ADDR 0x68
#define DS3231_REG_TIME 0x00
#define DS3231_REG_TEMP_MSB 0x11
#define DS3231_REG_TEMP_LSB 0x12

static uint8_t bcd2bin(uint8_t val)
{
    return ((val >> 4) * 10 + (val & 0x0F));
}

static uint8_t bin2bcd(uint8_t val)
{
    return ((val / 10) << 4 | (val % 10));
}

bool ds3231_init(ds3231_t *dev, i2c_port_t port, uint8_t address)
{
    dev->i2c_port = port;
    dev->i2c_address = address;
    ESP_LOGI(TAG, "DS3231 initialized at address 0x%02X", address);
    return true;
}

bool ds3231_get_time(ds3231_t *dev, struct tm *timeinfo)
{
    uint8_t data[7];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, DS3231_REG_TIME, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 7, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
        return false;

    timeinfo->tm_sec = bcd2bin(data[0]);
    timeinfo->tm_min = bcd2bin(data[1]);
    timeinfo->tm_hour = bcd2bin(data[2]);
    timeinfo->tm_mday = bcd2bin(data[4]);
    timeinfo->tm_mon = bcd2bin(data[5]) - 1;
    timeinfo->tm_year = bcd2bin(data[6]) + 100;

    return true;
}