#include "keys.h"
#include "stdlib.h"

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
    struct keyboard *keyb = malloc(sizeof(struct keyboard));

    keyb->channel = 0;
    keyb->octave = 5;
    keyb->volume = 64;
    for (uint32_t i = 0; i < KEY_COUNT; i++){
        keyb->keys[i].current_pos = 0x5C;
        keyb->keys[i].prev_pos = 0x5C;
        keyb->keys[i].pressed = 0;
    }
    return keyb;
}
