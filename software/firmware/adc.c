#include "adc.h"
#include <stdio.h>
#include "pins.h"
#include "midi.h"
#include "keys.h"

struct adc_t *init_adc(spi_inst_t *spi, uint16_t spi_cs) {
    // The adc will work in auto-mode 2 which is defined as auto
    // incrementing the channel from channel zero to the channel 
    // programmed in during the programming process.
    if(spi == NULL) {
        return NULL;
    }

    struct adc_t *adc = malloc(sizeof(struct adc_t));

    adc->spi = spi;
    spi_init(adc->spi, 200000);
    spi_set_format(adc->spi, 16, 0, 0, SPI_MSB_FIRST);
    adc->spi_cs = spi_cs;
    adc->control_reg = ADC_MODE_RESET;  
    // operating in manual mode
    for (int i = 0; i < 6; i++){
        adc_write_read_blocking(adc);
        printf("0x%04x 0x%04x\n", adc->control_reg, adc->channel_val);
    }
    // auto 2 programming register
    // We are entering auto-2 programming. 12 is the amount of channels 
    // we want to go up to. 
    adc->control_reg = ADC_AUTO2_PROG(12u);
    adc_write_read_blocking(adc);
    printf("0x%04x 0x%04x\n", adc->control_reg, adc->channel_val);
    // set control register to continue in auto mode-2
    adc->control_reg = ADC_MODE_AUTO2;

    // Configure timer for SPI writes
    // timer must be allocated in heap so it lives beyond lifetime of init_adc
    repeating_timer_t *timer = malloc(sizeof(repeating_timer_t));
    if(add_repeating_timer_ms(-5, adc_write_callback, NULL, timer)) {
        gpio_put(LED_0, 1);
    }

    // configure interrupt for spi reads
    spi_get_hw(adc->spi)->imsc = 1 << 1;
    irq_add_shared_handler(SPI1_IRQ, adc_read_irq, 1);
    irq_set_enabled(SPI1_IRQ, true);

    return adc;
}

bool adc_write_callback(struct repeating_timer *t) {
        (void)(t);
        spi_get_hw(adc_global->spi)->dr = adc_global->control_reg;
        return true;
}

// Callback function called after each ADC read
void adc_read_irq(void) {
    adc_global->channel_val = spi_get_hw(adc_global->spi)->dr; //Set channel value

    adc_global->prev_chanel = ADC_PRV_CHAN(adc_global->channel_val);
    uint8_t current_value = ADC_8BIT_VAL(adc_global->channel_val);

    //printf("0x%04x Channel:%d\tValue:%d\n", adc_global->control_reg, adc_global->prev_chanel, current_value); //Print value (clean)
    //printf("0x%04x 0x%04x\n", adc_global->control_reg, adc_global->channel_val); //Print Value (Raw)
    
    keyboard_global->keys[adc_global->prev_chanel].current_pos = current_value; //Update keys struct

    if (current_value < KEY_THRESH && 
            keyboard_global->keys[adc_global->prev_chanel].pressed == 0 &&
            keyboard_global->keys[adc_global->prev_chanel].prev_pos > keyboard_global->keys[adc_global->prev_chanel].current_pos)
    {
        printf("KEY PRESSED\n");
        keyboard_global->keys[adc_global->prev_chanel].pressed = 1;
        
    }
    
    if (keyboard_global->keys[adc_global->prev_chanel].pressed == 1 && current_value > KEY_THRESH){
        printf("KEY RELEASED\n");
        keyboard_global->keys[adc_global->prev_chanel].pressed = 0;
    }

    keyboard_global->keys[ADC_PRV_CHAN(adc_global->channel_val)].prev_pos = keyboard_global->keys[ADC_PRV_CHAN(adc_global->channel_val)].current_pos;

    spi_get_hw(adc_global->spi)->icr = 0;//Reset SPI Interrupt Control Register
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
    return 0;
}
