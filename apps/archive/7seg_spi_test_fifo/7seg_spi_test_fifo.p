.setcallreg r28.w0
.origin 0
.entrypoint START

#include "../sage_pru.hp"

#define SCLK        SPI1_SCLK
#define MOSI        SPI1_MOSI
#define MISO        SPI1_MISO
#define CS          SPI1_CS




START:

    // Set initial pin values
    SET CS
    CLR SCLK
    CLR MOSI
    
    delay 32000 // 800 instructions per clock cycle        

    CLR CS
    
    MOV r10, 0x76
    CALL SPI_XFER_BYTE

    delay 32000 // 800 instructions per clock cycle        
    
    SET CS
    
    delay 32000 // 800 instructions per clock cycle        

RUNLOOP:

    delay 32000

    LBCO r0, CONST_PRUDRAM, 0, 20
    QBBC EXIT, r4.t0

    CLR CS
    
    delay 32000

    MOV r10, r0
    CALL SPI_XFER_BYTE
    
    MOV r10, r1
    CALL SPI_XFER_BYTE
    
    MOV r10, r2
    CALL SPI_XFER_BYTE
    
    MOV r10, r3
    CALL SPI_XFER_BYTE

    delay 32000

    SET CS

    delay 32000

    JMP RUNLOOP
    
EXIT:

    // Tell the host program we're DONE    
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    HALT
    
// SPI byte transfer - sends r10 and reads into r11
SPI_XFER_BYTE:

    MOV r12, 8 // Read 8 bits total

SPI_XFER_BYTE_LOOP:
    
    // Set MOSI to the current bit we are writing
    QBBS SPI_XFER_BYTE_SET, r10, 7
    CLR MOSI
    JMP SPI_XFER_BYTE_DONE
    
    SPI_XFER_BYTE_SET:
    SET MOSI
    
    SPI_XFER_BYTE_DONE:
    // MOV MOSI, r0.t7
    LSL r10, r10, 1
    
    // Rise the clock
    SET SCLK
    
    // Read the next bit
    // MOV r1.t0, MISO
    // LSL r1, r1, 1

    delay 2000
    
    CLR SCLK
    
    delay 2000

    SUB r12, r12, 1
	QBNE SPI_XFER_BYTE_LOOP, r12, 0

    delay 8000

    RET
    

