#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "led.hpp"

#define LED_STRIP_LENGTH 49
#define LED_NUM_STRIPS 38
#define NUM_LEDS LED_STRIP_LENGTH * LED_NUM_STRIPS

LED led_buffer_1[NUM_LEDS];
LED led_buffer_2[NUM_LEDS];

volatile bool usb_transfer_finished = false;
volatile bool dma_transfer_finished = false;

volatile bool usb_buffer = false; // false = buffer 1, true = buffer 2
volatile bool dma_buffer = true; // false = buffer 1, true = buffer 2

// general loop
// fill one buffer with data from usb on core 0
// dma transfer other to leds on core 1
// once both are complete swap buffers

void core1_main();

int main() {
    // set frequency to 270 MHz
    set_sys_clock_khz(270000, true);

    sleep_ms(1000);

    stdio_init_all();
    printf("init\n");
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    // initialize buffers
    for (int i = 0; i < NUM_LEDS; i++) {
        led_buffer_1[i] = {0, 0, 0};
        led_buffer_2[i] = {0, 0, 0};
    }
    // start core 1
    multicore_launch_core1(core1_main);

    while (true) {
        LED *led_buffer = usb_buffer ? led_buffer_2 : led_buffer_1;
        for (int i = 0; i < NUM_LEDS; i++) {
            led_buffer[i] = {(uint8_t)getchar(), (uint8_t)getchar(), (uint8_t)getchar()};
        }
        usb_buffer = !usb_buffer;
        usb_transfer_finished = true;
        while (!dma_transfer_finished) {
            tight_loop_contents();
        }
        printf("a");
        gpio_put(PICO_DEFAULT_LED_PIN, usb_buffer);        
        dma_transfer_finished = false;
        dma_buffer = !dma_buffer;
    }
}

void core1_main() {
    while (true) {
        LED *led_buffer = dma_buffer ? led_buffer_2 : led_buffer_1; 
        sleep_us(NUM_LEDS * 30 + 50);
        dma_transfer_finished = true;
        while (!usb_transfer_finished) {
            tight_loop_contents();
        }
    }
}