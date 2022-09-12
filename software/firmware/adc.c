#include "adc.h"

int init_adc(struct adc_t* adc, spi_inst_t *spi, uint16_t spi_cs) {
    // The adc will work in auto-mode 2 which is defined as auto
    // incrementing the channel from channel zero to the channel 
    // programmed in during the programming process.
    if(adc == NULL || spi == NULL) {
        return 1;
    }

    spi_init(spi0, 100000);
    spi_set_format(spi, 16, 1, 1, SPI_MSB_FIRST);
    adc->spi = spi;
    adc->spi_cs = spi_cs;
    adc->control_reg = 0x1000;  
    // writes adc->control_reg to spi bus and reads into channel values
    adc_write_read_blocking(adc);
    // We are entering auto-2 programming. 0x9 is the code for enter auto 
    // programming and we want to have '12' in bits 9 - 6
    adc->control_reg = 0x9000 | (12u << 6);
    adc_write_read_blocking(adc);
    // set control register to continue in auto mode-2
    adc->control_reg = 0x3000; 
    // timer must be allocated in heap so it lives beyond lifetime of init_adc
    repeating_timer_t *timer = malloc(sizeof(repeating_timer_t));
    if(add_repeating_timer_ms(-100, adc_write_callback, NULL, timer)) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    }

    return 0;
}

bool adc_write_callback(struct repeating_timer *t) {
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        spi_get_hw(spi0)->dr = 0x3000;
        return true;
}

int adc_write_read_blocking(struct adc_t *adc) {
    //while(spi_is_busy(adc->spi));
    gpio_put(adc->spi_cs, 0);
    spi_write16_blocking(adc->spi, &adc->control_reg, 1);
    gpio_put(adc->spi_cs, 1);
    sleep_ms(10);
    gpio_put(adc->spi_cs, 0);
    spi_read16_blocking(adc->spi, adc->control_reg, &adc->channel_val, 1);
    gpio_put(adc->spi_cs, 1);
}
