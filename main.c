#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "magidoor/MagiDoor.h"
#include "penny.h"

uint8_t ram[0x4000];

uint32_t rc_crc32(uint32_t crc, const char *buf, size_t len) {
	static uint32_t table[256];
	static int have_table = 0;
	uint32_t rem;
	uint8_t octet;
	int i, j;
	const char *p, *q;

	/* This check is not thread safe; there is no mutex. */
	if (have_table == 0) {
		/* Calculate CRC table. */
		for (i = 0; i < 256; i++) {
			rem = i;  /* remainder from polynomial division */
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				} else
					rem >>= 1;
			}
			table[i] = rem;
		}
		have_table = 1;
	}

	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}

void dump_ram() {
    FILE *fptr;
    char buffer[256];
    struct stat s;
    snprintf(buffer, sizeof buffer, "%s %s", mdcontrol.user_firstname, mdcontrol.user_lastname);


    uint32_t crc = rc_crc32(0, buffer, strlen(buffer));

    snprintf(buffer, sizeof buffer, "%8x.dmp", crc);

    if (stat(buffer, &s) == 0) {
        md_printf("\r\n`bright red`Overwrite existing dump? (Y/N): `white`");
        char ch = md_get_answer("YyNn");
        switch (ch) {
            case 'n':
            case 'N':
                md_printf("\r\n");
                return;
            default:
                break;
        }
    }

    fptr = fopen(buffer, "wb");
    if (!fptr) {
        return;
    }

    fwrite(ram, 1, sizeof(ram), fptr);

    fclose(fptr);

}

int load_ram() {
    FILE *fptr;
    char buffer[256];
    snprintf(buffer, sizeof buffer, "%s %s", mdcontrol.user_firstname, mdcontrol.user_lastname);


    uint32_t crc = rc_crc32(0, buffer, strlen(buffer));

    snprintf(buffer, sizeof buffer, "%8x.dmp", crc);

    fptr = fopen(buffer, "rb");
    if (!fptr) {
        return -1;
    }

    fread(ram, 1, sizeof(ram), fptr);

    fclose(fptr);
    return 0;
}

uint8_t read6502(uint16_t address) {
    if (address < sizeof(ram)) {
        return ram[address];
    }

	if (address >= 0xc000) {
		const uint8_t *rom = &_binary_ROM_bin_start;

		return rom[address - 0xc000];
	}

	if (address >= 0xa000 && address <= 0xbfff) {
		return read6850(address);
	}

    return 0xff;
}

void write6502(uint16_t address, uint8_t value){
    if (address < sizeof(ram)) {
        ram[address] = value;
    }
	if (address >= 0xa000 && address <= 0xbfff) {
		write6850(address, value);
	}
}


int main(int argc, char **argv) {
    int socket = -1;
    char c;

    if (argc < 2) {
        printf("Usage %s DROPFILE [SOCKET]\n", argv[0]);
        return -1;
    }

    if (argc == 3) {
        socket = strtol(argv[2], NULL, 10);
    }

    md_init(argv[1], socket);
    md_clr_scr();
    md_sendfile("fake6502.ans", 0);
    md_getc();

    reset6502();
    while(1) {
        for (int i = 0; i < 100; i++) {
            step6502();
        }
        c = md_getche(0, 1000);
        if (c == -1) {
            continue;
        } else if (c == 0x1a) { // ctrl-z
            md_printf("\r\n`bright red`Exiting --- save memory dump? (Y/N) : `white`");
            char ch = md_get_answer("yYnN");
            switch (ch) {
                case 'y':
                case 'Y':
                    dump_ram();
                    md_printf("\r\n\r\n");
                    break;
                default:
                    md_printf("\r\n\r\n");
                    break;
            }
            break;
        } else if (c == 0x05) { // ctrl-e
            md_printf("\r\n`bright red`(L)oad Memory Dump, (S)ave Memory Dump (C)ancel: `white`");
            char ch = md_get_answer("lLsScC");
            switch (ch) {
                case 's':
                case 'S':
                    dump_ram();
                    break;
                case 'l':
                case 'L':
                    if (!load_ram()) {
                        md_printf("\r\n`bright red`Select WARM start to preserve memory contents after reset.\r\nPRESS A KEY TO RESET\r\n`white`");
                        reset6502();
                    } else {
                        md_printf("\r\n`bright red`No dump found.\r\n`white`\r\n\r\n");
                    }
                    break;
                default:
                    break;
            }
        } else if (c == 0x12) { // ctrl-r
            reset6502();
        } else if (c == '\n' || c == '\0') {
            continue;
        } else if (c == '\r') {
            inject6850('\r');
            inject6850('\n');
        } else {
            inject6850(c);
        }
    }

    md_exit(0);

    return 0;
}
