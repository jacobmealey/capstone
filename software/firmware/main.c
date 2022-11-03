// File: main.c
// Date: 10/25/22
// Authors: Jacob Mealey & Landyn Francis
// Purpose: Main loop :)

#include <stdlib.h>
#include <string.h>

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

#define DISP_SIZE 30720
int keyboard_task();

void midi_task(struct adc_t *adc);
void core1_main();

struct adc_t *adc_global;
struct keyboard *keyboard_global;
struct disp_t *disp_global;

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

    gpio_put(LED_3,1);
    multicore_launch_core1(core1_main);
    gpio_put(LED_3,0);

    gpio_put(LED_0,1);
    gpio_put(LED_1,1);
    gpio_put(LED_2,1);
    gpio_put(LED_3,1);
    sleep_ms(500);
    gpio_put(LED_1,0);
    gpio_put(LED_2,0);
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

    draw_rect(10, 10, 30, 54, ORANGE);
    draw_rect(100, 56, 15, 12, PURPLE);

    while(1) {
        // draw_font_test();
        draw_string("Velocity: 2.5cm/s", 45, 125, PINK, BLUE);
    }

}


int keyboard_task(){
    static int i = 0;
    key current_key = keyboard_global->keys[i];

    uint8_t note = i + (keyboard_global->octave * 12);

    if (current_key.pressed == 1 && current_key.active == 0){ //Falling edge
        current_key.active = 0;
        uint8_t velocity = (127 - current_key.current_pos);
        if(send_general_midi_message(NOTE_ON, keyboard_global->channel, note,velocity,0)){
            printf("MIDI NOTE ON FAIL\n");
            i++;
            if (i == KEY_COUNT){
                i = 0;
            }
            return 1;
        }
    } else if(current_key.active == 1 && current_key.pressed == 0){
        current_key.active = 0;
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
