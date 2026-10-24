# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

# Määritellään käytettävä kortti. Oletuksena Pico 2 W, mutta asetusta voidaan muuttaa.
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Tuodaan Pico SDK (varmista että PICO_SDK_PATH on asetettu ympäristömuuttujiin)
include(pico_sdk_import.cmake)

# Luodaan compile_commands.json IntelliSenseä varten
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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