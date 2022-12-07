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
#include "midi_velocity_lut.h"

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#define DISP_SIZE 30720

int keyboard_task();
void midi_task(struct adc_t *adc);
void core1_main();

// Global ADC Structure
struct adc_t *adc_global;

// Global Keyboard Structure
struct keyboard *keyboard_global;

// Global Display Structure
struct disp_t *disp_global;

// A queue for sending the current state of the keyboard
// to the second core 
queue_t key_state_q;

/*------------- MAIN -------------*/
int main(void) {
 
    // Initialize UART with 9600 baud rate
    stdio_init_all();
    uart_init(uart0, 9600);
    
    // Initialize GPIO pins
    pin_init();

    // Initialize ADC on SPI_1
    gpio_put(LED_0,1);
    adc_global = init_adc(spi1, SPI1_CS);
    gpio_put(LED_0,0);
    printf("ADC initialized\n");


    // Initialize Keyboard on SPI_0
    gpio_put(LED_1,1);
    keyboard_global = init_keys();
    gpio_put(LED_1,0);
    printf("Keyboard Initialized"); 

    // Initialize USB using TinyUSB
    gpio_put(LED_2,1);
    tusb_init();
    gpio_put(LED_2,0);
    printf("USB initialized\n");

    // Initialize Queue for inter core comms
    queue_init(&key_state_q, sizeof(struct keyboard), 2);

    // Launch display code on second core
    gpio_put(LED_3,1);
    multicore_launch_core1(core1_main);
    gpio_put(LED_3,0);


    while (1) {
        // TinyUSB Device task (setup USB configuration)
        tud_task();

        // MIDI Task for preparing MIDI transactions
        midi_task(adc_global);

        // Read keyboard
        keyboard_task();
    }


    return 0;
}
// Code to run on Second Core
// The second core runs the display, and handles any heavy compuations, such as velocity.
void core1_main() {
    struct disp_t disp;
    printf("initializing display");
    init_disp(&disp, spi0, TFT_DC);
    printf("Entering main loop\n");

    // Buffer that will be written to the screen
    char print_buffer[128];

    uint8_t buffer[DISP_SIZE];
    int screen_size = 128*160;
    uint16_t *screen = malloc(screen_size * sizeof(uint16_t));
    if (screen == NULL) {
        printf("SCREEN BASED\n");
    }

    for(int i = 0; i < screen_size; i++) {
        screen[i] = 0xFFF;
    }

    screen_to_disp(screen, buffer, screen_size);
    disp_wr_cmd(&disp, DISP_RAMWR, buffer, DISP_SIZE);
    disp_wr_cmd(&disp, DISP_NOP, NULL, 0);
    

    sprintf(print_buffer, "MIDI Keyboard");
    draw_string(print_buffer, 55, 125, WHITE, BLACK);
    
    while(1) {
        keyboard keystate;
        if(!queue_try_remove(&key_state_q, &keystate)) {
            continue; // we couldn't remove so we do nothing else
        }
        double vel;
        

        // Get most recently pressed key
        key active_key  = keystate.keys[keystate.last_pressed];
        // Determine velocity
        vel = key_get_velocity_cms(&active_key);
        
        // Print first voltage level read on downward press
        sprintf(print_buffer, "Voltage = %f", 2.5*(active_key.start_pos/ (255.0)));
        draw_string(print_buffer, 25, 125, WHITE, BLACK);

        // Print calculated velocity
        sprintf(print_buffer, "Velocity: %.2f cm/s", vel);
        draw_string(print_buffer, 45, 125, WHITE, BLACK);

        // Print ADC value of current key
        sprintf(print_buffer, "pos: %d ", active_key.current_pos);
        draw_string(print_buffer, 55, 125, WHITE, BLACK);

        // Clear out old data from screen
        sprintf(print_buffer, "                   ");
        draw_string(print_buffer, 35, 125, WHITE, BLACK);

        // Determine time difference between start of keypress and end of keypress
        float deltaT = to_us_since_boot(active_key.end_time) - to_us_since_boot(active_key.start_time);

        // Print delta T
        sprintf(print_buffer, "delta t (ms): %.2f", (deltaT/1000.0));
        draw_string(print_buffer, 35, 125, WHITE, BLACK);

        // Print which key number was pressed
        sprintf(print_buffer, "Last Pressed: %d", keystate.last_pressed);
        draw_string(print_buffer, 15, 125, WHITE, BLACK);

    }
    

}

