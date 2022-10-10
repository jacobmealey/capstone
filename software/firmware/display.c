// display.c 
// Authors: Jacob Mealey <jacob.mealey@maine.edu>
//          Landyn Francis <landyn.francis@maine.edu>
//

#include "display.h"
#include "pins.h"


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



int screan_to_disp(uint16_t *screen, uint8_t *disp, int screen_len) {
    uint8_t acc = 0; // accumulator for screen translation
    int acc_set = 0;
    int i = 0; 
    int bi = 0;
    while(i< screen_len) {
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
            disp[bi] = ((acc << 4) & 0xF0) | ((screen[i] >> 8) & 0x0F);
            acc = screen[i] & 0xFF; 
            acc_set = 2;
            bi++;
            i++;
        }
    }
    return bi;
}
