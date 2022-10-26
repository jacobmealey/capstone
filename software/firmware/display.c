// display.c 
// Authors: Jacob Mealey <jacob.mealey@maine.edu>
//          Landyn Francis <landyn.francis@maine.edu>
//

#include "display.h"
#include "pins.h"
#include "font.h"

#include <string.h>


// disp - a higher variable passed in from the calling funciton. This is different
//        from how we did the ADC because this *shouldn't* need any heap allocation
//
int init_disp(struct disp_t *disp, spi_inst_t *spi, uint16_t disp_dc) {
    if(disp == NULL || spi == NULL) {
        printf("Error bad values passes to init_disp\n");
        return 1;
    }
    disp->spi = spi;
    disp->dc = disp_dc;
    disp->cs = SPI0_CS;
    disp_global = disp; 

    spi_init(disp->spi, 1000 * 10000);
    spi_set_format(disp->spi, 8, 0, 0, SPI_MSB_FIRST);
 
    
    gpio_init(disp->dc);
    gpio_set_dir(disp->dc, GPIO_OUT);
    
    gpio_init(disp->cs);
    gpio_set_dir(disp->cs, GPIO_OUT);

    uint8_t command_buffer[16];

    disp_wr_cmd(disp_global, DISP_SWRST, NULL, 0);
    sleep_ms(50);
    disp_wr_cmd(disp_global, DISP_SLPOUT, NULL, 0);
    sleep_ms(255);
    command_buffer[0] = DISP_COL_4K;
    disp_wr_cmd(disp_global, DISP_COLMOD, command_buffer, 1);
    sleep_ms(10);
    command_buffer[0] = 0x01;
    command_buffer[1] = 0x2C;
    command_buffer[2] = 0x2D;
    disp_wr_cmd(disp_global, DISP_FRMCTR1, command_buffer, 3);
    disp_wr_cmd(disp_global, DISP_FRMCTR2, command_buffer, 3);
    command_buffer[0] = 0x01;
    command_buffer[1] = 0x2C;
    command_buffer[2] = 0x2D;
    command_buffer[3] = 0x01;
    command_buffer[4] = 0x2C;
    command_buffer[5] = 0x2D;
    disp_wr_cmd(disp_global, DISP_FRMCTR3, command_buffer, 6);
    command_buffer[0] = 0x07;
    disp_wr_cmd(disp_global, DISP_INVCTR, command_buffer, 6);
    command_buffer[0] = 0xA2;
    command_buffer[1] = 0x02;
    command_buffer[2] = 0x84;
    disp_wr_cmd(disp_global, DISP_PWRCTR1, command_buffer, 3);
    command_buffer[0] = 0xC5;
    disp_wr_cmd(disp_global, DISP_PWRCTR2, command_buffer, 1);
    command_buffer[0] = 0x0A;
    command_buffer[1] = 0x00;
    disp_wr_cmd(disp_global, DISP_PWRCTR3, command_buffer, 2);
    command_buffer[0] = 0x8A;
    command_buffer[1] = 0x2A;
    disp_wr_cmd(disp_global, DISP_PWRCTR4, command_buffer, 2);
    command_buffer[0] = 0x8A;
    command_buffer[1] = 0xEE;
    disp_wr_cmd(disp_global, DISP_PWRCTR5, command_buffer, 2);
    command_buffer[0] = 0x0E;
    disp_wr_cmd(disp_global, DISP_VMCTR1, command_buffer, 1);
    disp_wr_cmd(disp_global, DISP_INVOFF, NULL, 0);
    command_buffer[0] = 0xC8;
    disp_wr_cmd(disp_global, DISP_MADCTL, command_buffer, 1);
    command_buffer[0] = DISP_COL_4K;
    disp_wr_cmd(disp_global, DISP_COLMOD, command_buffer, 1);
    command_buffer[0] = 0x00;
    command_buffer[1] = 0x00;
    command_buffer[2] = 0x00;
    command_buffer[3] = 0x7F;
    disp_wr_cmd(disp_global, DISP_CASET, command_buffer, 4);
    command_buffer[0] = 0x00;
    command_buffer[1] = 0x00;
    command_buffer[2] = 0x00;
    command_buffer[3] = 0x9F;
    disp_wr_cmd(disp_global, DISP_RASET, command_buffer, 4);
    disp_wr_cmd(disp_global, DISP_NORON, NULL, 0);
    sleep_ms(10);
    disp_wr_cmd(disp_global, DISP_DISPON, NULL, 0);
    sleep_ms(100);

    return 0;
}



int disp_wr_cmd(struct disp_t *disp, uint8_t command, uint8_t *args, unsigned int len) {
    gpio_put(disp->cs, 0);
    gpio_put(disp->dc, 0);
    spi_write_blocking(disp->spi, &command, 1);
    gpio_put(disp->dc, 1);
    if(len != 0) {
        spi_write_blocking(disp->spi, args, len);
    }
    gpio_put(disp->cs, 1);

    return 0;
}


