//
//	 -------------------------------------------------------------------------
//
//	 This file is part of the Puggle Data Conversion and Processing System
//	 Copyright (C) 2013 Puggle
//
//	 -------------------------------------------------------------------------
//
//	Created in 2013 by: Yogi Patel <yapatel@gatech.edu>
//  Written by: Yogi Patel <yapatel@gatech.edu>
//              Jon Newman <jnewman6@gatech.edu>
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

#include "PRU.hp"

#define DAC_COUNT       r1
#define ADC_COUNT       r2
#define CHAN_NUM        r3
#define ADC_CH1         r4
#define ADC_CH2         r5
#define ADC_CH3         r6
#define ADC_CH4         r7
#define DAC_CH1         r8
#define DAC_CH2         r9
#define DAC_CH3         r10
#define DAC_CH4         r11
#define ADC_INIT        r12
#define ADC_WRITE_COUNT r13
#define INIT_CYCLES     r14
#define CUR_SAMPLE      r20
#define CUR_PAGE        r21
#define DAC_TX          r22
#define ADC_TX          r23
#define ADC_DATA        r28
#define ADC_RX          r31
#define ADDR_PRU_SHARED r24

// Initialize
INIT:
MOV DAC_CH1.b2, 0x31
MOV DAC_CH2.b2, 0x32
MOV DAC_CH3.b2, 0x34
MOV DAC_CH4.b2, 0x38
MOV ADC_CH1.w0, 0x0000
MOV ADC_CH2.w0, 0x1000
MOV ADC_CH3.w0, 0x2000
MOV ADC_CH4.w0, 0x3000

// Initialize onboard AIO SPI bus
SET DAC_CS
SET ADC_CS
CLR MOSI

// Set loop count
MOV ADC_COUNT, 15
MOV DAC_COUNT, 23
MOV ADC_WRITE_COUNT, 31
MOV CUR_SAMPLE, 10000

// Set initialization parameters
MOV CHAN_NUM, 1
MOV ADC_INIT, 0xe7ff
MOV INIT_CYCLES, 3

// Setup memory addresses
MOV ADDR_PRU_SHARED, PRU_SHARED_ADDR

CLR ADC_CS

// Run ADC SPI once
ADC_INIT_LOOP:
  // Configure ADC MOSI
  QBBS ADC_INIT_MOSI_HIGH, ADC_INIT, ADC_COUNT

  // Enable SCLK
  SET SCLK

  // Enable MOSI LOW
  CLR MOSI
  JMP ADC_INIT_MOSI_DONE

  // Enable MOSI HIGH
  ADC_INIT_MOSI_HIGH:
    SET SCLK
    SET MOSI
    delayOne

  // Register data on chip 
  ADC_INIT_MOSI_DONE:
    delayTwo
    CLR SCLK
    delayOne

  // Keep running?
  SUB ADC_COUNT, ADC_COUNT, 1
  QBNE ADC_INIT_LOOP, ADC_COUNT, 0
  
  delayOne
  SET SCLK
  QBBS ADC_INIT_FINAL, ADC_INIT, ADC_COUNT

  CLR MOSI
  CLR SCLK
  delayThree
  JMP INIT_OUT
  
  ADC_INIT_FINAL:
    SET MOSI
    delayTwo
    CLR SCLK
    delayFour

  INIT_OUT:
    SET SCLK
    delayTwo
    SET ADC_CS
    MOV ADC_COUNT, 15
    MOV DAC_COUNT, 23
    SUB INIT_CYCLES, INIT_CYCLES, 1
    QBEQ SET_CHANNEL, INIT_CYCLES, 0

    // Set first channel if one cycle thru init
    delayTen
    MOV ADC_INIT, 0xc000//ADC_CH1.w0
    CLR ADC_CS
    JMP ADC_INIT_LOOP

// Start acquisition
SET_CHANNEL:
  
  // Trigger conversion using CONVST
  CLR CNV
  delayTen
  
  // Disable CONVST
  SET CNV

  // ~1600 ns delay for the conversion
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayTwenty
  delayTen
  delayFive

  // Enable CS for ADC
  CLR ADC_CS
 
  // Configure DAC Channel Number
  QBEQ CH_1, CHAN_NUM, 1
  QBEQ CH_2, CHAN_NUM, 2 
  QBEQ CH_3, CHAN_NUM, 3 
  QBEQ CH_4, CHAN_NUM, 4

  CH_1:
    MOV DAC_TX, DAC_CH1
    MOV ADC_TX, ADC_CH2
    SET SCLK
    JMP ADC_LOOP
  CH_2:
    MOV DAC_TX, DAC_CH2
    MOV ADC_TX, ADC_CH3
    SET SCLK
    JMP ADC_LOOP
  CH_3:
    MOV DAC_TX, DAC_CH3
    MOV ADC_TX, ADC_CH4
    SET SCLK
    JMP ADC_LOOP
  CH_4:
    MOV DAC_TX, DAC_CH4
    MOV ADC_TX, ADC_CH1
    MOV CHAN_NUM, 0
    SET SCLK
    JMP ADC_LOOP