// Query each key and determine if they are pressed
// If a key is pressed do the following:
//      1. Send a MIDI message with the appropraite note and MIDI velocity
//      2. Gather press data, and transfer to second core to display.
int keyboard_task(){
    static int i = 0;

    // Calculate note based on current octave and key pressed
    uint8_t note = i + (keyboard_global->octave * 12);

    if (keyboard_global->keys[i].current_pos < 75 && keyboard_global->keys[i].midi_active == 0){
        keyboard_global->keys[i].midi_active = 1;
        keyboard_global->keys[i].midi_start = get_absolute_time();
        printf("First threshold!\n");
    }

    if (keyboard_global->keys[i].current_pos > 80 && keyboard_global->keys[i].midi_active == 1){
        keyboard_global->keys[i].midi_active = 0;
    }

    // If the key has been pressed, and was previously not pressed (falling edge)
    if (keyboard_global->keys[i].pressed == 1 && keyboard_global->keys[i].active == 0){ 
        // Key is now active
        keyboard_global->keys[i].active = 1;

        keyboard_global->keys[i].midi_end = get_absolute_time();

        uint16_t input_start = 1;
        uint16_t input_end = 127;
        uint16_t output_start = 127;
        uint16_t output_end = 1;

        double slope = 1.0 * (output_end - output_start) / (input_end - input_start);


        

        int delta_t = to_ms_since_boot(keyboard_global->keys[i].midi_end) - to_ms_since_boot(keyboard_global->keys[i].midi_start);
        
        if (delta_t > 127){
            delta_t = 127;
        }
        if (delta_t < 1){
            delta_t = 1;
        }
        
        
        uint8_t velocity = 127 - delta_t;
        //uint8_t velocity = output_start + slope * ((delta_t) - input_start);
        printf("MIDI Delta T (%d): %d, velocty = %d\n",i,delta_t, velocity);

        // Send NOTE ON MIDI Message
        if(send_general_midi_message(NOTE_ON, keyboard_global->channel, note,velocity,0)){
            printf("MIDI NOTE ON FAIL\n");
            // Continue to iterate through keys
            i++;
            if (i == KEY_COUNT){
                i = 0;
            }
            return 1;
        }
    // If Key is active, but no longer pressed (Rising edge)
    } else if(keyboard_global->keys[i].active == 1 && keyboard_global->keys[i].pressed == 0){
        // Key is no longer active
        keyboard_global->keys[i].active = 0;

        // Update most recently pressed key
        keyboard_global->last_pressed = i;

        // push new state of the global keyboard
        // note: we are not doing the blocking one because if the 
        // queue is full we can just skip this as it's not "critical"
        queue_try_add(&key_state_q, keyboard_global);

        // Send NOTE OFF MIDI Message
        if (send_general_midi_message(NOTE_OFF, keyboard_global->channel, note,0,0)){
            printf("MIDI NOTE OFF FAIL\n");
            // Continue to iterate through keys
            i++;
            if (i == KEY_COUNT){
                i = 0;
            }
            return 1;
        }
    }
    
    // Continue to iterate through keys
    i++;
    if (i == KEY_COUNT){
        i = 0;
    }
    return 0;
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

// MIDI Task to read incoming MIDI messages from host device. Must do this
// to be able to write messages. 
void midi_task(struct adc_t *adc) {
    static uint32_t start_ms = 0;

    //uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
    if(adc == NULL) return;


    // The MIDI interface always creates input and output port/jack descriptors
    // regardless of these being used or not. Therefore incoming traffic should be read
    // (possibly just discarded) to avoid the sender blocking in IO
    uint8_t packet[4];
    while ( tud_midi_available() ) tud_midi_packet_read(packet);

    // Wait appropriate time between sending MIDI packages
    if (board_millis() - start_ms < 286) return;
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
