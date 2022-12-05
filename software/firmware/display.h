#ifndef DISPLAY_H
#define DISPLAY_H
// display.c 
// Authors: Jacob Mealey <jacob.mealey@maine.edu>
//          Landyn Francis <landyn.francis@maine.edu>
// The Display uses SPI, it can operate in many different 
// colors modes but we will be operating in 4k color 
// because we don't need that much stuff. 
//
// The display we are using is a 1.8 inch TFT display, 
// the driver chip is the ST7735R, and we are using a 
// breakout board from Adafruit. Much of this code is based 
// off elements described in the data shhet and the work 
// done for the ST7735R Arduino Library written by Adafruit,
// 

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#define DISPLAY_W 128
#define DISPLAY_HEIGHT 160


// List of commands from pg. 77 of ST7735 Datasheet
#define DISP_NOP 0x00
#define DISP_SWRST 0x01
#define DISP_RDID 0x04
#define DISP_RDDST 0x09
#define DISP_RDDPM 0x0A
#define DISP_RDD_MACDCTL 0x0B
#define DISP_RDD_COLMOD 0x0C
#define DISP_RDDIM 0x0D
#define DISP_RDDSM 0x0E
#define DISP_SLPIN 0x10
#define DISP_SLPOUT 0x11
#define DISP_PTLON 0x12
#define DISP_NORON 0x13
#define DISP_INVOFF 0x20
#define DISP_ONVON 0x021
#define DISP_GAMSET 0x26
#define DISP_DISPOFF 0x28
#define DISP_DISPON 0x29
#define DISP_CASET 0x2A
#define DISP_RASET 0x2B
#define DISP_RAMWR 0x2C
#define DISP_RAMRD 0x2E
#define DISP_PLAR 0x30
#define DISP_TEOFF 0x34
#define DISP_TEON 0x35
#define DISP_MADCTL 0x36
#define DISP_IDMOFF 0x38
#define DISP_ODMON 0x39
#define DISP_COLMOD 0x3A
#define DISP_RDID1 0xDA
#define DISP_RDID2 0xDB
#define DISP_RDID3 0xDC

#define DISP_FRMCTR1 0xB1
#define DISP_FRMCTR2 0xB2
#define DISP_FRMCTR3 0xB3
#define DISP_INVCTR 0xB4
#define DISP_DISSET5 0xB6
#define DISP_PWRCTR1 0xC0
#define DISP_PWRCTR2 0xC1
#define DISP_PWRCTR3 0xC2
#define DISP_PWRCTR4 0xC3
#define DISP_PWRCTR5 0xC4
#define DISP_VMCTR1 0xC5
#define DISP_VMOFCTR 0xC7
#define DISP_WRID2 0xD1
#define DISP_WRID3 0xD2
#define DISP_PWCTR6 0xFC
#define DISP_NVCTR1 0xD9
#define DISP_NVCTR2 0xDE
#define DISP_NVCTR3 0xDF
#define DISP_GAMCTRP1 0xE0
#define DISP_GAMCTRN1 0xE1
#define DISP_EXTCTRL 0xF0
#define DISP_VCOM4L 0xFF

// Extra macros like colors and such
#define DISP_COL_4K 0x03
#define DISP_COL_65K 0x05
#define DISP_COLOR_262K 0x06

struct disp_t {
    spi_inst_t *spi;
    uint16_t dc;
    uint16_t cs;
};

enum display_colors {
    RED = 0xF00,
    GREEN = 0x0F0,
    BLUE = 0x00F,
    PURPLE = 0xF0F,
    YELLOW = 0xFF0,
    ORANGE = 0xF70,
    PINK = 0xF88,
    BLACK = 0x000,
    WHITE = 0xFFF,
};

extern struct disp_t *disp_global;

int init_disp(struct disp_t *disp, spi_inst_t *spi, uint16_t disp_dc);
int disp_wr_cmd(struct disp_t *disp, uint8_t command, uint8_t *args, unsigned int len);

int screen_to_disp(uint16_t *screen, uint8_t *disp, int screen_len);

void set_x(uint8_t x);
void set_y(uint8_t y);

int draw_rect(uint8_t x, uint8_t y, uint8_t h, uint8_t w, uint16_t color);

void draw_font_test();
void draw_char(char c, uint8_t x, uint8_t y, uint16_t font_bg, uint16_t font_fg);
void draw_string(const char *c, uint8_t x, uint8_t y, uint16_t font_bg, uint16_t font_fg);
#endif




