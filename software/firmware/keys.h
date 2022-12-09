// keys.h
// Authors: Jacob Mealey & Landyn Francis
// Purpose: See keys.c
#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>
#include "pico/time.h"

// amount of keys in the keyboard
#define KEY_COUNT 12U

// The point at which a key is "pressed"
#define KEY_THRESH 63

// Largest ADC Value
#define KEY_UPPER 95U

// Lowest ADC Value
#define KEY_LOWER 3U

// Individual Key Structure: A structure that holds relevant (individual) key data
// current_pos: key position from current read
// prev_pos: key position from previous read
// pressed: Falling/Rising edge flag
// active: Held down key flag
// start_time: Timestamp of beginning of keypress
// end_time: Timestamp of end of keypress
// start_pos: Position of beggining of keypress
// end_pos: Position of end of keypress
typedef struct key
{
    uint8_t current_pos;        // Current position of the key (ADC Value)
    uint8_t prev_pos;           // Previous position of the key (ADC Value)
    uint8_t pressed;            // Flag showing whether or not the key has been pressed.
    uint8_t active;             // Flag showing whether or not the key is active. (Being held down)
    absolute_time_t start_time; // The time when a key press begins
    absolute_time_t end_time;   // The time when a key press ends
    uint8_t start_pos;          // The position from the start of a keypress (ADC Value)
    uint8_t end_pos;            // The position from the end of a keypress (ADC Value)
    absolute_time_t midi_start; // Timer start for MIDI Velocity
    absolute_time_t midi_end;   // Timer end for MIDI Velocity
    uint8_t midi_active;
    float activation_height;

} key;

// Keyboard Structure: A structure that holds relevant keyboard data
// keys[]: Array of key structs for each individual key
// volume: 8 bit MIDI volume value
// channel: MIDI Channel (stays as 0)
// last_pressed: last pressed key number (0-11)
typedef struct keyboard
{
    key keys[KEY_COUNT];
    uint8_t volume;       // MIDI Volume (0-127)
    uint8_t octave;       // Keyboard Octave (Range of 2-8 Allowed)
    uint8_t channel;      // MIDI Channel (stays as 0)
    uint8_t last_pressed; // Last pressed key number (0-11)
} keyboard;

// Declare global keyboard structure
extern struct keyboard *keyboard_global;

// Initialization function
struct keyboard *init_keys();

// Individual key functions

// takes a type k and returns the decimal calculation velocity in cm/s
double key_get_velocity_cms(key *k);

#endif
