.origin 0
.entrypoint START

#include "../prus/PRU.hp"

#define AM33XX

START:

MOV r1, 1000

LOOP:
  CLR SPI_SCLK
  SET SPI_SCLK
  delay 10
  QBNE LOOP, r1, 0
  JMP EXIT

EXIT:
  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT
