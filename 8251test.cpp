/*
Copyright 2022 Majenko Technologies

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <dos.h>
#include <bios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F_NONE 0
#define F_LOOPBACK 1
#define F_INIT 2
#define F_BAUD 4


void printStatus(unsigned char status) {
        printf("Status:\n");
        printf("    DSR:     %c\n", status & 0x80 ? '1' : '.');
        printf("    BRKDET:  %c\n", status & 0x40 ? '1' : '.');
        printf("    FE:      %c\n", status & 0x20 ? '1' : '.');
        printf("    OE:      %c\n", status & 0x10 ? '1' : '.');
        printf("    PE:      %c\n", status & 0x08 ? '1' : '.');
        printf("    TxEMPTY: %c\n", status & 0x04 ? '1' : '.');
        printf("    RxRDY:   %c\n", status & 0x02 ? '1' : '.');
        printf("    TxRDY:   %c\n", status & 0x01 ? '1' : '.');
}

int getStatus(command) {
        return inportb(command);
}

int tx(int data, int command, unsigned char b) {
        unsigned long ts = time(NULL);
        while ((getStatus(command) & 4) == 0) {
                if (time(NULL) - ts > 5) {
                        return -1;
                }
        }
        outportb(data, b);
        ts = time(NULL);
        while ((getStatus(command) & 4) == 0) {
                if (time(NULL) - ts > 5) {
                        return -1;
                }
        }
        return 0;
}

int rx(int data, int command) {
        unsigned long ts = time(NULL);
        while ((getStatus(command) & 2) == 0) {
                if (time(NULL) - ts > 5) {
                        return -1;
                }
        }
        return inportb(data);
}

int main(int argc, char **argv) {

        printf("8251TEST (c) 2022 Majenko Technologies\n");
        printf("\n");

        int command = 0x343;
        int data = 0x342;

        int function = 0;

        for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '/') {
                       
                        char opt = argv[i][1] & 0x5F;
                        if (strlen(argv[i]) >= 3) {
                                if (argv[i][2] == ':') {
                                        switch (opt) {
                                                case 'C': // /C:343
                                                        command = strtoul(argv[i] + 3, NULL, 16);
                                                        break;
                                                case 'D': // /D:342
                                                        data = strtoul(argv[i] + 3, NULL, 16);
                                                        break;
                                                default:
                                                        printf("Unknown option: %c\n", opt);
                                                        break;
                                        }
                                } else {
                                        printf("Badly formatted option: %c\n", opt);
                                }
                        } else {
                                switch (opt) {
                                        case 'B':
                                                function |= F_BAUD;
                                                break;
                                        case 'L': 
                                                function |= F_LOOPBACK;
                                                break;
                                        case 'I':
                                                function |= F_INIT;
                                                break;
                                        default:
                                                printf("Unknown option: %c\n", opt);
                                                break;
                                }
                        }
                }
        }

        if (function == F_NONE) {
                printf("Please specify options to execute:\n");
                printf("    /L - Loopback test\n");
                printf("    /B - Baud rate test\n");
                printf("    /I - Cold initialization\n");
                return -1;
        }


        if (function & F_INIT) {
                printf("Performing cold init...");
                outportb(command, 0x4e);
        } else {
                printf("Performing warm init...");
                outportb(command, 0x40);
                outportb(command, 0x4e);
        }

        outportb(command, 0x05);

        printf("done\n");

        unsigned char status = getStatus(command);
        printStatus(status);

        if (function & F_LOOPBACK) {
                printf("Running loopback test...");
        
                for (int i = 0; i < 256; i++) {
                        int c = tx(data, command, i);
                        if (c < 0) {
                                printf("TX timeout on %d\n", i);
                                printStatus(getStatus(command));
                                return -1;
                        }
                        c = rx(data, command);
                        if (c < 0) {
                                printf("RX timeout on %d\n", i);
                                printStatus(getStatus(command));
                                return -1;
                        }
                        if (c != i) {
                                printf("Mismatch on %d != %d\n", c, i);
                                return -1;
                        }
                }
                printf("pass\n");
        
        }

        if (function & F_BAUD) {
                printf("Running baud rate test. Press any key to stop\n");
                while (bioskey(0) == 0) {
                        if(tx(data, command, 'U') < 0) {
                                printf("TX timeout\n");
                                return -1;
                        }
                }
                bioskey(1);
        }

        return 0;
}
