#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



int main() {

    uint16_t screen[] = {
        0x123, 0x456, 0x789, 0xABC, 0xDEF, 0x123
    };

    uint8_t  buffer[9];
    uint8_t acc = 0;
    int i = 0;
    int bi = 0;

    printf("screen:\n");
    while(bi < 9) {
        // if acc is empty
        if(acc == 0) {
            buffer[bi] = screen[i] >> 4;
            printf("nb  buffer[%d]: %x\n", bi, buffer[bi]);
            acc = screen[i] & 0xF;
            bi++;
            i++;
        } else if(acc & 0xF0) { // if the high bit of acc is set
           buffer[bi] = acc;
           printf("hb  buffer[%d]: %x\n", bi, buffer[bi]);
           acc = 0;
           bi++;
        } else { // final case - acc lowest nib is set
            buffer[bi] = (acc << 4) | ((screen[i] >> 8));
            acc = screen[i] & 0xFF;  
            printf("lb  buffer[%d]: %x\n", bi, buffer[bi]);
            bi++;
            i++;
        }
    }  
    printf("buffer:\n");

    return 0;

}
