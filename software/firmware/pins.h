//File: pins.h
//Date Created: 9/16/22
//Authors: Landyn Francis (landyn.francis@maine.edu) Jacob Mealey (jacob.mealey@maine.edu)
//Purpose: Header file containing GPIO Pin definitions for MIDI Keyboard device

#ifndef PINS_H
#define PINS_H

//SPI0 is the Display
#define SPI0_RX         0
#define SPI0_CS         1
#define SPI0_SCLK       2
#define SPI0_TX         3

//Four Status/Debugging LED's
#define LED_0           4
#define LED_1           5
#define LED_2           6
#define LED_3           7

//SPI1 is the ADC
#define SPI1_RX         11
#define SPI1_CS         12
#define SPI1_SCLK       13
#define SPI1_TX         14

//UART for Printing Debug Messages
#define UART0_TX        16
#define UART0_RX        17

//I2C1
#define I2C1_SDA        18
#define I2C1_SCL        19

//I2C0
#define I2C0_SDA        20
#define I2C0_SCL        21

//User Control Buttons
#define RESET           22
#define OCT_UP          23
#define OCT_DOWN        24

//Volume Dial (Rotary Encoder)
#define ENCODE_PRESS    26
#define ENCODE_B        27
#define ENCODE_A        28




#endif