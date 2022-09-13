#ifndef SPI_ADC_H
#define SPI_ADC_H

#include <stdlib.h>
#include <stdint.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

// ADC Modes
#define ADC_MODE_RESET 0x0001
#define ADC_MODE_AUTO1 (0x0002 << 12) | 1u << 10
#define ADC_MODE_AUTO2 0x3000 

// ADC Auto Mode 1 Control Flags
#define M1_FLAG_EN_PROGRAMMING  0x0800
#define M1_FLAG_RESET_COUNTER   0x0400
#define M1_FLAG_2X_RANGE        0x0040
#define M1_FLAG_DEV_POW_DOWN    0x0020
#define M1_FLAG_EN_GPIO_MAP     0x0010

// Masks and shifts for reading DATA
#define ADC_8BIT_VAL(d) ((d >> 4) & 0xFF)   
#define ADC_PRV_CHAN(d) (d >> 12)

// For entering programming ADC auto mode 1
#define ADC_AUTO1_PROG 0x8000 
// ADC_AUTO2_PROG takes one parameter an 8 bit integer to 
// specify the max channel to read to starting from zero
#define ADC_AUTO2_PROG(a) (0x9000 | ((a & 0xF) << 6u))
#define ADC_ALARM_PROG ADC_MODE_RESET << 12
#define ADC_GPIO_PROG  ADC_MODE_RESET << 12
// Enabling all channels for the 12 bit ADC
#define ADC_PROG_ENALL 0x0FFF

struct adc_t {
    spi_inst_t *spi;
    uint16_t control_reg;
    uint16_t prev_chanel;
    uint16_t channel_val;
    uint16_t spi_cs;
};

extern struct adc_t *adc_global;

// init adc takes the spi and the CS lines, it allocates 
// and adc_t on the heap which is returned at the end of 
// the function. it also allocats a repeating_timer_t 
struct adc_t *init_adc(spi_inst_t *spi, uint16_t spi_cs);

// thing wrapper around spi_write_read16_blocking
int adc_write_read_blocking(struct adc_t *adc);

// the callback function whenever the timer allocated in 
// init elapses. TODO: add someway to overright this? 
// perhaps using __weak__?
bool adc_write_callback(repeating_timer_t *t);
// the function which is called when the SPI bus recieves
// data from the ADC TODO add somway to overwrite this.
void adc_read_irq(void);

#endif
