/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
//#include <stdio.h>
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

#define DISP_SIZE 30720

void midi_task(struct adc_t *adc);

struct adc_t *adc_global;
struct keyboard *keyboard_global;
struct disp_t *disp_global;

/*------------- MAIN -------------*/
int main(void) {
 
    stdio_init_all();
    uart_init(uart0, 9600);
    struct disp_t disp;
    
    //Initialize GPIO pins
    pin_init();

    //Initialize ADC
    adc_global = init_adc(spi1, SPI1_CS);
    printf("ADC initialized\n");

    //Initialize Keyboard
    keyboard_global = init_keys();   


    //Initialize USB
    //tusb_init();
    printf("USB initialized\n");

    printf("initializing display");
    init_disp(&disp, spi0, TFT_DC);
    printf("Entering main loop\n");

    uint8_t buffer[DISP_SIZE];
    int screen_size = 128*160;
    uint16_t *screen = malloc(screen_size * sizeof(uint16_t));
    if (screen == NULL) {
        printf("SCREEN BASED\n");
    }

    uint16_t colors[5] = {0x0FF, 0x0F0, 0x00F, 0xF0F, 0xFF0};
    for(int i = 0; i < screen_size; i++) {
        screen[i] = 0xFFF;
    }

    screan_to_disp(screen, buffer, screen_size);

    disp_wr_cmd(&disp, DISP_RAMWR, buffer, DISP_SIZE);
    disp_wr_cmd(&disp, DISP_NOP, NULL, 0);
    int i = 1; 
    int k = 0;
    while (1) {
        // draw_font_test();
        draw_string("Velocity: 2.5cm/s", 45, 125);
        //tud_task(); // tinyusb device tast
        //midi_task(adc_global);
    }


    return 0;
}


//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

// Variable that holds the current position in the sequence.
uint32_t note_pos = 0;

// Store example melody as an array of note values
uint8_t note_sequence[] = {
    74,78,81,86,90,93,98,102,57,61,66,69,73,78,81,85,88,92,97,100,97,92,88,85,81,78,
    74,69,66,62,57,62,66,69,74,78,81,86,90,93,97,102,97,93,90,85,81,78,73,68,64,61,
    56,61,64,68,74,78,81,86,90,93,98,102
};

void midi_task(struct adc_t *adc) {
    static uint32_t start_ms = 0;

    //uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
    uint8_t const channel   = 0; // 0 for channel 1
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

    // Previous positions in the note sequence.
    //int previous = note_pos - 1;

    // If we currently are at position 0, set the
    // previous position to the last note in the sequence.
    //if (previous < 0) previous = sizeof(note_sequence) - 1;

    //Makes use of midi.c functiosn to package and send MIDI messages in one line
    //if (send_general_midi_message(NOTE_ON,channel,note_sequence[note_pos],adc->channel_val >> 4 ,0)){
    //    //printf("MIDI NOTE ON SEND ERROR\n");
    //}
    //Corresponding NOTE OFF message
    //if (send_general_midi_message(NOTE_OFF,channel, note_sequence[previous],0,0)){
        //printf("MIDI NOTE OFF SEND ERROR\n");
    //}

   

    // Increment position
    //note_pos++;

    // If we are at the end of the sequence, start over.
    //if (note_pos >= sizeof(note_sequence)) note_pos = 0;
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
