//
//	 -------------------------------------------------------------------------
//
//	 This file is part of the Puggle Data Conversion and Processing System
//	 Copyright (C) 2013 Puggle
//
//	 -------------------------------------------------------------------------
//
//	Written in 2013 by: Yogi Patel <yapatel@gatech.edu>
//
//	To the extent possible under law, the author(s) have dedicated all copyright
//	and related and neighboring rights to this software to the public domain
//	worldwide. This software is distributed without any warranty.
//
//	You should have received a copy of the CC Public Domain Dedication along with
//	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
//

.origin 0
.entrypoint START

#include "puggle.hp"

#define DAC_DATA            r1
#define ADDR_DDR            r3
#define ADDR_PRU1_DRAM      r2
#define ADDR_PRU_SHARED     r4
#define PRU_DDR_XFER_SAMPLE r5
#define CYCLES              r6

START:
// Enable OCP
LBCO  r0, CONST_PRUCFG, 4, 4
CLR   r0, r0, 4
SBCO  r0, CONST_PRUCFG, 4, 4

// Configure pointer register for PRU0 by setting c28_pointer[15:0] 0x00010000 (PRU shared RAM)
MOV   r0, 0x00000100
MOV   r1, CTPPR_0_0
ST32  r0, r1

// Configure pointer register for PRU0 by setting c31_pointer[15:0] to 0x80001000 (DDR memory)
MOV   r0, 0x00100000
MOV   r1, CTPPR_0_1
ST32  r0, r1

MOV ADDR_DDR, (DDR_BASE_ADDR+DDR_OFFSET)
MOV ADDR_PRU1_DRAM, OTHER_DRAM_ADDR
MOV ADDR_PRU_SHARED, PRU_SHARED_ADDR
MOV CYCLES, 0

INIT:

  // Read PRU memory and store into register
  LBBO PRU_DDR_XFER_SAMPLE, ADDR_PRU_SHARED, 0, 2

  // Move value from register to DDR
  SBBO PRU_DDR_XFER_SAMPLE, ADDR_DDR, 0, 2

  // Read DDR memory and store into register
  LBBO  DAC_DATA, ADDR_DDR, 0, 2

  // Move value from register to PRU1 DRAM
  SBBO  DAC_DATA, ADDR_PRU1_DRAM, 0, 2

  // Incrememnt memory addresses
  ADD ADDR_PRU_SHARED, ADDR_PRU_SHARED, 2
  ADD ADDR_PRU1_DRAM, ADDR_PRU1_DRAM, 2
  ADD ADDR_DDR, ADDR_DDR, 2
  ADD CYCLES, CYCLES, 1

  // Cycle back
  QBLE INIT, CYCLES, 200

  // Reset memory addresses
  MOV ADDR_DDR, 0x80001000
  MOV ADDR_PRU1_DRAM, OTHER_DRAM_ADDR
  MOV ADDR_PRU_SHARED, PRU_SHARED_ADDR
  MOV CYCLES, 0
  JMP INIT

EXIT:
  MOV r31.b0, PRU0_ARM_INTERRUPT+16
  HALT
