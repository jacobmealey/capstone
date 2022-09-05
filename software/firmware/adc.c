#include "adc.h"

int init_adc(struct adc_t* adc, spi_inst_t *spi, uint16_t spi_cs) {
    if(adc == NULL || spi == NULL) {
        return 1;
    }

    adc->spi = spi;
    adc->spi_cs = spi_cs;
    adc->control_reg = 0x1000;  
    // writes adc->control_reg to spi bus and reads into channel values
    adc_write_read_blocking(adc);

    return 0;
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
