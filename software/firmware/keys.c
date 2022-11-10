#include "keys.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "adc_pos.h"

uint8_t key_get_velocity(key *k) {
    // TODO

    return 0;
}
double key_get_velocity_cms(key *k) {
    static float cm_per_step = 0.0161;
    double delta_x = adc_pos_lut[k->start_pos] - adc_pos_lut[k->end_pos];
    double delta_t = to_ms_since_boot(k->end_time) - to_ms_since_boot(k->start_time);
    return (delta_x)/(delta_t / 1000.0);
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
        keyb->keys[i].active = 0;
    }
    return keyb;
}
