// File: gpio.c
// Date Created: 9/18/22
// Authors: Landyn Francis (landyn.francis@maine.edu) Jacob Mealey (jacob.mealey@maine.edu)
// Purpose: Source file containing gpio initialization and interrupt functions
#include "pins.h"
#include <stdint.h>
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include <stdio.h>
#include "keys.h"
#include "midi.h"

uint8_t pin_init()
{

	// UART pin defs
	gpio_set_function(UART0_TX, GPIO_FUNC_UART);
	gpio_set_function(UART0_RX, GPIO_FUNC_UART);
	printf("Hello, Midi!\n");

	// SPI1 pin defs (ADC)
	gpio_set_function(SPI1_SCLK, GPIO_FUNC_SPI);
	gpio_set_function(SPI1_RX, GPIO_FUNC_SPI);
	gpio_set_function(SPI1_TX, GPIO_FUNC_SPI);
	gpio_set_function(SPI1_CS, GPIO_FUNC_SPI);
	printf("alternate functions for ADC set\n");

	// SPI0 pin defs (Display)
	gpio_set_function(SPI0_SCLK, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_RX, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_TX, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_CS, GPIO_FUNC_SPI);
	printf("alternate functions for Display set\n");

	// LED pin defs
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	// Button Initialization
	gpio_init(RESET);
	gpio_set_dir(RESET, GPIO_IN);

	gpio_init(OCT_DOWN);
	gpio_set_dir(OCT_DOWN, GPIO_IN);

	gpio_init(OCT_UP);
	gpio_set_dir(OCT_UP, GPIO_IN);

	gpio_init(ENCODE_PRESS);
	gpio_set_dir(ENCODE_PRESS, GPIO_IN);

	// GPIO Interrupt Setup
	gpio_set_irq_enabled_with_callback(ENCODE_PRESS, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	gpio_set_irq_enabled(OCT_DOWN,GPIO_IRQ_EDGE_FALL,true);
	gpio_set_irq_enabled(OCT_UP,GPIO_IRQ_EDGE_FALL,true);

	return 0;
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

void gpio_callback(uint gpio, uint32_t events)
{
	char event_str[128];
	gpio_event_string(event_str, events);
	printf("GPIO IRQ CALLBACK\n GPIO Num %d\n, Event %s\n", gpio, event_str);
	
	switch(gpio){
		case ENCODE_PRESS:
			mute_midi_volume(keyboard_global->channel);
			break;
		case OCT_DOWN:
			//Lower octave on keyboard struct
			keyboard_global->octave--;
			printf("Keyboard Octave: %d\n",keyboard_global->octave);

			break;
		case OCT_UP:
			//Increase octave on keyboard struct
			printf("Keyboard Octave: %d\n",keyboard_global->octave);
			keyboard_global->octave++;
			printf("Keyboard Octave: %d\n",keyboard_global->octave);

			break;
	}

	gpio_put(PICO_DEFAULT_LED_PIN, 0);
}