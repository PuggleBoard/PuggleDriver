#include <stdio.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#define PRU_NUM 	0
#define AM33XX

static int LOCAL_exampleInit ();
static void *pruDataMem;
static unsigned char *pruDataMem_byte;

int main (void)
{
    unsigned int ret, i, j;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    
    printf("\nINFO: Starting %s example.\r\n", "blinkslave");
    /* Initialize the PRU */
    prussdrv_init ();		
    
    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
    
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    printf("\tINFO: Initializing example.\r\n");
    LOCAL_exampleInit();
    
    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./blinkslave.bin");

    // Instead of waiting patiently for the PRU to finish, we're going to screw around with the shared memory and hopefully influence the PRU
    
    j = 20;

    while (--j) {
	for (i = 0; i < 3; i++) {
            pruDataMem_byte[i] = 1;
            usleep(50000);
	}
	for (i = 0; i < 3; i++) {
            pruDataMem_byte[i] = 0;
            usleep(50000);
	}
    }

    pruDataMem_byte[3] = 0;
    
    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();

    return(0);

}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ()
{  
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    pruDataMem_byte = (unsigned char*) pruDataMem;

    pruDataMem_byte[0] = 0;
    pruDataMem_byte[1] = 0;
    pruDataMem_byte[2] = 0;
    pruDataMem_byte[3] = 1;

    return(0);
}