ADC_LOOP:

  // ADC write bit
  QBBS ADC_MOSI_HIGH, ADC_TX, ADC_COUNT

  CLR MOSI
  JMP ADC_MOSI_DONE

  ADC_MOSI_HIGH:
    SET MOSI

  ADC_MOSI_DONE:
    SET SCLK
   
  QBBS ADC_MISO_HIGH, ADC_RX.t0

  CLR ADC_DATA, ADC_RX, ADC_COUNT // Needs dst, src, op(31)
  JMP ADC_MISO_DONE

  ADC_MISO_HIGH:
    SET ADC_DATA, ADC_RX, ADC_COUNT // Needs dst, src, op(31)
    delayOne

  ADC_MISO_DONE:
    // Keep running?
    CLR SCLK    
    SUB ADC_COUNT, ADC_COUNT, 1
    QBNE ADC_LOOP, ADC_COUNT, 0

  // ADC write last bit
  delayOne
  SET SCLK
  QBBS ADC_FINAL_MOSI_HIGH, ADC_TX, ADC_COUNT

  CLR MOSI
  JMP ADC_FINAL_MOSI_DONE
  
  ADC_FINAL_MOSI_HIGH:
    SET MOSI
    delayOne
  
  ADC_FINAL_MOSI_DONE:
    delayOne

  // ADC read last bit
  QBBS ADC_FINAL_MISO_HIGH, ADC_RX.t0

  CLR ADC_DATA, ADC_RX, ADC_COUNT // Needs dst, src, op(31)
  JMP ADC_FINAL_MISO_DONE

  ADC_FINAL_MISO_HIGH:
    SET ADC_DATA, ADC_RX, ADC_COUNT // Needs dst, src, op(31)
    delayOne

  ADC_FINAL_MISO_DONE:
    SBBO ADC_DATA, ADDR_PRU_SHARED, 0, 2                // Copy acquired word (4 bytes) to shared memory
    ADD ADDR_PRU_SHARED, ADDR_PRU_SHARED, 2           // Increment address by 4 bytes
    SET SCLK
    delayTwo
    SET ADC_CS
    delayTwo
    CLR DAC_CS

DAC_LOOP: 
  // Configure MOSI
  QBBS DAC_MOSI_HIGH, DAC_TX, DAC_COUNT

  // Enable SCLK
  SET SCLK

  // Enable MOSI LOW
  CLR MOSI
  JMP DAC_MOSI_DONE

  // Enable MOSI HIGH
  DAC_MOSI_HIGH:
    SET SCLK
    SET MOSI

  DAC_MOSI_DONE:
    delayTwo
    CLR SCLK
    delayOne

  // Keep running?
  SUB DAC_COUNT, DAC_COUNT, 1
  QBNE DAC_LOOP, DAC_COUNT, 0 
  
  // Configure MOSI
  QBBS DAC_MOSI_HIGH_FINAL, DAC_TX, DAC_COUNT

  // Enable SCLK
  SET SCLK

  // Enable MOSI LOW
  CLR MOSI
  JMP DAC_MOSI_DONE_FINAL

  // Enable MOSI HIGH
  DAC_MOSI_HIGH_FINAL:
    SET SCLK
    SET MOSI

  DAC_MOSI_DONE_FINAL:
    delayTwo
    CLR SCLK
    JMP RESET

// Reset pin states
RESET:
  // DAC Lines
  SET SCLK
  SET DAC_CS
  CLR MOSI
  //CLR MISO

  // ADC Lines
  SET ADC_CS
  SET CNV

  // Counters
  MOV ADC_COUNT, 15
  MOV DAC_COUNT, 23
  MOV ADC_WRITE_COUNT, 31
  MOV DAC_CH1.w0, 0x0001//ADC_DATA.w0
  MOV DAC_CH2.w0, 0xffff//ADC_DATA.w0
  MOV DAC_CH3.w0, 0xf0f0//ADC_DATA.w0
  MOV DAC_CH4.w0, ADC_DATA.w0

  SUB CUR_SAMPLE, CUR_SAMPLE, 1
  ADD CHAN_NUM, CHAN_NUM, 1
  QBEQ FS_DELAY, CHAN_NUM, 4

  FS_DELAY:
    //delay 500
    QBNE SET_CHANNEL, CUR_SAMPLE, 0
    JMP EXIT

EXIT:
  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT
