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
    
    delay 32000

    MOV r0, 0x0B
    CALL SPI_XFER_BYTE
    
    MOV r0, 0x0E
    CALL SPI_XFER_BYTE
    
    MOV r0, 0x0E
    CALL SPI_XFER_BYTE
    
    MOV r0, 0x0F    
    CALL SPI_XFER_BYTE

    delay 32000

    SET CS

    // Tell the host program we're DONE    
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    HALT
    
// SPI byte transfer - sends r0 and reads into r1
SPI_XFER_BYTE:

    MOV r2, 8 // Read 8 bits total
    ZERO 4, 4 // Zero R4

SPI_XFER_BYTE_LOOP:
    
    // Set MOSI to the current bit we are writing
    QBBS SPI_XFER_BYTE_SET, r0, 7
    CLR MOSI
    JMP SPI_XFER_BYTE_DONE
    
    SPI_XFER_BYTE_SET:
    SET MOSI
    
    SPI_XFER_BYTE_DONE:
    // MOV MOSI, r0.t7
    LSL r0, r0, 1
    
    // Rise the clock
    SET SCLK
    
    // Read the next bit
    // MOV r1.t0, MISO
    // LSL r1, r1, 1

    delay 2000
    
    CLR SCLK
    
    delay 2000

    SUB r2, r2, 1
	QBNE SPI_XFER_BYTE_LOOP, r2, 0

    delay 8000

    RET
    

