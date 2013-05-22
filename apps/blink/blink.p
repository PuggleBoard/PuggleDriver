.origin 0
.entrypoint START

#include "../prus/PRU.hp"

#define AM33XX

START:

CLR SPI0_CS
CLR SPI0_SCLK
delay_s 2
SET SPI0_CS
delay_s 2
QBBS LED, SPI1_MISO
QBBC STOP, SPI1_MISO

LED:
  SET SPI0_SCLK
  delay_s 2
  CLR SPI0_CS

STOP:
  HALT
