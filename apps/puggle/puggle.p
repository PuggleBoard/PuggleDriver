.origin 0
.entrypoint START
#include "puggle.hp"

//////////////////////////////////////
START:
// Enable OCP master port
LBCO r0, C4, 4, 4
CLR r0, r0, 4
SBCO r0, C4, 4, 4

// Enable CLKSPIREF and CLK
MOV addr, CM_PER_SPI1_CLK_CTRL
MOV val, CM_PER_SPI1_CLK_EN
SBBO val, addr, 0, 4

// Reset SPI
MOV addr, MCSPI_SYSCONFIG
LBBO val, addr, 0, 4
SET val.t1
SBBO val, addr, 0, 4

//////////////////////////////////////
// Validate reset
CHECKRESET:
MOV addr, MCSPI_SYSSTATUS
LBBO val, addr, 0, 4
QBBC CHECKRESET, val.t0

// Configure MODULCTRL
MOV addr, MCSPI_MODULCTRL
MOV val, MODCONTROL
SBBO val, addr , 0, 4

// Configure SYSCONFIG
MOV  addr, MCSPI_SYSCONFIG
MOV  val, ADC_SYSCONFIG
SBBO val, addr, 0, 4

// Reset interrupt status bits by writing all ones
MOV addr, MCSPI_IRQSTATUS
MOV val, RESET_IRQ_STAT 
SBBO val, addr, 0, 4

// Enable interupts
MOV addr, MCSPI_IRQENABLE
MOV val, ADC_IRQENABLE
SBBO val, addr, 0, 4

//////////////////////////////////////
// Configure MCSPI1 channel 0 - ADC

// Disable channel 0
MOV addr, MCSPI_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

// Configure channel 0 of MCSPI1
MOV addr, MCSPI_CH0CONF     
MOV val, ADC_CH0_CONF
SBBO val, addr, 0, 4

// Set XFER Level
MOV addr, MCSPI_XFERLEVEL
MOV val, ADC_XFER
SBBO val, addr, 0, 4

// Configure MCSPI1 channel 1 - DAC

// Disable channel 1
MOV addr, MCSPI_CH1CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

// Configure channel 1 of MCSPI1
MOV addr, MCSPI_CH1CONF     
MOV val, DAC_CH1_CONF
SBBO val, addr, 0, 4

//////////////////////////////////////
CONFIGURE:

// Set ADC Channel to 0 before starting autocycle
CALL ENABLE_CH0

// Write ADC command to SPI_TX0
MOV addr, MCSPI_TX0
MOV val, ADC_CH0
SBBO val, addr, 0, 4

// Disable channel 0
MOV addr, MCSPI_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

delay

CALL ENABLE_CH0

// Write ADC configuration to SPI_TX0
// Sets ADC CMR and CFR to default
// Autocycle channels
MOV addr, MCSPI_TX0
MOV val, ADC_CONFIG
SBBO val, addr, 0, 4

// Disable channel 0
MOV addr, MCSPI_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

delay

CALL ENABLE_CH1

// Write DAC configuration to SPI_TX1
MOV addr, MCSPI_TX1
MOV val, DAC_CONFIG
SBBO val, addr, 0, 4

// Disable channel 1
MOV addr, MCSPI_CH1CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

delay

SET ADC_CONVST

//////////////////////////////////////
RUN_AQ:

delay

CALL ENABLE_CH0

// Write ADC command to SPI_TX0
MOV addr, MCSPI_TX0
MOV val, READ_ADC_DATA
SBBO val, addr, 0, 4

// Disable channel 0
MOV addr, MCSPI_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

CLR ADC_CONVST

delay

CALL ENABLE_CH1

// Write DAC configuration to SPI_TX1
MOV addr, MCSPI_TX1
MOV val, MCSPI_RX0
SBBO val, addr, 0, 4

// Disable channel 1
MOV addr, MCSPI_CH1CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

delay

JMP EXIT //RUN_AQ

//////////////////////////////////////
CHECKTX0:
MOV addr, MCSPI_CH0STAT
LBBO val, addr, 0, 4
QBBC CHECKTX0, val.t1
JMP r18.w0

ENABLE_CH0:
// Enable Channel 0
MOV addr, MCSPI_CH0CTRL
MOV val, EN_CH
SBBO val, addr, 0, 4
JAL r18.w0, CHECKTX0
RET

CHECKTX1:
MOV addr, MCSPI_CH1STAT
LBBO val, addr, 0, 4
QBBC CHECKTX1, val.t1
JMP r18.w0

ENABLE_CH1:
// Enable Channel 1
MOV addr, MCSPI_CH1CTRL
MOV val, EN_CH
SBBO val, addr, 0, 4
JAL r19.w0, CHECKTX1
RET

//////////////////////////////////////
EXIT:
#ifdef AM33XX
//Send notification to Host for program completion
MOV R31.b0, PRU0_ARM_INTERRUPT+16
#else
MOV R31.b0, PRU0_ARM_INTERRUPT
#endif

HALT
