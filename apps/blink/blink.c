#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "prussdrv.h"
#include <pruss_intc_mapping.h>

#define PRU_NUM 	1
#define AM33XX

int mux(char *name, int val) {
	char cmd[1024];
	sprintf(cmd, "echo %x > /sys/kernel/debug/omap_mux/%s", val, name);
	if (system(cmd) != 0) {
		printf("ERROR: Failed to set pin mux %s = %x\n", name, val);
		return -1;
	}
	return 0;
}

int main (void)
{
	static FILE *fp = 0;
	unsigned int ret;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

	printf("\nINFO: Starting %s example.\r\n", "blink");
	/* Initialize the PRU */
	prussdrv_init ();		

	// Set ADC CS
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 62.\n");
		return (1);
	}
	fprintf(fp,"62");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio62/direction", "w"))==NULL){
		printf("cannot open gpio direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_pclk",0x0d);

	/* Open PRU Interrupt */
	ret = prussdrv_open(PRU_EVTOUT_0);
	if (ret)
	{
		printf("prussdrv_open open failed\n");
		return (ret);
    }
    
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./blink.bin");
    
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

