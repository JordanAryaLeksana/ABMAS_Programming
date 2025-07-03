#include "gy2561.h"
#include "driver/i2c.h"

bool gy2561_init(gy2561_t *dev, i2c_port_t port, uint8_t address){
    dev->i2c_port = port;
    dev->i2c_address = address;
    return true;
}

bool gy2561_readlux(gy2561_t *dev, float *lux){
    //configure lux via i2c
    
    return true;
}