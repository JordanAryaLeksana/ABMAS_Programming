#ifndef DS3231_H_
#define DS3231_H_

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include "driver/i2c.h"

typedef struct 
{
    i2c_port_t i2c_port;   
    uint8_t i2c_address;  
} ds3231_t;

// Hanya deklarasi fungsi di sini
bool ds3231_init(ds3231_t *dev, i2c_port_t port, uint8_t address);
bool ds3231_set_epoch(ds3231_t *dev, time_t t);
bool ds3231_get_epoch(ds3231_t *dev, time_t *t);
bool ds3231_read_time(ds3231_t *dev, struct tm *timeinfo);
bool ds3231_write_time(ds3231_t *dev, const struct tm *timeinfo);

#endif // DS3231_H_
