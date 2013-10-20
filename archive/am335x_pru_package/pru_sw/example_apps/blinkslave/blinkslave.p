.origin 0
.entrypoint START

#include "blinkslave.hp"

#define GPIO1 0x4804c000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

START:

  SET r20.t0
  SET r30.t7
  delayTen
  CLR r30.t7
  QBBS START, r20.t0
  JMP EXIT

EXIT:
  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT
