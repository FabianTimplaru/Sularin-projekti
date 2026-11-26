#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h" // Tarvitaan IP-osoitteen lukemiseen
#include "lwip/tcp.h"
#include "wifi_config.h" // Omat salasanat
#include "dht11.h"
#include <string.h>
#include <stdlib.h>
#include "pico_led.h"
#include "bootsel_button.h"

// Määritellään pinnit
#define POWER_PIN 15
#define ADC_PIN 26
#define ADC_CHANNEL 0 // GP26 on ADC-kanava 0
#define LED_PIN 13
#define DHT11_PIN 14

// ADC-oletusarvot:
#define DEFAULT_DRY_ADC 1342.0f
#define DEFAULT_WET_ADC 3200.0f

static volatile uint32_t network_request_start_time = 0;
static volatile bool is_sending = false;
static volatile bool waiting_for_settings = false;
static volatile bool trigger_fetch = false;
float moisture_threshold = 30.0f;
void fetch_settings(void);
// 1342 ADC on mitä realistisesti tarkottaa kuiva multa, ei multa jossa olisi 0g vettä
// Kuiva ja märkä ADC (0% ja 100%) 
float dry_adc = DEFAULT_DRY_ADC;
float wet_adc = DEFAULT_WET_ADC;
bool is_custom_calibrated = false;

typedef enum {
    STATE_NORMAL, // Normaalitoiminta
    STATE_CALIBRATE_INIT, // siirtymä kalibrointitilaan. 5 sekuntia painiketta pohjassa normaalitilassa siirtää tänne. 10 sekuntia pohjassa resetoi ja siirrytään takaisin normaalitilaan.
    STATE_CALIBRATE_DRY, // Odotetaan kuivan mullan kalibrointia
    STATE_CALIBRATE_WET // Odotetaan märän mullan
} system_state_t;

system_state_t current_state = STATE_NORMAL;

float calculate_moisture(uint16_t adc);
// 1. Tämä funktio laukeaa automaattisesti, kun backend vastaa (Kuitti!)
static err_t http_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL)
    { // Yhteys suljettiin palvelimen toimesta
        tcp_close(tpcb);
        is_sending = false;
        return ERR_OK;
    }

    char buffer[512];
    uint16_t copy_len = p->len < (sizeof(buffer) - 1) ? p->len : (sizeof(buffer)-1);
    memcpy(buffer, p->payload, copy_len);
    buffer[copy_len] = '\0';

    printf("Vastaus backendilta:\n%s\n", buffer);
    const char *key = "\"minMoistureThreshold\":";
    char *threshold_pos = strstr(buffer, key);

if (threshold_pos != NULL)
{
    threshold_pos += strlen(key);
    moisture_threshold = atof(threshold_pos);
    printf("Backendin raja-arvo paivittyi: %.1f %%\n\n",moisture_threshold);
} 

if (waiting_for_settings)
{
    waiting_for_settings = false;
}
else
{
    // Käsketään pääsilmukkaa hakemaan asetukset myöhemmin!
    trigger_fetch = true; 
}


    led_play_sequence(SEQ_BACKEND_OK);
    tcp_recved(tpcb, p->tot_len); // Kerrotaan verkkopiirille, että data otettiin vastaan
    pbuf_free(p);                 // Vapautetaan muisti
    return ERR_OK;
}

static void http_err_cb(void *arg, err_t err)
{
    printf("\n[VERKKOVIRHE] LwIP-virhe (koodi %d)! Yhteys katkesi.\n", err);
    is_sending = false;
    waiting_for_settings = false;
    led_play_sequence(SEQ_BACKEND_ERR); // "0001"
}

// Tämä funktio laukeaa automaattisesti, kun yhteys backendiin on auki
static err_t http_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err != ERR_OK)
    {
        printf("Yhteys backendiin epaonnistui! kättelyssä!\n");
        is_sending = false;
        waiting_for_settings = false;
        return err;
    }
    char *request = (char *)arg; // Haetaan lähetettävä JSON-teksti

    // Kerrotaan Picolle, mihin funktioon mennään, kun vastaus saapuu
    tcp_recv(tpcb, http_recv_cb);

    // Lähetetään HTTP POST -pyyntö matkaan
    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    printf("Data lahetetty!\n");

    return ERR_OK;
}