// screen is the "easy" to interract with form of display data, each element
// in screen corresponds to 1 pixel, formattted like: 0x0RGB.
// disp is the formmatted version of screen to be written to the SPI bus
int screan_to_disp(uint16_t *screen, uint8_t *disp, int screen_len) {
    uint8_t acc = 0; // accumulator for screen translation
    int acc_set = 0; // variable to check accumlator
    int i = 0; // the current pixel in the screen array
    int bi = 0; // the current byte being set in disp

    while(i < screen_len) {
        if(acc_set  == 0) { // if acc is empty
            disp[bi] = (screen[i] >> 4) & 0xFF;
            acc = screen[i] & 0xF;
            acc_set = 1;
            bi++;
            i++;
        } 
        if(acc_set == 2) { // if the top half of acc is set 
           disp[bi] = acc;
           acc_set = 0;
           bi++;
        }
        if(acc_set == 1){ // final case - acc lowest half is set
            // write acc to the highest part of disp
            // and write the highest part of screen 
            disp[bi] = ((acc << 4) & 0xF0) | ((screen[i] >> 8) & 0x0F);
            // save the lower 2/3 of screen to acc and set where to next
            acc = screen[i] & 0xFF; 
            acc_set = 2;
            bi++;
            i++;
        }
    }
    return bi;
}

void set_x(uint8_t x) {
    if(x > DISPLAY_W) x = DISPLAY_W;
    uint8_t command_buffer[4];
    command_buffer[0] = 0x00;
    command_buffer[1] = x;
    command_buffer[2] = 0x00;
    command_buffer[3] = 0x7F;
    disp_wr_cmd(disp_global, DISP_CASET, command_buffer, 4);
}

void set_y(uint8_t y) {
    if(y > DISPLAY_HEIGHT) y = DISPLAY_HEIGHT;
    uint8_t command_buffer[4];
    command_buffer[0] = 0x00;
    command_buffer[1] = y;
    command_buffer[2] = 0x00;
    command_buffer[3] = 0x9F;
    disp_wr_cmd(disp_global, DISP_RASET, command_buffer, 4);
}


// draws a rectangle at location x,y with height h amd width w
// it fills it with color.
int draw_rect(uint8_t x, uint8_t y, uint8_t h, uint8_t w, uint16_t color){
   // static because they shouldn't be allocated everytime?
   static uint16_t screen[128];
   static uint8_t buffer[200];
   
   //screen is a single "row" of the rect, so only fill to w
   for(int i = 0; i < w + 1; i++) {
       screen[i] = color;
   }

   // convert screen to w
   screan_to_disp(screen, buffer, w);
   // go the the x position
   set_x(x);
  
   if(!(w % 3)) w += w%3;
   // loop through h values, draw a new line of the screen
   // at every incrementing h
   for(int i = 0; i < h; i++) {
       set_y(y+i);
       disp_wr_cmd(disp_global, DISP_RAMWR, buffer, (w*3) / 2);
   }

   // reset x and y to zero;
   set_x(0);
   set_y(0);
   
   return 0;

}


// Draw a string of characters to the display and location x,y 
// str - a null terminatd string of characters. 
void draw_string(const char *str, uint8_t x, uint8_t y, uint16_t font_bg, uint16_t font_fg){
    set_x(x);
    set_y(y);
    // loop until the the null character is met
    while(*str != '\0' || y < 5 || x < 5 || x > DISPLAY_W || y > DISPLAY_HEIGHT) {
        draw_char(*str, x, y, font_bg, font_fg);
        y -= 6;
        str++;
    }
}


// Draw a single character to the display
// c - character to draw
// x - x location to draw display
// y - y location to draw the display
void draw_char(char c, uint8_t x, uint8_t y, uint16_t font_bg, uint16_t font_fg) {
    static uint8_t character[5];
    static uint16_t screen[7];
    static uint8_t buffer[16];

    memcpy(character, font + 5*c, 5); 
    if(y == 0) y = 6;
    if(x == 0) x = 1;


    draw_rect(x-7, y-5, 7, 9, font_bg);
    set_x(x - 7);
    for(int i = 0; i < 5; i++){ //loop through lines
        for(int j = 0; j < 7; j++){ //loop through pixels of current line
            screen[j] = ((character[i] >> j) & 1u) ? font_fg: font_bg;
        }
        set_y(y - i); // move to to next line
        // write current line to the display
        screan_to_disp(screen, buffer, 7); 
        disp_wr_cmd(disp_global, DISP_RAMWR, buffer, 11);
    }

}

// this a test to draw the font - it uses some static variables 
// so to use draw_font_test just run it in main loop with nothing else
void draw_font_test() {
    static int offset = 0;
    static int x;
    static int y;

    uint8_t character[5];
    uint16_t screen[7];
    uint8_t buffer[16];

    if(offset > 3125) return;

    // get current character
    memcpy(character, font + offset, 5); 
    if(y == 0) y = 6;
    if(x == 0) x = 1;

    for(int i = 0; i < 5; i++){ //loop through lines
        for(int j = 0; j < 7; j++){ //loop through pixels
            screen[j] = ((character[i] >> j) & 1u) ? 0x000: 0xFFF;
        }
        set_y(y - i);
        screan_to_disp(screen, buffer, 7);
        disp_wr_cmd(disp_global, DISP_RAMWR, buffer, 11);
    }

    // update x and ys accordingly
    x += 8;
    offset += 5;

    if(x > DISPLAY_W - 7) {
        x = 1;
        y += 6;
    }
    set_x(x);
}
