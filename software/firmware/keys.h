// keys.h
// Authors: Jacob Mealey & Landyn Francis
#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>

// amount of keys in the keyboard
#define KEY_COUNT 12U

// The amount of difference between key.current_pos
// and key.prev_pos to be considered "pressed" this
// will most likely need to be tuned later.
#define KEY_THRESH 83U
#define KEY_UPPER 95U
#define KEY_LOWER 3U

typedef struct key
{
    uint8_t current_pos; // current position of the ADC
    uint8_t prev_pos;    // previous position of the ADC
} key;

typedef struct keyboard
{
    key keys[KEY_COUNT];
    uint8_t volume;
    uint8_t octave;
    uint8_t channel;
} keyboard;

extern struct keyboard *keyboard_global;

// Initialization function
struct keyboard *init_keys();

// individual key functions

// takes a type k and returns the midi acceptable velocity of it.
uint8_t key_get_velocity(key *k);

// takes a type k and returns the decimal calculation velocity in cm/s
double key_get_velocity_cms(key *k);

// struct keys functions
// takes a pointer to an instance of keys and a buffer to write into. this
// assumes midi_buffer is 3 * KEY_COUNT as each key takes 3 bytes TODO check
// returns 0 if succesfully generated midi data, 1 if not
int generate_midi_output(struct keyboard *ks, uint8_t *midi_buffer);

#endif