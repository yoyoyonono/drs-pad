#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "Adafruit_NeoPixel.hpp"
#include "brotli/c/include/brotli/decode.h"
#include "led.hpp"

#define LED_STRIP_LENGTH 49
#define LED_NUM_STRIPS 38
#define NUM_LEDS LED_STRIP_LENGTH * LED_NUM_STRIPS

#define LED_STRIP_1_PIN 17
#define LED_STRIP_2_PIN 18

uint8_t usb_read_buffer[NUM_LEDS * 3 + 1000];
uint8_t led_buffer[NUM_LEDS * 3];

size_t decompressed_size = sizeof(led_buffer);

Adafruit_NeoPixel strip_1 = Adafruit_NeoPixel(NUM_LEDS/2, LED_STRIP_1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_2 = Adafruit_NeoPixel(NUM_LEDS/2, LED_STRIP_2_PIN, NEO_GRB + NEO_KHZ800);

volatile bool usb_buffer = false; // false = buffer 1, true = buffer 2

// general loop
// fill one buffer with data from usb on core 0
// dma transfer other to leds on core 1
// once both are complete swap buffers

void core1_main();

int main() {
    // set frequency to 270 MHz
    set_sys_clock_khz(270000, true);


    sleep_ms(1000);

    stdio_uart_init_full(uart0, 460800, 0, 1);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // initialize strips
    strip_1.begin();
    strip_1.clear();

    // initialize buffers
    for (int i = 0; i < NUM_LEDS * 3; i++) {
        led_buffer[i] = 0;
    }
    // start core 1

    while (true) {
        // first two bytes are length and then read that many bytes from usb
        
        printf("a");
        uint16_t length = static_cast<uint8_t>(getchar()) << 8 | static_cast<uint8_t>(getchar());
        // read data from usb
        for (uint16_t i = 0; i < length; i++) {
            usb_read_buffer[i] = getchar();
        }

        // decompress data into buffer

        BrotliDecoderResult result = BrotliDecoderDecompress(length, usb_read_buffer, &decompressed_size, (uint8_t *)led_buffer);

        if (result == BROTLI_DECODER_RESULT_ERROR || decompressed_size != NUM_LEDS * 3) {
            printf("e");
        }
        
        for (uint16_t i = 0; i < NUM_LEDS/2; i++) {
            strip_1.setPixelColor(i, strip_1.Color(led_buffer[i * 3], led_buffer[i * 3 + 1], led_buffer[i * 3 + 2]));
        }
        for (uint16_t i = NUM_LEDS/2; i < NUM_LEDS; i++) {
            strip_2.setPixelColor(i, strip_1.Color(led_buffer[i * 3], led_buffer[i * 3 + 1], led_buffer[i * 3 + 2]));
        }
        strip_1.show();
//        strip_2.show();
        
        gpio_put(PICO_DEFAULT_LED_PIN, usb_buffer);        
        usb_buffer = !usb_buffer;
    }
}