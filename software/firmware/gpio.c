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
#include "pico/sync.h"

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
	//gpio_set_function(SPI0_CS, GPIO_FUNC_SPI);
	printf("alternate functions for Display set\n");

	gpio_set_pulls(SPI0_SCLK,false,true);
	gpio_set_pulls(SPI0_TX,false,true);

	// LED pin defs
	gpio_init(LED_0);
	gpio_set_dir(LED_0, GPIO_OUT);

	gpio_init(LED_1);
	gpio_set_dir(LED_1, GPIO_OUT);

	gpio_init(LED_2);
	gpio_set_dir(LED_2, GPIO_OUT);

	gpio_init(LED_3);
	gpio_set_dir(LED_3, GPIO_OUT);

	// Button Initialization

	gpio_init(OCT_DOWN);
	gpio_set_dir(OCT_DOWN, GPIO_IN);

	gpio_init(OCT_UP);
	gpio_set_dir(OCT_UP, GPIO_IN);

	gpio_init(ENCODE_PRESS);
	gpio_set_pulls(ENCODE_PRESS,true,false);
	gpio_set_dir(ENCODE_PRESS, GPIO_IN);

	gpio_init(ENCODE_A);
	gpio_set_slew_rate(ENCODE_A,GPIO_SLEW_RATE_FAST);
	gpio_set_pulls(ENCODE_A,true,false);
	gpio_set_dir(ENCODE_A, GPIO_IN);

	gpio_init(ENCODE_B);
	gpio_set_slew_rate(ENCODE_B,GPIO_SLEW_RATE_FAST);
	gpio_set_pulls(ENCODE_B,true,false);
	gpio_set_dir(ENCODE_B, GPIO_IN);


	// GPIO Interrupt Setup
	gpio_set_irq_enabled_with_callback(ENCODE_PRESS, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	gpio_set_irq_enabled(OCT_DOWN,GPIO_IRQ_EDGE_FALL,true);
	gpio_set_irq_enabled(OCT_UP,GPIO_IRQ_EDGE_FALL,true);
	gpio_set_irq_enabled(ENCODE_A, GPIO_IRQ_EDGE_FALL, true);
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
	int status = save_and_disable_interrupts();
	char event_str[128];
	int volume_offset;
	gpio_event_string(event_str, events);
	printf("GPIO IRQ CALLBACK\n GPIO Num %d\n, Event %s\n", gpio, event_str);
	
	switch(gpio){
		case ENCODE_PRESS:
			printf("Mute\n");
			mute_midi_volume(keyboard_global->channel);
			break;
		case OCT_DOWN:
			//Lower octave on keyboard struct
			if (keyboard_global->octave <= 2){//Don't allow users to go below octave 2
				printf("Keyboard Octave: %d\n",keyboard_global->octave);
				break;
			}
			keyboard_global->octave--;
			printf("Keyboard Octave: %d\n",keyboard_global->octave);

			break;
		case OCT_UP:
			//Increase octave on keyboard struct
			if (keyboard_global->octave >= 8){//Don't allow users to go above octave 8
				printf("Keyboard Octave: %d\n",keyboard_global->octave);
				break;
			}
			printf("Keyboard Octave: %d\n",keyboard_global->octave);
			keyboard_global->octave++;
			break;
		case ENCODE_A:
			volume_offset = 0;
			if(gpio_get(ENCODE_B)==0){ //Clockwise
				printf("VOLUME DOWN\n");
				volume_offset++;
				if (volume_offset > keyboard_global->volume){
					keyboard_global->volume=0; //Set to 0 if negative
				}else{
					keyboard_global->volume-=volume_offset;
				}	
			}else{ //Otherwise, Counter
				

				printf("VOLUME UP\n");
				volume_offset++;
				if (keyboard_global->volume+volume_offset > 127){
					keyboard_global->volume=127; //Set to max if over 127
				}else{
					keyboard_global->volume+=volume_offset;
				}
			}
			printf("Volume offset:%d\n", volume_offset);
			change_midi_volume(0,keyboard_global->volume);//This can move to the main function I think
			break;
	}

	restore_interrupts(status);
}