//GET/api/settings 
void fetch_settings()
{
    waiting_for_settings = true;
    is_sending = true;

    network_request_start_time = to_ms_since_boot(get_absolute_time());

    static char http_request[200];

    snprintf(http_request,
             sizeof(http_request),
             "GET /api/settings HTTP/1.1\r\n"
             "Host: %s:3000\r\n"
             "Connection: close\r\n"
             "\r\n",
             SERVER_ADDRESS);
    cyw43_arch_lwip_begin();

    struct tcp_pcb *pcb = tcp_new();

    if (pcb != NULL)
    {
        ip_addr_t server_ip;
        ipaddr_aton(SERVER_ADDRESS, &server_ip);

        tcp_arg(pcb, http_request);
        tcp_err(pcb, http_err_cb);

        tcp_connect(pcb,
                    &server_ip,
                    3000,
                    http_connected_cb);

        printf("Haetaan asetukset backendilta... ");
    } 
    else // LISÄÄ TÄMÄ
    {
        printf("Virhe: Ei voitu luoda tcp_pcb-oliota asetusten hakuun.\n");
        is_sending = false;
        waiting_for_settings = false;
    }
    cyw43_arch_lwip_end();
}

// Mullankosteusanturin funktio
uint16_t read_soil_adc(void) {
    gpio_put(POWER_PIN, 1);
    sleep_ms(10);
    uint16_t raw_value = adc_read();
    gpio_put(POWER_PIN, 0);
    return raw_value;
}


int main()
{
    // Alustetaan standarditulostus (mahdollistaa printf-käytön USB:n kautta)
    stdio_init_all();
    
    // Alustetaan Pico W:n CYW43-verkkopiiri
    if (cyw43_arch_init())
    {
        printf("Virhe: Wi-Fi-piirin alustus epaonnistui!\n");
        return -1;
    }

    // Määritetään ajastukset toiminnoille:
    
    const uint32_t sensor_wait_time = 2000; // 2 sekuntia
    const uint32_t sensor_sample_count = 10; // Kuinka monta tulosta kerätään,ennen kuin lähetetään tiedot backendille
    bool first_send = true; // Ensimmäinen lähetys heti käynnistyessä, jotta voidana määrittää laite yhdistetyksi

    bool button_was_pressed = false;
    uint32_t button_press_start_time = 0;
    bool five_sec_triggered = false;
    bool ten_sec_triggered = false;
   
    uint32_t last_sensor_read_time = to_ms_since_boot(get_absolute_time());  // Määritetään viime mittaus tähän hetkeen, jotta varmasti saadaan DHT alustettua ennen mittauksia.
    uint32_t last_cal_blink_time = 0; // Määritetään vilkkumisaika kalibroinnissa.
    uint32_t cal_state_enter_time = 0; // Määritetään kalibroinnin tilojen aloituksen

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
    led_play_sequence(SEQ_STARTUP);
    uint32_t wait_start = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time())- wait_start < 2000) {
        led_sequencer_update(to_ms_since_boot(get_absolute_time()));
        sleep_ms(1);
    }

    printf("DIY Mullan Kosteusmittari (Pico 2W) käynnistetty!\n");

    // LED raja-arvo
    bool led_state = false;

    // Oletusarvo, jos backendistä ei ole vielä saatu asetuksia
    //float moisture_threshold = 30.0f;
    printf("Alustetaan Wi-Fi-piiri..\n");

    

    // Asetetaan piiri Station-tilaan (yhdistää reitittimeen, ei luo omaa verkkoa)
    cyw43_arch_enable_sta_mode();

    printf("Yhdistetaan verkkoon: %s\n", WIFI_SSID);

    // Yritetään yhdistää verkkoon (aikakatkaisu 10 sekuntia eli 10000 ms)
    // CYW43_AUTH_WPA2_MIXED_PSK toimii lähes kaikissa nykyaikaisissa kotiverkoissa
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK, 10000))
    {
        printf("Virhe: Verkkoon yhdistaminen epaonnistui.\n");
    }
    else {

    printf("Yhdistetty onnistuneesti!\n");
    led_play_sequence(SEQ_ONLINE);
    // Luetaan ja tulostetaan laitteen saama IP-osoite
    // netif_default on lwIP-kirjaston oletusverkkoliitäntä (eli Picon Wi-Fi)
    printf("IP-osoite: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
}

//GET /api/ settings (raja arvo )

trigger_fetch = true;



    // Hystereesi
    const float hysteresis = 5.0f;

    float temp_sum = 0.0f;
    float humidity_sum = 0.0f;
    float moisture_sum = 0.0f;
    float adc_sum = 0.0f;

    int sample_count = 0;


while (1) {

    // Määritetään tämän hetkinen aika
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    //  Luetaan BOOTSEL-painikkeen tila
    bool is_pressed = read_bootsel_button();
    led_set_continuous_long(is_pressed && !five_sec_triggered);
    // Päivitetään led-sekvensserin tila
    led_sequencer_update(now);

    // Tunnista lyhyt painallus, 5 s painallus j 10 s painallus
    bool short_press = false;
    bool long_press = false;
    bool reset_press = false;

    if (is_pressed && !button_was_pressed) {
        button_was_pressed = true;
        button_press_start_time = now;
        five_sec_triggered = false;
        ten_sec_triggered = false;
    }

    else if (is_pressed && button_was_pressed) {
        uint32_t held_time = now - button_press_start_time;

        // 5 sekuntia pidetty pohjassa --> Kalibrointi
        if (!five_sec_triggered && held_time >= 5000) {
            five_sec_triggered = true;
            long_press = true;
        }
        // 10 sekuntia pidetty pohjassa --> arvojen resetointi
        if (!ten_sec_triggered && held_time >= 10000) {
            ten_sec_triggered = true;
            reset_press = true;
        }    
    }
    else if (!is_pressed && button_was_pressed) {
        // Lyhy painallus vain, jos kumpikaan pitkä raja ei lauennut
        if (!five_sec_triggered && !ten_sec_triggered) {
            short_press = true;
        }
        button_was_pressed = false;
    }

    

    // TILAKONE
    switch (current_state) {

        case STATE_NORMAL:
        // Siirrytään kalibrointiin 5 s painalluksella
        if (long_press) {
            printf("\nSiirrytaan kalibrointitilaan.\n");
            printf("10 sekuntia painiketta pohjassa palautetaan tehdasasetukset.\n");
            current_state = STATE_CALIBRATE_INIT;
            break;
        }
    
    // Pääsilmukka "NORMAL_STATE"

    if (trigger_fetch && !is_sending) {
            trigger_fetch = false;
            fetch_settings();
        }


    // Onko verkko lukittuna yli 15 sekuntia --> Pakotetaan se auki
    if (is_sending && (now - network_request_start_time >= 15000)) {
        printf("\n[TIMEOUT] Backend ei vastannut 15 sekunnissa! Pakotetaan verkko auki uusia yrityksia varten.\n\n");
        network_request_start_time = now;
        is_sending = false;
        waiting_for_settings = false;
        led_play_sequence(SEQ_BACKEND_ERR);
    }

    // Rinnakkaisuusajo mittaukselle:
    if (now - last_sensor_read_time >= sensor_wait_time) {
        last_sensor_read_time = now;
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

        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        // 3. Luetaan mullankosteusanturi-funktio
        uint16_t raw_value = read_soil_adc();

        // Lasketaan arvot prosentteina
        float moisture = calculate_moisture(raw_value);

        temp_sum += dht.temperature;
        humidity_sum += dht.humidity;
        moisture_sum += moisture;
        adc_sum += raw_value;

        sample_count++;

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

        // lasketaan 10 min keskiarvot
        if (sample_count >= sensor_sample_count || first_send && sample_count >= 1)
        {
            first_send = false;
            int avg_temp = (int)(temp_sum / sample_count + 0.5f);
            int avg_humidity = (int)(humidity_sum / sample_count + 0.5f);
            int avg_adc = (int)(adc_sum / sample_count + 0.5f);
            int avg_moisture = (int)(moisture_sum / sample_count + 0.5f);
          
        //JSON
        printf("\nLahetetaan tulosten keskiarvot... ");
        led_play_sequence(SEQ_WIFI_SEND);
        char json_body[200];
        snprintf(json_body, sizeof(json_body),
        "{\"token\":\"%s\", \"temp\":%d, \"airHumidity\":%d, \"soilMoisture_adc\":%d, \"soilMoisture_percent\":%d}",  
        API_KEY,
        avg_temp,
        avg_humidity,
        avg_adc,
        avg_moisture
        ); 
        if (!is_sending) { // Estä uusi lähetys jos edellinen on kesken
            is_sending = true;
            network_request_start_time = now;
        // HTTP-POST -pyyntö
        static char http_request[400];
        snprintf(http_request, sizeof(http_request),
        "POST /api/measurements HTTP/1.1\r\n"
        "Host: %s:3000\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"  
        "%s",   
        SERVER_ADDRESS, strlen(json_body), json_body);
        // Avataan TCP-yhteys ja lähetetään data Raw API:lla
        cyw43_arch_lwip_begin();
        struct tcp_pcb *pcb = tcp_new();
        if (pcb != NULL) {
            ip_addr_t server_ip;
            ipaddr_aton(SERVER_ADDRESS, &server_ip);
            // Välitetään http_request-merkkijono takaisinkutsufunktioille (arg)
            tcp_arg(pcb, http_request);
            tcp_err(pcb, http_err_cb); 
            // Yhdistetään IP-osoitteeseen porttiin 3000
            tcp_connect(pcb, &server_ip, 3000, http_connected_cb);
        } else {
            printf("Virhe: Ei voitu luoda tcp_pcb-oliota.\n");
            is_sending = false;
        }
        cyw43_arch_lwip_end();
    } else {
            printf("Verkko varattu, ohitetaan lähetys...\n");
        }
    
    temp_sum = 0.0f;
    humidity_sum = 0.0f;
    moisture_sum = 0.0f;
    adc_sum = 0.0f;

    sample_count = 0;
}
}
break;

case STATE_CALIBRATE_INIT:
    if(!is_pressed){
        printf("Aseta anturi KUIVAAN ja paina nappia\n");
        printf("Siirry tallentamatta kuivaa arvoa painalla 5 sekuntia painiketta\n");
            
        led_play_sequence(SEQ_DRY);
        last_cal_blink_time = now; // led-vilkku Ajastin käyntiin
        cal_state_enter_time = now; // Timeout-ajastin käyntiin
        current_state = STATE_CALIBRATE_DRY;
        break;
    }
    else if (now - last_cal_blink_time >= 500 && !reset_press) {
                // Nopea syke 5-10 s välillä: "pidä vielä resettiin tai päästä irti"
                led_play_sequence(".");
                last_cal_blink_time = now;
            }
    if (reset_press) {
        printf("Tehdasasetukset palautettu!\n");

        // Palautetaan oletuskalibrointiarvot
        is_custom_calibrated = false;
        dry_adc = DEFAULT_DRY_ADC;
        wet_adc = DEFAULT_WET_ADC;
        printf("Kuiva: %.0f Märkä: %.0f \n\n", dry_adc, wet_adc);
        led_play_sequence(SEQ_STARTUP);
        // Nollataan mittaussummat ja palataan normaalitilaan
        sample_count =0;
        temp_sum = humidity_sum = moisture_sum = adc_sum = 0.0f;
        last_sensor_read_time =now;
        current_state = STATE_NORMAL;
        network_request_start_time = now;
    }
    break;

case STATE_CALIBRATE_DRY:
    if (!is_pressed && (now - last_cal_blink_time >= 4000)) {
                led_play_sequence(SEQ_DRY);
                last_cal_blink_time = now;
            }
    if (!is_pressed && (now - cal_state_enter_time >= 10000)) {
                printf("Kalibrointi aikakatkaistiin. Palataan normaaliin tilaan.\n");
                led_play_sequence(SEQ_STARTUP);
                sample_count = 0;
                temp_sum = humidity_sum = moisture_sum = adc_sum = 0.0f;
                last_sensor_read_time = now;
                current_state = STATE_NORMAL;
                network_request_start_time = now;
                break;
            }
    if (long_press) {
        printf("Arvoa ei tallennettu. kuiva-arvo on %.0f.\nAseta anturi MARKAAN ja paina nappia.\n", dry_adc);
        printf("Lopeta kalibrointi tallentamatta markaa arvoa painalla 5 sekuntia painiketta.\n");
        led_play_sequence(SEQ_LOAD);
        cal_state_enter_time = now; 
        last_cal_blink_time = now;
        current_state = STATE_CALIBRATE_WET;
    }
    else if (short_press) {
        dry_adc = (float)read_soil_adc();
        printf("Kuiva arvo tallennettu %.0f!\nAseta anturi MARKAAN ja paina nappia.\n", dry_adc);
        is_custom_calibrated = true;
        led_play_sequence(SEQ_SAVE);
        cal_state_enter_time = now; 
        last_cal_blink_time = now;
        current_state = STATE_CALIBRATE_WET;
    }
    break;

case STATE_CALIBRATE_WET:
    if (!is_pressed && (now - cal_state_enter_time >= 10000)) {
                printf("Kalibrointi aikakatkaistiin. Palataan normaaliin tilaan.\n");
                led_play_sequence(SEQ_STARTUP);
                sample_count = 0;
                temp_sum = humidity_sum = moisture_sum = adc_sum = 0.0f;
                last_sensor_read_time = now;
                current_state = STATE_NORMAL;
                network_request_start_time = now;
                break;
    }
    if (!is_pressed && (now - last_cal_blink_time >= 4000)) {
                led_play_sequence(SEQ_WET);
                last_cal_blink_time = now;
            }

    if (long_press || short_press) {
    if (long_press) {
        printf("Arvoa ei tallennettu. marka-arvo on %.0f.\n", wet_adc);
        led_play_sequence(SEQ_LOAD);
    }
    else if (short_press) {
        wet_adc = (float)read_soil_adc();
        printf("Marka arvo tallennettu %.0f!\n", wet_adc);
        is_custom_calibrated = true;
        led_play_sequence(SEQ_SAVE);
        
    }
    printf("Palataan takaisin normaaliin tilaan.\n\n");
    
    sample_count = 0;
    temp_sum = humidity_sum = moisture_sum = adc_sum = 0.0f;
    last_sensor_read_time = now;
    current_state = STATE_NORMAL;
    network_request_start_time = now;
    }
    break;
        
}
sleep_ms(1);
}
}

// Tämän funktion avulla saadaan arvot suhteelisena prosenttiarvona

float regression(float x)
{
    return 0.0000909081f * x * x - 0.244061f * x + 177.237f;
}

float calculate_moisture(uint16_t adc)
{


    if (adc <= dry_adc)
        return 0.0f;

    if (adc >= wet_adc)
        return 100.0f;
    if (wet_adc <= dry_adc) 
        return 0.0f;

    float moisture;
    if(is_custom_calibrated) {
        // Käyttäjäkalibrointi: Turvallinen lineaarinehn skaalaus koko ADC-alueelle
        moisture = ((float)adc-(float)dry_adc) / ((float)wet_adc - (float) dry_adc) * 100.0f;
    } else {

    moisture = (regression(adc) - regression(dry_adc)) / (regression(wet_adc) - regression(dry_adc)) * 100.0f;
    }
    if (moisture< 0.0f) return 0.0f;
    if (moisture > 100.0f) return 100.0f;

    return moisture;
}
