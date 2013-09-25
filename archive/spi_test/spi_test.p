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
    
    //  Wait 100ns
    MOV r29, 9
    NOP
    SETUP_WAIT:
        SUB r29, r29, 1
        QBNE SETUP_WAIT, r29, 0
        

// Transfer 1 words
MOV r10, 1 

TRANSFER_WORD:

    //  Wait 80ns after CS went low
    MOV r29, 7
    NOP
    CS_WAIT:
        SUB r29, r29, 1
        QBNE CS_WAIT, r29, 0

    // Set CS low
    CLR CS

    // Wait 15ns
    NOP
    
    // We want to transfer 14 bits
    MOV r11, 14 

    // TODO: Set MOSI to the first bit of whatever we want to send
    NOP

    TRANSFER_BIT:
            
        // Set SCLK high
        SET SCLK
        
        // TODO: Read in MISO 
        NOP
        
        // Wait another 10ns
        NOP
        NOP
        
        // Set clock to zero
        CLR SCLK
        
        // Wait 5ns
        NOP
    
        // TODO: Set MOSI to the next bit of whatever we want to send
        NOP
        
        SUB r11, r11, 1
        QBNE TRANSFER_BIT, r11, 0
	
	// Send the last two clocks
    SET SCLK
    NOP
    NOP
    NOP
    CLR SCLK
    NOP
    NOP

    // Return CS to high
    CLR CS

    // Loop to the next word
    SUB r10, r10, 1
    QBNE TRANSFER_WORD, r10, 0
    
    
// Tell the host program we're DONE    
MOV R31.b0, PRU0_ARM_INTERRUPT+16
HALT
