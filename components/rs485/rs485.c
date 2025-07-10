#include "rs485.h"
#include <stdint.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#ifndef CONFIG_LOG_MAXIMUM_LEVEL
#define CONFIG_LOG_MAXIMUM_LEVEL 3
#endif

#define CONFIG_SENSOR_TX_PIN 17
#define CONFIG_SENSOR_RX_PIN 16
#define DE_RE_PIN 4
#define CONFIG_FREE_RTOS_HZ 1000
uint16_t crc16(uint8_t *data, int length)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void soil_initialize()
{
    ESP_LOGI("rs485", "Initializing soil sensor");

    // Set up UART for communication
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, CONFIG_SENSOR_TX_PIN, CONFIG_SENSOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 256, 0, 0, NULL, 0));
}

void read_all_soil_parameters(soil_parameters_t *params) {
    // ---------- pH + EC + Temp + Humid ----------
    uint8_t ecph_cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
    uint8_t ecph_resp[13] = {0};

    gpio_set_level(DE_RE_PIN, 1); // TX
    uart_write_bytes(UART_NUM_2, (const char *)ecph_cmd, sizeof(ecph_cmd));
    uart_wait_tx_done(UART_NUM_2, pdMS_TO_TICKS(100));
    gpio_set_level(DE_RE_PIN, 0); // RX
    vTaskDelay(pdMS_TO_TICKS(50));

    int len = uart_read_bytes(UART_NUM_2, ecph_resp, sizeof(ecph_resp), pdMS_TO_TICKS(1000));
    if (len == 13 && ecph_resp[0] == 0x01 && ecph_resp[1] == 0x03) {
        params->humidity     = (ecph_resp[3] << 8) | ecph_resp[4];
        params->temperature  = (ecph_resp[5] << 8) | ecph_resp[6];
        params->ec           = (ecph_resp[7] << 8) | ecph_resp[8];
        params->ph           = ((ecph_resp[9] << 8) | ecph_resp[10]) / 10.0;
    } else {
        ESP_LOGW("SOIL", "Failed to read EC+PH response");
    }

    // ---------- NPK Individu ----------
    struct {
        const char *label;
        uint16_t reg;
        int *target;
    } npk_regs[] = {
        {"N", 0x001E, &params->nitrogen},
        {"P", 0x001F, &params->phosphor},
        {"K", 0x0020, &params->kalium},
    };

    for (int i = 0; i < 3; ++i) {
        uint8_t req[8];
        req[0] = 0x01;
        req[1] = 0x03;
        req[2] = (npk_regs[i].reg >> 8) & 0xFF;
        req[3] = npk_regs[i].reg & 0xFF;
        req[4] = 0x00;
        req[5] = 0x01;
        uint16_t crc = crc16(req, 6);
        req[6] = crc & 0xFF;
        req[7] = (crc >> 8) & 0xFF;

        gpio_set_level(DE_RE_PIN, 1);
        uart_write_bytes(UART_NUM_2, (const char *)req, 8);
        uart_wait_tx_done(UART_NUM_2, pdMS_TO_TICKS(100));
        gpio_set_level(DE_RE_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(50));

        uint8_t resp[7] = {0};
        int len = uart_read_bytes(UART_NUM_2, resp, sizeof(resp), pdMS_TO_TICKS(1000));
        if (len == 7 && resp[0] == 0x01 && resp[1] == 0x03 && resp[2] == 0x02) {
            *(npk_regs[i].target) = (resp[3] << 8) | resp[4];
        } else {
            ESP_LOGW("SOIL", "Failed to read %s value", npk_regs[i].label);
        }
    }
}

