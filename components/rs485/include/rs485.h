#ifndef __SOIL7IN1_H__
#define __SOIL7IN1_H__

#include <stdint.h>
#include "driver/uart.h"

typedef struct
{
    uart_port_t uart_num;
    int tx_pin;
    int rx_pin;
    int de_re_pin;
} soil_sensor_t;

typedef struct
{
    float ec;
    float ph;
    float salinity;
    float tds;
    float nitrogen;
    float phosphor;
    float kalium;
} soil_parameters_t;

#define EC_REG             0x0002  // 40003
#define PH_REG             0x0003  // 40004
#define SALINITY_REG       0x0007  // 40005           
#define TDS_REG            0x0008  // 40006
#define NITROGEN_REG       0x001E  // 40031 
#define PHOSPHORUS_REG     0x001F // 40032
#define POTASSIUM_REG      0x0020  // 40033

// Device config
#define EQUIP_ADDR_REG     0x07D0
#define BAU_RATE_REG       0x07D1

// API
void init_soil_sensor(const soil_sensor_t *sensor);
void read_ecph_parameters(const soil_sensor_t *sensor, soil_parameters_t *soil_params);
void read_npk_parameters(const soil_sensor_t *sensor, soil_parameters_t *soil_params);

#endif
