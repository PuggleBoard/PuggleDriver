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

#include "PRU_cape.hp"

#define CUR_OFFSET      r16
#define RUN_FLAG        r17
#define ADDR_PRURAM     r18
#define NUM_SAMPLES     r19
#define NUM_PAGES       r20
#define CUR_SAMPLE      r21
#define CUR_PAGE        r22
#define SPI_TX          r23
#define SPI_RX          r24
#define SPI_CUR_BIT     r25
#define PRU1_CUR_PAGE   r26
#define ADDR_CUR_PAGE   r27
#define ADDR_SHARED     r28

// Set as number of cycles to delay, each cycle is 5ns with 50% duty cycle
#define SCLK_FREQ       5  // 20MHz

START:

// Enable OCP
LBCO r0, CONST_PRUCFG, 4, 4
CLR r0, r0, 4
SBCO r0, CONST_PRUCFG, 4, 4

// Initialize SPI busses
SET SPI_SCLK
SET SPI0_CS
CLR SPI0_MOSI
CLR SPI0_MISO

// Set loop count
MOV r1, 16
MOV CUR_SAMPLE, 1000

delay 1

// Start SPI
//LOOP: 
  // Enable CS
  CLR SPI0_CS

  // Enable SCLK
  CLR SPI_SCLK

  // Enable MOSI
  SET SPI0_MOSI

  //delay 2
  SET SPI_SCLK
  //delay 2

  // Keep running?
  //SUB r1, r1, 1
  //QBNE LOOP, r1, 0
  //JMP RESET

//EXIT:
  SET SPI_SCLK
  SET SPI0_CS
  CLR SPI0_MOSI
  CLR SPI0_MISO
  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT

// Initialize SPI busses
RESET:
SET SPI_SCLK
SET SPI0_CS
CLR SPI0_MOSI
CLR SPI0_MISO
MOV r1, 16
delay 1000
SUB CUR_SAMPLE, CUR_SAMPLE, 1
//QBNE LOOP, r21, 0
//JMP EXIT
