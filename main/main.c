#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#define TAG "MAIN_LOOP"
#include "rs485.h"


void app_main()
{
    soil_parameters_t soil_params;
    ESP_LOGI(TAG, "Starting RS485 communication");
    soil_initialize();
    while (1)
    {
        read_all_soil_parameters(&soil_params);
        ESP_LOGI(TAG, "Soil Parameters: EC=%.2f, pH=%.2f, Temp=%.2f, Humidity=%.2f, Nitrogen=%.2f, Phosphor=%.2f, Kalium=%.2f",
                 soil_params.ec, soil_params.ph, soil_params.temperature,
                 soil_params.humidity, soil_params.nitrogen, soil_params.phosphor, soil_params.kalium);
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}
