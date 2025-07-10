#ifndef __SOIL7IN1_H__
#define __SOIL7IN1_H__

#include <stdint.h>

typedef struct {
  uint8_t address;
  uint8_t func;
  uint16_t start;
  uint16_t length;
  uint16_t CRC;
} inquirity_frame_t;

typedef struct {
  uint8_t address;
  uint8_t func;
  uint8_t effective_number;  
} response_frame_meadata_t;

#define PH_VALUE_REG 0x6
#define SOIL_MOIST_VALUE_REG 0x12
#define SOIL_TEMP_REG 0x13
#define SOIL_COND_REG 0x15
#define SOIL_NITROGEN_REG 0x1E
#define SOIL_PHOSPORUS_REG 0x1F
#define SOIL_POTASIUM_REG 0x20
#define EQUIP_ADDR_REG 0x100
#define BAU_RATE_REG 0x101

typedef struct {
    float ec;
    float ph;
    float temperature;
    float humidity;
    float nitrogen;
    float phosphor;
    float kalium;
} soil_parameters_t;



void soil_initialize();
void read_all_soil_parameters(soil_parameters_t *soil_params);

#endif