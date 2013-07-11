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
.entrypoint INIT

#include "PRU_cape.hp"

#define DAC_COUNT       r1
#define ADC_COUNT       r2
#define CHAN_NUM        r3
#define ADC_CH1         r4
#define ADC_CH2         r5
#define ADC_CH3         r6
#define ADC_CH4         r7
#define DAC_CH1	        r8
#define DAC_CH2        	r9
#define DAC_CH3	        r10
#define DAC_CH4	        r11
#define ADC_INIT        r12

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

// Initialize!
INIT:

// Configure DAC channels
MOV DAC_CH1, 0
MOV DAC_CH2, 0
MOV DAC_CH3, 0
MOV DAC_CH4, 0
MOV DAC_CH1.b2, 0x31
MOV DAC_CH2.b2, 0x32
MOV DAC_CH3.b2, 0x34
MOV DAC_CH4.b2, 0x38

MOV ADC_CH1, 0
MOV ADC_CH2, 0
MOV ADC_CH3, 0
MOV ADC_CH4, 0

// Enable OCP
LBCO r0, CONST_PRUCFG, 4, 4
CLR r0, r0, 4
SBCO r0, CONST_PRUCFG, 4, 4

// Configure pointer register for PRU1 by setting c28_pointer[15:0]
// 0x00012000 (PRU Shared RAM)
MOV  r0, 0x00012000

// Initialize SPI SCLK
SET SPI_SCLK

// Initialize DAC SPI busses
SET SPI0_CS
CLR SPI0_MOSI
CLR SPI0_MISO

// Initialize ADC SPI Buses
SET SPI1_CS
SET SPI1_CNV

// Set loop count
MOV DAC_COUNT, 23
MOV ADC_COUNT, 15
MOV CUR_SAMPLE, 65535
MOV CHAN_NUM, 1
MOV ADC_INIT, 0xe7ff

CLR SPI1_CS

// Run ADC SPI once
ADC_INIT_LOOP:
  // Configure ADC MOSI
  QBBS ADC_MOSI_HIGH, ADC_INIT, ADC_COUNT

  // Enable SCLK
  SET SPI_SCLK

  // Enable MOSI LOW
  CLR SPI1_MOSI
  JMP ADC_MOSI_DONE

  // Enable MOSI HIGH
  ADC_MOSI_HIGH:
    SET SPI_SCLK
    SET SPI1_MOSI
    delayOne

  ADC_MOSI_DONE:
  delayTwo
  CLR SPI_SCLK
  delayOne

  // Keep running?
  SUB ADC_COUNT, ADC_COUNT, 1
  QBNE ADC_INIT_LOOP, ADC_COUNT, 0
  
  delayOne
  SET SPI_SCLK
  QBBS ADC_BELOW, ADC_INIT, ADC_COUNT

  CLR SPI1_MOSI
  CLR SPI_SCLK
  delayThree
  JMP RESET
  
  ADC_BELOW:
    SET SPI1_MOSI
    delayTwo
    CLR SPI_SCLK
    delayFour

  SET SPI_SCLK
  SET SPI1_CS
  JMP RESET

// Start acquisition!
SET_CHANNEL:
  // Enable CONVST
  CLR SPI1_CNV

  delayTen

  // Disable CONVST
  SET SPI1_CNV

  delayTen
  delayTen

  // Enable CS for ADC
  CLR SPI1_CS
 
  // Configure DAC Channel Number
  QBEQ DAC_1, CHAN_NUM, 1
  QBEQ DAC_2, CHAN_NUM, 2 
  QBEQ DAC_3, CHAN_NUM, 3 
  QBEQ DAC_4, CHAN_NUM, 4

  DAC_1:
    MOV SPI_TX, DAC_CH1
    MOV SPI_RX, ADC_CH1
    JMP ADC_LOOP
  DAC_2:
    MOV SPI_TX, DAC_CH2
    MOV SPI_RX, ADC_CH2
    JMP ADC_LOOP
  DAC_3:
    MOV SPI_TX, DAC_CH3
    MOV SPI_RX, ADC_CH3
    JMP ADC_LOOP
  DAC_4:
    MOV SPI_TX, DAC_CH4
    MOV SPI_RX, ADC_CH4
    MOV CHAN_NUM, 0
    JMP ADC_LOOP

// Start ADC SPI
ADC_LOOP:
  // Disable SCLK
  CLR SPI_SCLK

  delayThree

  SET SPI_RX.w0, R31.b0, ADC_COUNT

  SET SPI_SCLK

  delayTwo

  // Keep running?
  SUB ADC_COUNT, ADC_COUNT, 1
  QBNE ADC_LOOP, ADC_COUNT, 0
  SET SPI1_CS
  SET SPI_SCLK
  CLR SPI0_CS

// Start DAC SPI
DAC_LOOP: 
  // Configure MOSI
  QBBS MOSI_HIGH, SPI_TX, DAC_COUNT

  // Enable SCLK
  SET SPI_SCLK

  // Enable MOSI LOW
  CLR SPI0_MOSI
  JMP MOSI_DONE

  // Enable MOSI HIGH
  MOSI_HIGH:
    SET SPI_SCLK
    SET SPI0_MOSI

  MOSI_DONE:

  delayTwo

  // Enable SCLK
  CLR SPI_SCLK

  delayOne

  // Keep running?
  SUB DAC_COUNT, DAC_COUNT, 1
  QBNE DAC_LOOP, DAC_COUNT, 0 
  
  // Configure MOSI
  QBBS MOSI_HIGH_FINAL, SPI_TX, DAC_COUNT

  // Enable SCLK
  SET SPI_SCLK

  // Enable MOSI LOW
  CLR SPI0_MOSI
  JMP MOSI_DONE_FINAL

  // Enable MOSI HIGH
  MOSI_HIGH_FINAL:
    SET SPI_SCLK
    SET SPI0_MOSI

  MOSI_DONE_FINAL:

  delayTwo

  // Enable SCLK
  CLR SPI_SCLK
  
  JMP RESET

// Reset pin states
RESET:
  // DAC Lines
  SET SPI_SCLK
  SET SPI0_CS
  CLR SPI0_MOSI
  CLR SPI0_MISO

  // ADC Lines
  SET SPI1_CS
  SET SPI1_CNV

  // Counters
  MOV ADC_COUNT, 15
  MOV DAC_COUNT, 23
  MOV DAC_CH1.w0, ADC_CH1.w0
  MOV DAC_CH2.w0, ADC_CH2.w0
  MOV DAC_CH3.w0, ADC_CH3.w0
  MOV DAC_CH4.w0, ADC_CH4.w0
  MOV ADC_CH1.w2, 0x1000
  MOV ADC_CH2.w2, 0x2000
  MOV ADC_CH3.w2, 0x3000
  MOV ADC_CH4.w2, 0x4000

  SUB CUR_SAMPLE, CUR_SAMPLE, 1
  ADD CHAN_NUM, CHAN_NUM, 1
  QBEQ FS_DELAY, CHAN_NUM, 4

  FS_DELAY:
    delay 1000
    QBNE SET_CHANNEL, CUR_SAMPLE, 0
    JMP EXIT

EXIT:
  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT
