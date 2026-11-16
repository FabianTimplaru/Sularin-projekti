#ifndef PICO_LED_H
#define PICO_LED_H
#include <stdint.h>
#include <stdbool.h>
// LED:n vilkutus sekvenssit
#define SEQ_WIFI_SEND   "000000" // 6 lyhyttä
#define SEQ_BACKEND_OK  "111"    // 3 pitkää
#define SEQ_BACKEND_ERR "0001"   // 3 lyhyttä, 1 pitkä
#define SEQ_STARTUP "11"
#define SEQ_ONLINE "10001"
#define SEQ_OFFLINE "111111"


// Kytkee Pico W:n sisäisen LEDin päälle (true) tai pois (false)
void set_internal_led(bool state);

// Vaihtaa LEDin tilaa vastakkaiseksi (päälle -> pois, pois -> päälle)
void toggle_internal_led(void);

// Sekvensserin funktiot
void led_sequencer_init(void);
void led_play_sequence(const char *seq);
void led_set_continuous_long(bool active);
void led_sequencer_update(uint32_t now_ms); // Kutsutaan while-silmukassa

#endif // PICO_LED_H