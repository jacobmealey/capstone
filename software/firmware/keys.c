#include "keys.h"

uint8_t key_get_velocity(key *k) {
    // TODO

    return 0;
}
double key_get_velocity_cms(key *k) {
    // TODO 
    return 0.;
}

int generate_midi_output(struct keyboard *ks, uint8_t *midi_buffer) {
    // TODO
    return 0;
}

struct keyboard *init_keys(){ 
    keyboard_global->channel = 0;
    keyboard_global->octave = 3;
    keyboard_global->volume = 64;
    for (uint32_t i = 0; i < KEY_COUNT; i++){
        keyboard_global->keys[i].current_pos = 0;
        keyboard_global->keys[i].prev_pos = 0;
    }
}
