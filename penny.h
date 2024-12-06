#pragma once

#include <stdint.h>


extern uint32_t instructions;
extern void reset6502();
extern void step6502();

extern uint8_t read6502(uint16_t address);
extern void write6502(uint16_t address, uint8_t value);


extern uint8_t read6850(uint16_t address);
extern void write6850(uint16_t address, uint8_t value);

extern void inject6850(char c);

extern unsigned char _binary_ROM_bin_end;
extern unsigned char _binary_ROM_bin_size;
extern unsigned char _binary_ROM_bin_start;
