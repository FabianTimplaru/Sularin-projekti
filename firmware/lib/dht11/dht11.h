#ifndef DHT11_H
#define DHT11_H

#include <stdbool.h>
#include "pico/stdlib.h"

typedef struct
{
    int temperature;
    int humidity;
} dht11_data_t;

bool dht11_read(uint gpio, dht11_data_t *data);

#endif