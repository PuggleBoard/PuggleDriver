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

#define ADDR_PRURAM         r16
#define ADDR_DDR            r17
#define ADDR_DDR_PARAMS     r18
#define CUR_SAMPLE          r19
#define CONTROLS            r20
#define CUR_PRU0_PAGE       r20.w0
#define CUR_PRU1_PAGE       r20.w2
#define CUR_PAGE_OFFSET     r21
#define NUM_PAGES           r22.w0
#define PAGE_SIZE           r22.w2
#define ADDR_CUR_PAGE       r23
#define ADDR_SHARED         r24
#define CUR_DDR_OFFSET      r25
#define NUM_DDR_PAGES       r26
#define CUR_DDR_PAGE        r27
#define TOTAL_PAGES_WRITTEN r28

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

INIT:
  // Read DDR for controls
  //MOV CONTROLS, COMMANDS_ADDR
  //LBCO CONTROLS, CONST_DDR, 0, 4

  // Store controls into PRU shared RAM
  //SBCO CONTROLS, CONST_PRU_SHAREDRAM, 0, 4

  JMP INIT

EXIT:
  MOV r31.b0, PRU0_ARM_INTERRUPT+16
  HALT
