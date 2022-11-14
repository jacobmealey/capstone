// File: main.c
// Date: 10/25/22
// Authors: Jacob Mealey & Landyn Francis
// Purpose: Main loop :)

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bsp/board.h"
#include "tusb.h"

#include "adc.h"
#include "pins.h"
#include "midi.h"
#include "keys.h"
#include "display.h"

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#define DISP_SIZE 30720
int keyboard_task();

void midi_task(struct adc_t *adc);
void core1_main();

struct adc_t *adc_global;
struct keyboard *keyboard_global;
struct disp_t *disp_global;

// A queue for sending the current state of the keyboard
// to the second core 
queue_t key_state_q;


/*------------- MAIN -------------*/
int main(void) {
 
    stdio_init_all();
    uart_init(uart0, 9600);
    
    //Initialize GPIO pins
    pin_init();

    //Initialize ADC
    gpio_put(LED_0,1);
    adc_global = init_adc(spi1, SPI1_CS);
    gpio_put(LED_0,0);
    printf("ADC initialized\n");


    //Initialize Keyboard
    gpio_put(LED_1,1);
    keyboard_global = init_keys();
    gpio_put(LED_1,0);
    printf("Keyboard Initialized"); 

    //Initialize USB
    gpio_put(LED_2,1);
    tusb_init();
    gpio_put(LED_2,0);
    printf("USB initialized\n");

    // Initialize Queue for inter core comms
    queue_init(&key_state_q, sizeof(struct keyboard), 2);

    gpio_put(LED_3,1);
    multicore_launch_core1(core1_main);
    gpio_put(LED_3,0);


    while (1) {
        tud_task(); // tinyusb device task
        midi_task(adc_global);
        keyboard_task();
    }


    return 0;
}

void core1_main() {
    struct disp_t disp;
    printf("initializing display");
    init_disp(&disp, spi0, TFT_DC);
    printf("Entering main loop\n");
    // Buffer that will be written to the screen
    char print_buffer[128];

    double top_velocity = 0;

    uint8_t buffer[DISP_SIZE];
    int screen_size = 128*160;
    uint16_t *screen = malloc(screen_size * sizeof(uint16_t));
    if (screen == NULL) {
        printf("SCREEN BASED\n");
    }

    for(int i = 0; i < screen_size; i++) {
        screen[i] = 0xFFF;
    }

     screan_to_disp(screen, buffer, screen_size);

    disp_wr_cmd(&disp, DISP_RAMWR, buffer, DISP_SIZE);
    disp_wr_cmd(&disp, DISP_NOP, NULL, 0);
    

    
    while(1) {
        keyboard keystate;
        if(!queue_try_remove(&key_state_q, &keystate)) {
            continue; // we couldn't remove so we do nothing else
        }
        int active = -1;
        double vel;
        // NOTE: THIS DOESN'T GET THE MOST RECENTLY PRESSED BUT THE 
        // FIRST PRESSED KEY IN THE ARRAY! FIX
        for(unsigned int i = 0; i < KEY_COUNT && active == -1; i++) {
            if(keystate.keys[i].active) {
                active = i;
            }
        }

        
        key active_key  = keystate.keys[2];
        vel = key_get_velocity_cms(&active_key);
        
        sprintf(print_buffer, "Voltage = %f", 2.5*(active_key.start_pos/ (255.0)));
        draw_string(print_buffer, 25, 125, WHITE, BLACK);


        top_velocity = vel;
        sprintf(print_buffer, "Velocity: %.2f cm/s", vel);
        draw_string(print_buffer, 45, 125, WHITE, BLACK);

        sprintf(print_buffer, "pos: %d ", active_key.current_pos);
        draw_string(print_buffer, 55, 125, WHITE, BLACK);

        sprintf(print_buffer, "                   ");
        draw_string(print_buffer, 35, 125, WHITE, BLACK);
        sprintf(print_buffer, "delta t: %ld", to_ms_since_boot(active_key.end_time) - to_ms_since_boot(active_key.start_time));
        draw_string(print_buffer, 35, 125, WHITE, BLACK);
    }
    

}


int keyboard_task(){
    static int i = 0;

    uint8_t note = i + (keyboard_global->octave * 12);

    if (keyboard_global->keys[i].pressed == 1 && keyboard_global->keys[i].active == 0){ //Falling edge
        keyboard_global->keys[i].active = 1;
        uint8_t velocity = (127 - (keyboard_global->keys[i].current_pos*2));
        if(send_general_midi_message(NOTE_ON, keyboard_global->channel, note,velocity,0)){
            printf("MIDI NOTE ON FAIL\n");
            i++;
            if (i == KEY_COUNT){
                i = 0;
            }
            return 1;
        }
    } else if(keyboard_global->keys[i].active == 1 && keyboard_global->keys[i].pressed == 0){
        keyboard_global->keys[i].active = 0;

        // push new state of the global keyboard
        // note: we are not doing the blocking one because if the 
        // queue is full we can just skip this as it's not "critical"
        queue_try_add(&key_state_q, keyboard_global);

        if (send_general_midi_message(NOTE_OFF, keyboard_global->channel, note,0,0)){
            printf("MIDI NOTE OFF FAIL\n");
            i++;
            if (i == KEY_COUNT){
                i = 0;
            }
            return 1;
        }
    }
    
    i++;
    if (i == KEY_COUNT){
        i = 0;
    }
    return 0;
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+


void midi_task(struct adc_t *adc) {
    static uint32_t start_ms = 0;

    //uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
    // GET most recently read key 
    if(adc == NULL) return;


    // The MIDI interface always creates input and output port/jack descriptors
    // regardless of these being used or not. Therefore incoming traffic should be read
    // (possibly just discarded) to avoid the sender blocking in IO
    uint8_t packet[4];
    while ( tud_midi_available() ) tud_midi_packet_read(packet);

    // send note periodically
    if (board_millis() - start_ms < 286) return; // not enough time
    start_ms += 286;
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
}
