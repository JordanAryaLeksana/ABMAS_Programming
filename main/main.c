#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rs485.h"
#include "esp_log.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    // Sensor 1: EC + pH + Temp + Humid
    soil_sensor_t ecph_sensor = {
        .uart_num = UART_NUM_1,
        .tx_pin = 17,
        .rx_pin = 16,
        .de_re_pin = 4};

    // Sensor 2: NPK
    soil_sensor_t npk_sensor = {
        .uart_num = UART_NUM_2,
        .tx_pin = 14,
        .rx_pin = 13,
        .de_re_pin = 32};

    // Struktur data untuk simpan hasil
    soil_parameters_t ecph_data = {0};
    soil_parameters_t npk_data = {0};
    init_soil_sensor(&ecph_sensor);
    init_soil_sensor(&npk_sensor);
    while (1)
    {
        // Baca ECPH dari sensor 1
        read_ecph_parameters(&ecph_sensor, &ecph_data);
        // Baca NPK dari sensor 2
        read_npk_parameters(&npk_sensor, &npk_data);

        ESP_LOGI(TAG, "=== Sensor EC+PH ===");
        ESP_LOGI(TAG, "EC: %.2f mS/cm", ecph_data.ec / 100.0);
        ESP_LOGI(TAG, "pH: %.2f", ecph_data.ph);
        ESP_LOGI(TAG, "TDS: %.1f ppm", ecph_data.tds);
        ESP_LOGI(TAG, "Salinity: %.1f ppt", ecph_data.salinity);

        ESP_LOGI(TAG, "=== Sensor NPK ===");
        ESP_LOGI(TAG, "Nitrogen (N): %.2f mg/kg", npk_data.nitrogen);
        ESP_LOGI(TAG, "Phosphorus (P): %.2f mg/kg", npk_data.phosphor);
        ESP_LOGI(TAG, "Potassium (K): %.2f mg/kg", npk_data.kalium);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
