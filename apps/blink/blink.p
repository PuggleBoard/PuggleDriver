.origin 0
.entrypoint START

#include "../prus/PRU.hp"

#define AM33XX

START:

MOV r1, 1000

LOOP:

CLR SPI1_SCLK
SET SPI1_SCLK
delay 10
QBNE LOOP, r1, 0

MOV r31.b0, PRU1_ARM_INTERRUPT+16
HALT
