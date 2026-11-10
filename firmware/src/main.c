#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"     // Tarvitaan IP-osoitteen lukemiseen
#include "wifi_config.h"    // Omat salasanat
#include "dht11.h"

// Määritellään pinnit
#define POWER_PIN 15
#define ADC_PIN 26
#define ADC_CHANNEL 0 // GP26 on ADC-kanava 0
#define LED_PIN 13
#define DHT11_PIN 14

float calculate_moisture(uint16_t adc);

int main()
{
    // Alustetaan standarditulostus (mahdollistaa printf-käytön USB:n kautta)
    stdio_init_all();

//DHT11
    dht11_data_t dht;

    // Alustetaan virransyöttöpinni (GP15)
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
    gpio_put(POWER_PIN, 0); // Varmistetaan, että virta on aluksi pois päältä

    // Alustetaan ADC (Analog-to-Digital Converter)
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);

    // Alustetaan LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // Annetaan USB-yhteydelle aikaa aueta ennen tulostelua
    sleep_ms(2000);

    printf("DIY Mullan Kosteusmittari (Pico 2W) käynnistetty!\n");

    //LED raja-arvo
    bool led_state = false;

// Oletusarvo, jos backendistä ei ole vielä saatu asetuksia
    float moisture_threshold = 30.0f;
    printf("Kaynnistetaan Wi-Fi testi...\n");

    // Alustetaan Pico W:n CYW43-verkkopiiri
    if (cyw43_arch_init()) {
        printf("Virhe: Wi-Fi-piirin alustus epaonnistui!\n");
        return -1;
    }

    // Asetetaan piiri Station-tilaan (yhdistää reitittimeen, ei luo omaa verkkoa)
    cyw43_arch_enable_sta_mode();

    printf("Yhdistetaan verkkoon: %s\n", WIFI_SSID);

    // Yritetään yhdistää verkkoon (aikakatkaisu 10 sekuntia eli 10000 ms)
    // CYW43_AUTH_WPA2_MIXED_PSK toimii lähes kaikissa nykyaikaisissa kotiverkoissa
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK, 10000)) {
        printf("Virhe: Verkkoon yhdistaminen epaonnistui.\n");
    }

    printf("Yhdistetty onnistuneesti!\n");

    // Luetaan ja tulostetaan laitteen saama IP-osoite
    // netif_default on lwIP-kirjaston oletusverkkoliitäntä (eli Picon Wi-Fi)
    printf("IP-osoite: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));


    /* Yritetään hakea backendistä
if (backend_connected)
{
    moisture_threshold = backend_threshold;
}
*/

// Hystereesi
    const float hysteresis = 5.0f;

    while (1)
    {




        // luetaan DHT11
        if (dht11_read(DHT11_PIN, &dht))
    {
        printf("Lampotila: %d C\n", dht.temperature);
        printf("Ilmankosteus: %d %%\n", dht.humidity);
    }
    else
    {
        printf("DHT11 luku epaonnistui\n");
    }



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

        // Lasketaan arvot prosentteina
        float moisture = calculate_moisture(raw_value);

        // LED päälle alle raja-arvon
        if (!led_state && moisture < moisture_threshold)
        {
            gpio_put(LED_PIN, 1);
            led_state = true;
        }

        // LED pois kun raja-arvo + hystereesi ylittyy
        if (led_state && moisture > (moisture_threshold + hysteresis))
        {
            gpio_put(LED_PIN, 0);
            led_state = false;
        }

        // Tulostetaan tulos
        // Picon ADC on 12-bittinen, eli arvot ovat välillä 0 (0V) - 4095 (3.3V)
        printf("Mullan kosteus (raaka-arvo): %d, Voltit %.2f V, Suhteellinen kosteus %.2f %%\n",
               raw_value, raw_value * conversion_factor, moisture);
        // Odotetaan 2 sekuntia ennen seuraavaa mittausta
        sleep_ms(2000);
    }
}

// Tämän funktion avulla saadaan arvot suhteelisena prosenttiarvona

float regression(float x)
{
    return 0.0000909081f * x * x - 0.244061f * x + 177.237f;
}

float calculate_moisture(uint16_t adc)
{
    // 1342 ADC on mitä realistisesti tarkottaa kuiva multa, ei multa jossa olisi 0g vettä
    const float dry_adc = 1342.0f;
    const float wet_adc = 3200.0f;

    if (adc <= dry_adc)
        return 0.0f;

    if (adc >= wet_adc)
        return 100.0f;

    float moisture = (regression(adc) - regression(dry_adc)) / (regression(wet_adc) - regression(dry_adc)) * 100.0f;

    return moisture;
}
