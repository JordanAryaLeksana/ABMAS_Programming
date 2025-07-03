#ifndef MAIN_GY2561_H_
#define MAIN_GY2561_H_ 
#include "driver/i2c.h"

typedef struct 
{
    i2c_port_t i2c_port;   
    uint8_t i2c_address; 
} gy2561_t;


bool gy2561_init(gy2561_t *dev, i2c_port_t port, uint8_t address);

bool gy2561_read_lux(gy2561_t *dev, float *lux);

#endif

