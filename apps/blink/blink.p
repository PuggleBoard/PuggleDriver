.origin 0
.entrypoint START

#include "../prus/PRU.hp"

#define AM33XX

START:

CLR SPI_SCLK
//MOV r1, 10

//LOOP:
  delay 2
  SET SPI_SCLK
  delay 2
  CLR SPI_SCLK
//  SUB r1, r1, 1
//  QBNE LOOP, r1, 0
//  JMP EXIT

//EXIT:
  MOV r31.b0, PRU0_ARM_INTERRUPT+16
  HALT
