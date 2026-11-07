#include "dht11.h"

#include "pico/stdlib.h"

#include "hardware/gpio.h"

#include <stdint.h>

bool dht11_read(uint gpio, dht11_data_t *data)
{
    // Lähetetään DHT11:lle aloitussignaali

    gpio_init(gpio);
    gpio_pull_up(gpio);

    gpio_set_dir(gpio, GPIO_OUT);

    gpio_put(gpio, 0);
    sleep_ms(20);

    gpio_put(gpio, 1);
    sleep_us(30);

    gpio_set_dir(gpio, GPIO_IN);

    // Odotetaan DHT11:n vastausta

    uint32_t timeout = 100;

    while (gpio_get(gpio) == 1 && timeout--)
    {
        sleep_us(1);
    }

    if (timeout == 0)
    {
        return false;
    }

    // DHT11:n ensimmäinen LOW-jakso

    timeout = 100;

    while (gpio_get(gpio) == 0 && timeout--)
    {
        sleep_us(1);
    }

    if (timeout == 0)
    {
        return false;
    }

    // DHT11:n HIGH-jakso

    timeout = 100;

    while (gpio_get(gpio) == 1 && timeout--)
    {
        sleep_us(1);
    }

    if (timeout == 0)
    {
        return false;
    }
   uint8_t bits[5] = {0};

    for (int i = 0; i < 40; i++)
    {
        // Odota bitin alkua (LOW)

        timeout = 100;

        while (gpio_get(gpio) == 0 && timeout--)
        {
            sleep_us(1);
        }

        if (timeout == 0)
        {
            return false;
        }

        // Mittaa HIGH-pulssin pituus

        uint32_t pulse_length = 0;

        while (gpio_get(gpio) == 1)
        {
            pulse_length++;
            sleep_us(1);

            if (pulse_length > 100)
            {
                return false;
            }
        }

        // Pitkä HIGH = 1
        // Lyhyt HIGH = 0

        if (pulse_length > 40)
        {
            bits[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

        data->humidity = bits[0];
    data->temperature = bits[2];

    return true;
}