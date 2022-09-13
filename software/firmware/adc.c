#include "adc.h"
#include <stdio.h>

struct adc_t *init_adc(spi_inst_t *spi, uint16_t spi_cs) {
    // The adc will work in auto-mode 2 which is defined as auto
    // incrementing the channel from channel zero to the channel 
    // programmed in during the programming process.
    if(spi == NULL) {
        return NULL;
    }

    struct adc_t *adc = malloc(sizeof(struct adc_t));

    spi_init(spi0, 10000);
    spi_set_format(spi, 16, 0, 0, SPI_MSB_FIRST);
    adc->spi = spi;
    adc->spi_cs = spi_cs;
    adc->control_reg = 0x1000;  
    // operating in manual mode
    for (int i = 0; i < 10; i++){
        adc_write_read_blocking(adc);
        printf("0x%04x 0x%04x\n", adc->control_reg, adc->channel_val);
        sleep_ms(100);
    }
    // auto 1 programming register
    adc->control_reg = 0x8000;
    adc_write_read_blocking(adc);
    printf("0x%04x 0x%04x\n", adc->control_reg, adc->channel_val);
    sleep_ms(50);
    adc->control_reg = 0x0FFF;
    adc_write_read_blocking(adc);
    printf("0x%04x 0x%04x\n", adc->control_reg, adc->channel_val);
    sleep_ms(50);
    // We are entering auto-2 programming. 0x9 is the code for enter auto 
    // programming and we want to have '12' in bits 9 - 6
    // set control register to continue in auto mode-2
    adc->control_reg = 0x2000;

    // Configure timer for SPI writes
    // timer must be allocated in heap so it lives beyond lifetime of init_adc
    repeating_timer_t *timer = malloc(sizeof(repeating_timer_t));
    if(add_repeating_timer_ms(-100, adc_write_callback, NULL, timer)) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    }

    // configure interrupt for spi reads
    spi_get_hw(spi0)->imsc = 1 << 1;
    irq_add_shared_handler(SPI0_IRQ, adc_read_irq, 1);
    irq_set_enabled(SPI0_IRQ, true);

    return adc;
}

bool adc_write_callback(struct repeating_timer *t) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        spi_get_hw(spi0)->dr = adc_global->control_reg;
        return true;
}

void adc_read_irq(void) {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    printf("0x%04x 0x%04x\n", adc_global->control_reg, adc_global->channel_val);
    adc_global->channel_val = spi_get_hw(spi0)->dr;
    adc_global->control_reg = 0x1000 | (((adc_global->channel_val >> 12) + 1) % 12u) << 7;
    spi_get_hw(spi0)->icr = 0;
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
