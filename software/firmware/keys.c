#include "keys.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

uint8_t key_get_velocity(key *k) {
    // TODO

    return 0;
}
double key_get_velocity_cms(key *k) {
    static float cm_per_step = 0.0161;
    int delta_x = k->prev_pos - k->current_pos;
    if(delta_x == 0){
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    }else {
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
    }
    return (delta_x * cm_per_step)/0.005;
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
