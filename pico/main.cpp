#include <stdio.h>

#include "brotli/c/include/brotli/decode.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "led.hpp"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "ws2812_parallel.pio.h"

#define LED_STRIP_LENGTH 49
#define LED_NUM_STRIPS 38
#define NUM_LEDS LED_STRIP_LENGTH * LED_NUM_STRIPS

uint8_t usb_read_buffer[NUM_LEDS * 3 + 1000];
uint8_t led_buffer[NUM_LEDS * 3];

size_t decompressed_size = sizeof(led_buffer);

volatile bool usb_buffer = false;  // false = buffer 1, true = buffer 2

#define STRIPS_PER_CHANNEL 5
#define CHANNEL_LENGTH LED_STRIP_LENGTH * STRIPS_PER_CHANNEL
#define NUM_CHANNELS 32

#define REAL_NUM_CHANNELS 8

#define RESET_TIME_US 400
#define START_PIN 7

int dma_chan;

uint32_t pixels[CHANNEL_LENGTH * 24];

void set_pixel_colour(int pixel, int channel, uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t colour_value = (blue << 16 | red << 8 | green << 0);
    for (int i = 0; i < 24; i++) {
        if (colour_value & (1u << i)) {
            pixels[(pixel * 24) + i] |= (1u << channel);
        } else {
            pixels[(pixel * 24) + i] &= ~(1u << channel);
        }
    }
}

int64_t dma_start() {
    dma_channel_set_read_addr(dma_chan, pixels, true);
    return 0;
}

void dma_handler() {
    dma_irqn_acknowledge_channel(DMA_IRQ_0, dma_chan);
    add_alarm_in_us(RESET_TIME_US, (alarm_callback_t)dma_start, NULL, false);
}


// general loop
// fill one buffer with data from usb on core 0
// dma transfer other to leds on core 1
// once both are complete swap buffers

void core1_main();

int main() {
    // set frequency to 270 MHz
    set_sys_clock_khz(270000, true);

    sleep_ms(1000);

//    stdio_uart_init_full(uart0, 460800, 0, 1);
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // initialize pio

    PIO pio = pio0;
    int sm = 0;

    unsigned int offset = pio_add_program(pio, &ws2812_parallel_program);

    ws2812_parallel_program_init(pio, sm, offset, START_PIN, REAL_NUM_CHANNELS, 800000);

    dma_chan = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);

    channel_config_set_dreq(&c, DREQ_PIO0_TX0);

    dma_channel_configure(
        dma_chan,
        &c,
        &pio0_hw->txf[0],
        NULL,
        CHANNEL_LENGTH*24,
        false
    );

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_handler();

    // initialize buffers
    for (int i = 0; i < NUM_LEDS * 3; i++) {
        led_buffer[i] = 0;
    }
    // start core 1

    getchar();

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

        // set pixel colours and show

        dma_channel_set_irq0_enabled(dma_chan, false);
        irq_set_enabled(DMA_IRQ_0, false);
        for (int i = 0; i < NUM_LEDS; i++) {
            set_pixel_colour(i % 245 , i / (LED_STRIP_LENGTH * STRIPS_PER_CHANNEL), led_buffer[i * 3], led_buffer[i * 3 + 1], led_buffer[i * 3 + 2]);
        }
        dma_channel_set_irq0_enabled(dma_chan, true);
        irq_set_enabled(DMA_IRQ_0, true);

        gpio_put(PICO_DEFAULT_LED_PIN, usb_buffer);
        usb_buffer = !usb_buffer;
    }
}