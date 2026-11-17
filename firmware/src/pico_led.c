#include "pico_led.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <string.h>

static char current_seq[64] = {0};
static int seq_index = 0;
static int tick_counter = 0;
static bool in_gap = false;
static bool continuous_mode = false;

static int on_ticks = 0;
static int gap_ticks = 0;

static uint32_t last_led_tick_time = 0;
static const uint32_t led_tick_interval = 100; // 100 ms per askel

void set_internal_led(bool state) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
}

void toggle_internal_led(void) {
    static bool current_state = false;
    current_state = !current_state;
    set_internal_led(current_state);
}

void led_play_sequence(const char *seq) {
    strncpy(current_seq, seq, sizeof(current_seq) - 1);
    current_seq[sizeof(current_seq) - 1] = '\0';
    seq_index = 0;
    tick_counter = 0;
    in_gap = false;
}

void led_set_continuous_long(bool active) {
    if (continuous_mode != active) {
        continuous_mode = active;
        tick_counter = 0;
        if (!active) {
            set_internal_led(false);
        }
    }
}

void led_sequencer_update(uint32_t now_ms) {
    // Tarkistetaan, onko 100 ms kulunut
    if (now_ms - last_led_tick_time < led_tick_interval) {
        return;
    }
    last_led_tick_time = now_ms;

    // 1. Jos nappi on pohjassa, toistetaan pitkää pulssia (400 ms päällä, 200 ms pois)
    if (continuous_mode) {
        tick_counter++;
        if (tick_counter <= 3) {
            set_internal_led(true);
        } else if (tick_counter <= 4) {
            set_internal_led(false);
        } else {
            tick_counter = 0;
        }
        return;
    }

    // 2. Jos ei ole sekvenssiä menossa, pidetään LED sammutettuna
    if (current_seq[seq_index] == '\0') {
        set_internal_led(false);
        return;
    }

    // Morse-käsittely
    if (!in_gap) {
        char c = current_seq[seq_index];

        // Määritetään merkin päälläoloaika ja sitä seuraava tauko
        if (c == '.') {
            on_ticks = 1;
            gap_ticks = 1;
        } else if (c == '-') {
            on_ticks = 3;
            gap_ticks = 1;
        } else if (c == ' ') {
            on_ticks = 0;
            gap_ticks = 3;
        } else if (c == '/') {
            on_ticks = 0;
            gap_ticks = 7;
        } else {
            // Tuntematon merkki, ohitetaan 1 yksikön tauolla
            on_ticks = 0;
            gap_ticks = 1;
        }
    

    if (on_ticks > 0) {
        set_internal_led(true);
        tick_counter++;
        if (tick_counter >= on_ticks) {
            in_gap = true;
            tick_counter = 0;
        }
     } else {
            // Pelkkä tauko (tuntematonm merkki, välilyönti tai /), siirrytään suoraan gap-vaiheeseen
            set_internal_led(false);
            in_gap = true;
            tick_counter = 0;
        }
    } else {
        // Gap-vaihe (LED sammutettuna tauon ajan)
        set_internal_led(false);
        tick_counter++;
        if (tick_counter >= gap_ticks) {
            in_gap = false;
            tick_counter = 0;
            seq_index++;
    }
}
}