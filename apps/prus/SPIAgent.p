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

#define DAC_COUNT       r1
#define ADC_COUNT       r2
#define CHAN_NUM        r7
#define DAC_CH1	        r8
#define DAC_CH2        	r9
#define DAC_CH3	        r10
#define DAC_CH4	        r11

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

START:

// Enable OCP
LBCO r0, CONST_PRUCFG, 4, 4
CLR r0, r0, 4
SBCO r0, CONST_PRUCFG, 4, 4

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
MOV ADC_COUNT, 16
MOV CUR_SAMPLE, 65535
MOV CHAN_NUM, 1
MOV DAC_CH1, 0x31ffff

// Set channel registers
SET_CHANNEL:
  // Enable CONVST
  CLR SPI1_CNV

  delay 10

  // Disable CONVST
  SET SPI1_CNV

  delay 20

  // Enable CS for ADC
  CLR SPI1_CS
 
  // Configure DAC Channel Number
  QBEQ DAC_1, CHAN_NUM, 1
  QBEQ DAC_2, CHAN_NUM, 2 
  QBEQ DAC_3, CHAN_NUM, 3 
  QBEQ DAC_4, CHAN_NUM, 4

  DAC_1:
    SUB DAC_CH1, DAC_CH1, 1
    MOV SPI_TX, DAC_CH1
    JMP ADC_LOOP
  DAC_2:
    MOV SPI_TX, DAC_CH2
    JMP ADC_LOOP
  DAC_3:
    MOV SPI_TX, DAC_CH3
    JMP ADC_LOOP
  DAC_4:
    MOV SPI_TX, DAC_CH4
    MOV CHAN_NUM, 0
    JMP ADC_LOOP

// Start ADC SPI
ADC_LOOP:
  // Enable SCLK
  SET SPI_SCLK

  // Disable SCLK
  CLR SPI_SCLK

  delay 1

  SET SPI_RX.w0, R31.b0, ADC_COUNT

  // Keep running?
  SUB ADC_COUNT, ADC_COUNT, 1
  QBNE ADC_LOOP, ADC_COUNT, 2
  SET SPI1_CS
  SET SPI_SCLK
  delay 10
  CLR SPI0_CS
  //JMP RESET

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

  delay 1

  // Enable SCLK
  CLR SPI_SCLK

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

  delay 1

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
  MOV ADC_COUNT, 16
  MOV DAC_COUNT, 23
  //MOV DAC_CH1, 0x310000
  MOV DAC_CH2, 0x32ffff
  MOV DAC_CH3, 0x34f0f0
  MOV DAC_CH4, 0x3800ff

  SUB CUR_SAMPLE, CUR_SAMPLE, 1
  ADD CHAN_NUM, CHAN_NUM, 1
  QBEQ FS_DELAY, CHAN_NUM, 4

  FS_DELAY:
    delay 4000
    QBNE SET_CHANNEL, CUR_SAMPLE, 0
    JMP EXIT

EXIT:
  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT
