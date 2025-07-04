#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include "bmx280.h"
#include "ds3231.h"
#include "gy2561.h"

#include "lora.h"

#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

static const char *TAG = "MAIN";

// Instances
bmx280_t *bmx;
ds3231_t rtc;
gy2561_t lux_sensor;

void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main() {
    // Inisialisasi I2C
    i2c_master_init();
    ESP_LOGI(TAG, "I2C Initialized");

    // Inisialisasi BME280 / BMP280
    bmx = bmx280_create_legacy(I2C_MASTER_NUM);
    if (bmx280_init(bmx) == ESP_OK) {
        ESP_LOGI(TAG, "BMX280 connected");
    } else {
        ESP_LOGE(TAG, "Failed to init BMX280");
    }

    // Inisialisasi DS3231
    ds3231_init(&rtc, I2C_MASTER_NUM, 0x68);

    // Inisialisasi BH1750 (GY2561)
    gy2561_init(&lux_sensor, I2C_MASTER_NUM, 0x23);

    // Inisialisasi LoRa
    if (lora_init()) {
        lora_set_frequency(915e6); // atau 868e6 tergantung modul
        lora_enable_crc();
        ESP_LOGI(TAG, "LoRa Initialized");
    } else {
        ESP_LOGE(TAG, "LoRa Init Failed");
    }

    while (1) {
        // Baca suhu, tekanan, kelembapan
        float temp, pres, hum;
        if (bmx280_readoutFloat(bmx, &temp, &pres, &hum) == ESP_OK) {
            ESP_LOGI(TAG, "Temp: %.2f C, Pressure: %.2f Pa, Hum: %.2f %%", temp, pres, hum);
        }

        // Baca lux dari BH1750
        float lux;
        if (gy2561_readlux(&lux_sensor, &lux)) {
            ESP_LOGI(TAG, "Lux: %.2f lx", lux);
        }

        // Baca waktu dari DS3231
        struct tm timeinfo;
        if (ds3231_get_time(&rtc, &timeinfo)) {
            ESP_LOGI(TAG, "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }

        // Kirim data via LoRa
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "T:%.2f P:%.2f H:%.2f Lux:%.2f Time:%02d:%02d:%02d",
                 temp, pres, hum, lux,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        lora_send_packet((uint8_t *)msg, strlen(msg));
        ESP_LOGI(TAG, "LoRa Sent: %s", msg);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
