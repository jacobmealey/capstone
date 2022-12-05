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
	// Set UART GPIO functions (Debugging)
	gpio_set_function(UART0_TX, GPIO_FUNC_UART);
	gpio_set_function(UART0_RX, GPIO_FUNC_UART);
	printf("Hello, Midi!\n");

	// Set SPI1 GPIO functions (Display)
	gpio_set_function(SPI1_SCLK, GPIO_FUNC_SPI);
	gpio_set_function(SPI1_RX, GPIO_FUNC_SPI);
	gpio_set_function(SPI1_TX, GPIO_FUNC_SPI);
	gpio_set_function(SPI1_CS, GPIO_FUNC_SPI);
	printf("alternate functions for ADC set\n");

	// Set SPI0 GPIO functions (Display)
	gpio_set_function(SPI0_SCLK, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_RX, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_TX, GPIO_FUNC_SPI);
	// Set SPI0 pull downs
	gpio_set_pulls(SPI0_SCLK, false, true);
	gpio_set_pulls(SPI0_TX, false, true);
	printf("alternate functions for Display set\n");

	// LED pin definitions and initialization
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
	gpio_set_pulls(ENCODE_PRESS, true, false);
	gpio_set_dir(ENCODE_PRESS, GPIO_IN);

	gpio_init(ENCODE_A);
	// Fast slew rate
	gpio_set_slew_rate(ENCODE_A, GPIO_SLEW_RATE_FAST);
	// Pull up
	gpio_set_pulls(ENCODE_A, true, false);
	// Input
	gpio_set_dir(ENCODE_A, GPIO_IN);

	gpio_init(ENCODE_B);
	// Fast slew rate
	gpio_set_slew_rate(ENCODE_B, GPIO_SLEW_RATE_FAST);
	// Pull up
	gpio_set_pulls(ENCODE_B, true, false);
	// Input
	gpio_set_dir(ENCODE_B, GPIO_IN);

	// GPIO Interrupt Setup
	// All GPIO interrupts use the same callback function, must determine which GPIO line triggered the callback
	gpio_set_irq_enabled_with_callback(ENCODE_PRESS, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	gpio_set_irq_enabled(OCT_DOWN, GPIO_IRQ_EDGE_FALL, true);
	gpio_set_irq_enabled(OCT_UP, GPIO_IRQ_EDGE_FALL, true);
	gpio_set_irq_enabled(ENCODE_A, GPIO_IRQ_EDGE_FALL, true);
	return 0;
}

// GPIO IRQ Trigger lookup table (used for GPIO event debugging)
static const char *gpio_irq_str[] = {
	"LEVEL_LOW",  // 0x1
	"LEVEL_HIGH", // 0x2
	"EDGE_FALL",  // 0x4
	"EDGE_RISE"	  // 0x8
};

// Convert GPIO event to a printable string (GPIO Event debugging)
void gpio_event_string(char *buf, uint32_t events)
{
	for (uint i = 0; i < 4; i++)
	{
		uint mask = (1 << i);
		if (events & mask)
		{
			// Copy this event string into the user string
			const char *event_str = gpio_irq_str[i];
			while (*event_str != '\0')
			{
				*buf++ = *event_str++;
			}
			events &= ~mask;

			// If more events add ", "
			if (events)
			{
				*buf++ = ',';
				*buf++ = ' ';
			}
		}
	}
	*buf++ = '\0';
}

// GPIO Interrupt callback function
void gpio_callback(uint gpio, uint32_t events)
{
	// Disable interrupts while servicing GPIO interrupt
	int status = save_and_disable_interrupts();
	char event_str[128];
	int volume_offset;
	// Print GPIO Event
	gpio_event_string(event_str, events);
	printf("GPIO IRQ CALLBACK\n GPIO Num %d\n, Event %s\n", gpio, event_str);

	// Determine which GPIO pin triggered the interrupt
	switch (gpio)
	{
	// Rotary Encoder switch
	case ENCODE_PRESS:
		// Mute MIDI Volume (essentially sends a Note OFF for all notes)
		mute_midi_volume(keyboard_global->channel);
		break;
	case OCT_DOWN:
		// Don't allow users to go below octave 2
		if (keyboard_global->octave <= 2)
		{ 
			break;
		}
		// Decrement octave
		keyboard_global->octave--;
		// Turn off all MIDI Notes (changing octave with key press can leave higher octave note playing)
		mute_midi_volume(0);
		break;
	case OCT_UP:
		// Don't allow users to go above octave 8
		if (keyboard_global->octave >= 8)
		{ 
			break;
		}
		// Increment octave
		keyboard_global->octave++;
		// Turn off all MIDI Notes (changing octave with key press can leave lower octave note playing)
		mute_midi_volume(0);
		break;
	case ENCODE_A:
		// Amount to adjust volume 
		volume_offset = 0;
		// Check other Rotary Encoder line
		if (gpio_get(ENCODE_B) == 0)
		{ // Clockwise
			// Increment offset
			volume_offset++;
			// Don't allow negative volume
			if (volume_offset > keyboard_global->volume)
			{
				keyboard_global->volume = 0; 
			}
			else
			{
				// Apply volume decrease
				keyboard_global->volume -= volume_offset;
			}
		}
		// Otherwise, Counter-clockwise
		else
		{ 
			// Increment offset
			volume_offset++;
			// Don't allow volumes over 127
			if (keyboard_global->volume + volume_offset > 127)
			{
				keyboard_global->volume = 127; 
			}
			else
			{
				// Apply volume increase
				keyboard_global->volume += volume_offset;
			}
		}
		// Send volume MIDI message
		change_midi_volume(0, keyboard_global->volume);
		break;
	}
	// Re-enable interrupts
	restore_interrupts(status);
}
