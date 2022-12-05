#include "keys.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "adc_pos.h"

// Pass a key and get the centimeter per second velocity
// Uses start_pos, start_time, end_pos, and end_time, stored within the key struct
double key_get_velocity_cms(key *k) {
    // Change in distance
    double delta_x = adc_pos_lut[k->start_pos] - adc_pos_lut[k->end_pos];
    // Change in time
    double delta_t = to_us_since_boot(k->end_time) - to_us_since_boot(k->start_time);
    // Return velocity
    return (delta_x)/(delta_t / 1000000.0);
}

// Initialize global keyboard
struct keyboard *init_keys(){
    // Allocate space in memory for the full keyboard
    struct keyboard *keyb = malloc(sizeof(struct keyboard));

    // Initialize default channel to 0
    keyb->channel = 0;

    // Initialize default octave to Middle 5
    keyb->octave = 5;

    // Initialize volume to half
    keyb->volume = 64;
    
    // Initialize last_pressed to 0
    keyb->last_pressed = 0;

    // Initialize each key in keyboard
    for (uint32_t i = 0; i < KEY_COUNT; i++){
        // Set current and previous position to top
        keyb->keys[i].current_pos = 0x5C;
        keyb->keys[i].prev_pos = 0x5C;
        // Initalize to not pressed
        keyb->keys[i].pressed = 0;
        // Initialize to not active
        keyb->keys[i].active = 0;
    }
    return keyb;
}
