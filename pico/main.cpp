#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "ws2812.hpp"

#define LED_STRIP_LENGTH 49
#define LED_NUM_STRIPS 38
#define NUM_LEDS LED_STRIP_LENGTH * LED_NUM_STRIPS

LED led_buffer_1[NUM_LEDS];
LED led_buffer_2[NUM_LEDS];

// general loop
// fill one buffer with data from usb on core 0
// dma transfer other to leds on core 1
// once both are complete swap buffers

void core1_main();

int main() {
    stdio_init_all();
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    // initialize buffers
    for (int i = 0; i < NUM_LEDS; i++) {
        led_buffer_1[i] = {0, 0, 0};
        led_buffer_2[i] = {0, 0, 0};
    }
    // start core 1
    multicore_launch_core1(core1_main);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
    while (true) {
        
    }
}

void core1_main() {
    while (true) {
    }
}