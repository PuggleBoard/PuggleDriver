.origin 0
.entrypoint START

#include "../prus/PRU.hp"

#define AM33XX

START:

CLR SPI1_SCLK
delay_s 2
SET SPI1_SCLK
delay_s 2
CLR SPI1_SCLK
delay_s 2
SET SPI1_SCLK
delay_s 2
CLR SPI1_SCLK
JMP EXIT
delay_s 2
SET SPI1_SCLK
delay_s 2

EXIT:
CLR SPI1_SCLK
HALT
