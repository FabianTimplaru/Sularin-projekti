# Luodaan kirjasto nimeltä dht11_driver
add_library(dht11_driver dht11.c)

# Kerrotaan, mistä dht11.h löytyy (tästä samasta kansiosta)
target_include_directories(dht11_driver PUBLIC .)

# DHT11 vaatii Picon GPIO- ja timer-toimintoja signaalin lukemiseen
target_link_libraries(dht11_driver pico_stdlib hardware_gpio hardware_timer )