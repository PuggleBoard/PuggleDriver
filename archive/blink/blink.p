.origin 0
.entrypoint START

#include "../prus/PRU.hp"

#define AM33XX

START:

CLR SPI_SCLK

delay 
SET SPI_SCLK
delay 2
CLR SPI_SCLK

MOV r31.b0, PRU0_ARM_INTERRUPT+16
HALT
