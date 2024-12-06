#include <stdint.h>
#include <stdio.h>
#include "magidoor/MagiDoor.h"

#define ACIA_CONTROL 0
#define ACIA_STATUS  0
#define ACIA_DATA    1

#define TDRF (1 << 1)
#define RDRE (1 << 0)

uint8_t input[256];
uint8_t input_idx = 0;
uint8_t input_proc = 0;

void inject6850(char c) {
    input[input_idx++] = c;
    input_idx %= sizeof(input);
}

uint8_t read6850(uint16_t address) {
    uint8_t ret;

    switch (address & 1) {
        case ACIA_STATUS:
            ret = TDRF;
            if (input_proc != input_idx) {
                ret |= RDRE;
            }
            return ret;
        case ACIA_DATA:
            if (input_proc != input_idx) {
                ret = input[input_proc++];
                input_proc %= sizeof(input);

                return ret;
            }
            break;

    }
    return 0xff;
}

void write6850(uint16_t address, uint8_t value){
    switch (address & 1) {
        case ACIA_CONTROL:
            break;
        case ACIA_DATA:
            md_printf("%c", value);
            break;
    }
}
