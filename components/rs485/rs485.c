#include "rs485.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RS485";

static uint16_t crc16(uint8_t *data, int length)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

static void set_tx_mode(const soil_sensor_t *sensor)
{
    gpio_set_level(sensor->de_re_pin, 1);
}

static void set_rx_mode(const soil_sensor_t *sensor)
{
    gpio_set_level(sensor->de_re_pin, 0);
}

void init_soil_sensor(const soil_sensor_t *sensor)
{
    // Konfigurasi GPIO DE/RE
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << sensor->de_re_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    set_rx_mode(sensor);

    // Konfigurasi UART
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(sensor->uart_num, &uart_config);
    uart_set_pin(sensor->uart_num, sensor->tx_pin, sensor->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(sensor->uart_num, 256, 0, 0, NULL, 0);

    ESP_LOGI(TAG, "Sensor initialized on UART%d - TX: %d, RX: %d, DE/RE: %d",
             sensor->uart_num, sensor->tx_pin, sensor->rx_pin, sensor->de_re_pin);
}

void read_ecph_parameters(const soil_sensor_t *sensor, soil_parameters_t *params)
{
    uart_flush(sensor->uart_num);

    uint8_t cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
    uint8_t resp[13] = {0};

    set_tx_mode(sensor);
    vTaskDelay(pdMS_TO_TICKS(10));
    uart_write_bytes(sensor->uart_num, (const char *)cmd, sizeof(cmd));
    uart_wait_tx_done(sensor->uart_num, pdMS_TO_TICKS(200));
    set_rx_mode(sensor);
    vTaskDelay(pdMS_TO_TICKS(100));

    int len = uart_read_bytes(sensor->uart_num, resp, sizeof(resp), pdMS_TO_TICKS(2000));

    if (len == 13 && resp[0] == 0x01 && resp[1] == 0x03)
    {
        params->salinity = ((resp[3] << 8) | resp[4]) / 10.0f;
        params->tds = ((resp[5] << 8) | resp[6]) / 10.0f;
        params->ec = ((resp[7] << 8) | resp[8]) / 100.0f;
        params->ph = ((resp[9] << 8) | resp[10]) / 10.0f;

     
        ESP_LOGI(TAG, "ECPH: EC=%.2f, pH=%.2f, Salinity=%.1f, TDS=%.1f",
                 params->ec, params->ph,
                params->salinity, params->tds);
    }
    else
    {
        ESP_LOGW(TAG, "ECPH read failed or invalid response");
        params->salinity = params->tds = params->ec = params->ph = 0;
    }
}

void read_npk_parameters(const soil_sensor_t *sensor, soil_parameters_t *params)
{
    struct
    {
        const char *label;
        uint16_t reg;
        float *target;
    } npk_map[] = {
        {"N", NITROGEN_REG, &params->nitrogen},
        {"P", PHOSPHORUS_REG, &params->phosphor},
        {"K", POTASSIUM_REG, &params->kalium},
    };

    for (int i = 0; i < 3; i++)
    {
        uart_flush(sensor->uart_num);

        uint8_t req[8];
        req[0] = 0x01;
        req[1] = 0x03;
        req[2] = (npk_map[i].reg >> 8) & 0xFF;
        req[3] = npk_map[i].reg & 0xFF;
        req[4] = 0x00;
        req[5] = 0x01;
        uint16_t crc = crc16(req, 6);
        req[6] = crc & 0xFF;
        req[7] = (crc >> 8) & 0xFF;

        set_tx_mode(sensor);
        vTaskDelay(pdMS_TO_TICKS(10));
        uart_write_bytes(sensor->uart_num, (const char *)req, 8);
        uart_wait_tx_done(sensor->uart_num, pdMS_TO_TICKS(200));
        set_rx_mode(sensor);
        vTaskDelay(pdMS_TO_TICKS(100));

        uint8_t resp[7] = {0};
        int len = uart_read_bytes(sensor->uart_num, resp, sizeof(resp), pdMS_TO_TICKS(2000));

        if (len == 7 && resp[0] == 0x01 && resp[1] == 0x03 && resp[2] == 0x02)
        {
            *(npk_map[i].target) = (float)((resp[3] << 8) | resp[4]);
            ESP_LOGI(TAG, "%s value: %.2f", npk_map[i].label, *(npk_map[i].target));
        }
        else
        {
            *(npk_map[i].target) = 0;
            ESP_LOGW(TAG, "Failed to read %s", npk_map[i].label);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
