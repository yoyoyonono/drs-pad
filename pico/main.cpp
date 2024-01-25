#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "led.hpp"

#define LED_STRIP_LENGTH 49
#define LED_NUM_STRIPS 38
#define NUM_LEDS LED_STRIP_LENGTH * LED_NUM_STRIPS

LED led_buffer_1[NUM_LEDS];
LED led_buffer_2[NUM_LEDS];

LED strip_1[LED_STRIP_LENGTH * 5];
LED strip_2[LED_STRIP_LENGTH * 5];
LED strip_3[LED_STRIP_LENGTH * 5];
LED strip_4[LED_STRIP_LENGTH * 5];
LED strip_5[LED_STRIP_LENGTH * 5];
LED strip_6[LED_STRIP_LENGTH * 5];
LED strip_7[LED_STRIP_LENGTH * 4];
LED strip_8[LED_STRIP_LENGTH * 4];

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
        // add leds from buffer to strip buffers in zig-zag pattern
        // if even strip, add normally else add in reverse order
        for (int i = 0; i < NUM_LEDS; i++) {
            bool is_even_strip = (i / LED_STRIP_LENGTH) % 2;
            if (i < LED_STRIP_LENGTH * 5) {
                if (is_even_strip) {
                    strip_1[i] = led_buffer[i];
                } else {
                    strip_1[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH)] = led_buffer[i];
                }
            } else if (i < LED_STRIP_LENGTH * 10) {
                if (is_even_strip) {
                    strip_2[i - LED_STRIP_LENGTH * 5] = led_buffer[i];
                } else {
                    strip_2[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 5] = led_buffer[i];
                }
            } else if (i < LED_STRIP_LENGTH * 15) {
                if (is_even_strip) {
                    strip_3[i - LED_STRIP_LENGTH * 10] = led_buffer[i];
                } else {
                    strip_3[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 10] = led_buffer[i];
                }
            } else if (i < LED_STRIP_LENGTH * 20) {
                if (is_even_strip) {
                    strip_4[i - LED_STRIP_LENGTH * 15] = led_buffer[i];
                } else {
                    strip_4[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 15] = led_buffer[i];
                }
            } else if (i < LED_STRIP_LENGTH * 25) {
                if (is_even_strip) {
                    strip_5[i - LED_STRIP_LENGTH * 20] = led_buffer[i];
                } else {
                    strip_5[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 20] = led_buffer[i];
                }
            } else if (i < LED_STRIP_LENGTH * 30) {
                if (is_even_strip) {
                    strip_6[i - LED_STRIP_LENGTH * 25] = led_buffer[i];
                } else {
                    strip_6[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 25] = led_buffer[i];
                }
            } else if (i < LED_STRIP_LENGTH * 34) {
                if (is_even_strip) {
                    strip_7[i - LED_STRIP_LENGTH * 30] = led_buffer[i];
                } else {
                    strip_7[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 30] = led_buffer[i];
                }
            } else {
                if (is_even_strip) {
                    strip_8[i - LED_STRIP_LENGTH * 34] = led_buffer[i];
                } else {
                    strip_8[LED_STRIP_LENGTH - 1 - (i % LED_STRIP_LENGTH) + LED_STRIP_LENGTH * 34] = led_buffer[i];
                }
            }
        }
        sleep_us(NUM_LEDS / 8 * 30 + 50);
        dma_transfer_finished = true;
        while (!usb_transfer_finished) {
            tight_loop_contents();
        }
    }
}