#ifndef SPI_ADC_H
#define SPI_ADC_H

#include <stdlib.h>
#include <stdint.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"

// ADC Modes
#define ADC_MODE_RESET 0x0001
#define ADC_MODE_AUTO1 0x0002

// ADC Auto Mode 1 Control Flags
#define M1_FLAG_EN_PROGRAMMING  0x0800
#define M1_FLAG_RESET_COUNTER   0x0400
#define M1_FLAG_2X_RANGE        0x0040
#define M1_FLAG_DEV_POW_DOWN    0x0020
#define M1_FLAG_EN_GPIO_MAP     0x0010

// Masks and shifts for reading DATA
#define ADC_8BIT_VAL(d) ((d >> 4) & 0xFF)   
#define ADC_PRC_CHAN(d) (d >> 12)

// For entering programming ADC auto mode 1
#define ADC_PROG_AUTO1 0x8000
// Enabling all channels for the 12 bit ADC
#define ADC_PROG_ENALL 0x0FFF

struct adc_t {
    spi_inst_t *spi;
    uint16_t control_reg;
    uint16_t prev_chanel;
    uint16_t channel_val;
    uint16_t spi_cs;
};

int init_adc(struct adc_t* adc, spi_inst_t *spi, uint16_t spi_cs);
int adc_write_read_blocking(struct adc_t *adc);
bool adc_write_callback(repeating_timer_t *t);

#endif
