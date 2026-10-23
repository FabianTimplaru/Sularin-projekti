cmake_minimum_required(VERSION 3.13)

# Tuodaan Pico SDK (varmista että PICO_SDK_PATH on asetettu ympäristömuuttujiin)
include(pico_sdk_import.cmake)

project(mullan_kosteusmittari C CXX ASM)

# Alustetaan Pico SDK
pico_sdk_init()

# Määritellään suoritettava tiedosto ja sen lähdekoodit
add_executable(kosteusmittari
    main.c
)

# Linkitetään tarvittavat kirjastot (standardikirjasto ja ADC-laitteistokirjasto)
target_link_libraries(kosteusmittari
    pico_stdlib
    hardware_adc
)

# Ohjataan standarditulostus (printf) USB-porttiin UARTin sijaan
pico_enable_stdio_usb(kosteusmittari 1)
pico_enable_stdio_uart(kosteusmittari 0)

# Generoidaan UF2-tiedosto, joka on helppo raahata Picon muistiin
pico_add_extra_outputs(kosteusmittari)