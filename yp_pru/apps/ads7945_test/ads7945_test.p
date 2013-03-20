.setcallreg r28.w0
.origin 0
.entrypoint START

#include "../sage_pru_p2cape.hp"

#define SCLK        HSPD_SPI_SCLK
#define MOSI        HSPD_SPI_MOSI
#define MISO        HSPD_SPI_MISO
#define CS          HSPD_SPI_CS

#define ADDR_PRURAM		r0
#define ADDR_DDR		r5
#define RUN_FLAG		r6
#define NUM_SAMPLES		r7

#define ARM_CUR_SAMPLE	r8

#define CUR_SAMPLE		r2

#define SPI_TX			r10
#define SPI_RX			r11
#define SPI_CUR_BIT		r12

START:

    // Enable OCP master port
    LBCO    r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4
    SBCO    r0, CONST_PRUCFG, 4, 4

	MOV		ADDR_PRURAM, 0
	
	// Get the DRAM base address from pru_mem[0] and put it into R5
	LBBO 	ADDR_DDR, ADDR_PRURAM, 0, 4

	// Get the # of samples we have available and put it into R7
	LBBO 	NUM_SAMPLES, ADDR_PRURAM, 4, 4
    
	// Get the # of samples we have available and put it into R7
	LBBO 	NUM_SAMPLES, ADDR_PRURAM, 4, 4

	MOV		ARM_CUR_SAMPLE, ADDR_DDR
	ADD		ARM_CUR_SAMPLE, ARM_CUR_SAMPLE, NUM_SAMPLES
	ADD		ARM_CUR_SAMPLE, ARM_CUR_SAMPLE, NUM_SAMPLES
	
	
    // Set initial pin values
    SET CS
    CLR SCLK
    CLR MOSI
    
    delay 32000 // 800 instructions per clock cycle        

BUFFLOOP:
	// Reset current sample counter
    MOV CUR_SAMPLE, 0
RUNLOOP:

	MOV	SPI_TX, CUR_SAMPLE	
	// Transfer one word via SPI
    CALL SPI_XFER_WORD
    
    // Save SPI result into DDR+CUR_SAMPLE
	SBBO SPI_RX.w0, ADDR_DDR, CUR_SAMPLE, 2

    // Save current sample
	SBBO CUR_SAMPLE, ARM_CUR_SAMPLE, 0, 4
	
	
	
	// Increment CUR_SAMPLE by 2
    ADD CUR_SAMPLE, CUR_SAMPLE, 2
    
    QBLT RUNLOOP, NUM_SAMPLES, CUR_SAMPLE
    
	// Is the run flag still set?
	LBBO RUN_FLAG, ADDR_PRURAM, 8, 4
    QBNE BUFFLOOP, RUN_FLAG, 0
	
	// Runloop is off, so QUIT
	JMP EXIT
    
EXIT:

    // Tell the host program we're DONE    
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    HALT
    
SPI_XFER_WORD:

    CLR CS
    
    MOV SPI_CUR_BIT, 16 // Read 16 bits total
    MOV SPI_RX, 0 // Start what we are reading at 0

    delay 50
    
SPI_XFER_WORD_LOOP:

	// Set MOSI to the current bit of SPI_TX
	QBBS MOSI_HIGH, SPI_TX, SPI_CUR_BIT
	CLR MOSI
	JMP MOSI_DONE
	MOSI_HIGH:
	SET MOSI
	MOSI_DONE:
	
    SET SCLK
    
    // Set b0 of R11 to MISO
    QBBC MISO_LOW, MISO
    OR SPI_RX.b0, SPI_RX.b0, 1
    // r11.b0 is already zero    
    MISO_LOW:
    // Shift left R11 one bit
    LSL SPI_RX, SPI_RX, 1

    delay 20
    
    CLR SCLK
    
    delay 20
    
    SUB SPI_CUR_BIT, SPI_CUR_BIT, 1
	QBNE SPI_XFER_WORD_LOOP, SPI_CUR_BIT, 0

    delay 20
    
    SET CS

    delay 50

    RET
    

