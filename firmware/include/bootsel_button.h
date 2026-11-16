#ifndef BOOTSEL_BUTTON_H
#define BOOTSEL_BUTTON_H

#include <stdbool.h>

// Palauttaa true, jos Picon sisäinen BOOTSEL-painike on painettuna pohjaan.
bool read_bootsel_button(void);

#endif // BOOTSEL_BUTTON_H