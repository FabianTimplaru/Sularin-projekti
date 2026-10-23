#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Määritellään pinnit
#define POWER_PIN 15
#define ADC_PIN 26
#define ADC_CHANNEL 0 // GP26 on ADC-kanava 0

int main() {
    // Alustetaan standarditulostus (mahdollistaa printf-käytön USB:n kautta)
    stdio_init_all();

    // Alustetaan virransyöttöpinni (GP15)
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
    gpio_put(POWER_PIN, 0); // Varmistetaan, että virta on aluksi pois päältä

    // Alustetaan ADC (Analog-to-Digital Converter)
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);

    // Annetaan USB-yhteydelle aikaa aueta ennen tulostelua
    sleep_ms(2000);
    printf("DIY Mullan Kosteusmittari (Pico 2W) käynnistetty!\n");

    while (1) {
        // 1. Kytketään virta anturiin
        gpio_put(POWER_PIN, 1);
        
        // 2. Odotetaan hetki, jotta jännite tasaantuu
        sleep_ms(10); 
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        // 3. Luetaan analoginen arvo (0 - 4095)
        uint16_t raw_value = adc_read();
        
        // 4. Sammutetaan virta heti elektrolyysin estämiseksi
        gpio_put(POWER_PIN, 0);

        // Tulostetaan tulos
        // Picon ADC on 12-bittinen, eli arvot ovat välillä 0 (0V) - 4095 (3.3V)
        printf("Mullan kosteus (raaka-arvo): %d, Voltit %f V\n", raw_value, raw_value * conversion_factor);

        // Odotetaan 2 sekuntia ennen seuraavaa mittausta
        sleep_ms(2000);
    }
}