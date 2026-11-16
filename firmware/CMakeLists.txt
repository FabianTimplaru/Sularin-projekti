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
set(PICO_BOARD pico2_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(PicoW_WeatherStation C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

# Lisätään alikansio DHT11-ajurille
add_subdirectory(lib/dht11)

file(GLOB_RECURSE SRC_FILES "src/*.c")
add_executable(PicoApp ${SRC_FILES})

# Koska wifi_config.h on firmware-kansion juuressa, lisätään se include-polkuihin
target_include_directories(PicoApp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # wifi_config.h löytyy täältä
)

# Linkitetään Wi-Fi, lwIP (verkko), vakiokirjasto ja oma DHT11-kirjasto
target_link_libraries(PicoApp 
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background # Pico W Wi-Fi + verkkoyhteydet
    hardware_adc
    dht11_driver
)

pico_enable_stdio_usb(PicoApp 1)
pico_enable_stdio_uart(PicoApp 0)

pico_add_extra_outputs(PicoApp)