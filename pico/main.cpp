#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "Adafruit_NeoPixel.hpp"
#include "led.hpp"

#define LED_STRIP_LENGTH 49
#define LED_NUM_STRIPS 38
#define NUM_LEDS LED_STRIP_LENGTH * LED_NUM_STRIPS

#define LED_STRIP_1_PIN 2
#define LED_STRIP_2_PIN 3
#define LED_STRIP_3_PIN 4
#define LED_STRIP_4_PIN 5
#define LED_STRIP_5_PIN 6
#define LED_STRIP_6_PIN 7
#define LED_STRIP_7_PIN 8
#define LED_STRIP_8_PIN 9

LED led_buffer_1[NUM_LEDS];
LED led_buffer_2[NUM_LEDS];

Adafruit_NeoPixel strip_1 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 5, LED_STRIP_1_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_2 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 5, LED_STRIP_2_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_3 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 5, LED_STRIP_3_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_4 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 5, LED_STRIP_4_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_5 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 5, LED_STRIP_5_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_6 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 5, LED_STRIP_6_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_7 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 4, LED_STRIP_7_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_8 = Adafruit_NeoPixel(LED_STRIP_LENGTH * 4, LED_STRIP_8_PIN, NEO_GRB + NEO_KHZ800);

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
        for (int i = 0; i < LED_STRIP_LENGTH * 5; i++) {
            strip_1.setPixelColor(i, strip_1.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
        }
//        for (int i = LED_STRIP_LENGTH * 5; i < LED_STRIP_LENGTH * 10; i++) {
//            strip_2.setPixelColor(i, strip_2.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
//        for (int i = LED_STRIP_LENGTH * 10; i < LED_STRIP_LENGTH * 15; i++) {
//            strip_3.setPixelColor(i, strip_3.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
//        for (int i = LED_STRIP_LENGTH * 15; i < LED_STRIP_LENGTH * 20; i++) {
//            strip_4.setPixelColor(i, strip_4.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
//        for (int i = LED_STRIP_LENGTH * 20; i < LED_STRIP_LENGTH * 25; i++) {
//            strip_5.setPixelColor(i, strip_5.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
//        for (int i = LED_STRIP_LENGTH * 25; i < LED_STRIP_LENGTH * 30; i++) {
//            strip_6.setPixelColor(i, strip_6.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
//        for (int i = LED_STRIP_LENGTH * 30; i < LED_STRIP_LENGTH * 34; i++) {
//            strip_7.setPixelColor(i, strip_7.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
//        for (int i = LED_STRIP_LENGTH * 34; i < LED_STRIP_LENGTH * 38; i++) {
//            strip_8.setPixelColor(i, strip_8.Color(led_buffer[i].r, led_buffer[i].g, led_buffer[i].b));
//        }
        strip_1.show();
//        strip_2.show();
//        strip_3.show();
//        strip_4.show();
//        strip_5.show();
//        strip_6.show();
//        strip_7.show();
//        strip_8.show();
        dma_transfer_finished = true;
        while (!usb_transfer_finished) {
            tight_loop_contents();
        }
    }
}