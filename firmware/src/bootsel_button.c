#include "bootsel_button.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

// Funktio pakotetaan RAM-muistiin, koska Flash-muisti irrotetaan hetkeksi luvun ajaksi.
bool __no_inline_not_in_flash_func(read_bootsel_button)(void) {
    const uint CS_PIN_INDEX = 1;

    // 1. Tallennetaan keskeytystila ja poistetaan keskeytykset käytöstä.
    uint32_t flags = save_and_disable_interrupts();

    // 2. Asetetaan QSPI CS -pinni Hi-Z (korkea impedanssi) -tilaan painikkeen lukemista varten.
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // 3. Odotetaan hetki signaalin tasaantumista (kasvatettu 50 -> 1000 Pico 2:lle).
    for (volatile int i = 0; i < 1000; ++i) {
        __asm volatile ("nop");
    }

    // 4. Käytetään SDK:n vakiota, joka lukee oikean bitin sekä Pico 1:llä että Pico 2:lla.
#if PICO_RP2040
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));
#else
    bool button_state = !(sio_hw->gpio_hi_in & SIO_GPIO_HI_IN_QSPI_CSN_BITS);
#endif

    // 5. Palautetaan pinnin alkuperäinen tila, jotta Flash-muisti toimii taas.
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // 6. Palautetaan keskeytykset takaisin päälle.
    restore_interrupts(flags);

    return button_state;
}