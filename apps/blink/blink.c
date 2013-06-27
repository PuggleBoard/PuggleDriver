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

	/* Initialize the PRU */
	prussdrv_init ();		

	// Set ADC SCLK
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 76.\n");
		return(1);
	}
	fprintf(fp,"76");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio76/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file 76.");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_data6",0x0d);

	/* Open PRU Interrupt */
	ret = prussdrv_open(PRU_EVTOUT_1);
	if (ret)
	{
		printf("prussdrv_open open failed.\n");
		return (ret);
    }
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Execute example on PRU */
    printf("Executing example.\n");
    prussdrv_exec_program (PRU_NUM, "./blink.bin");
    
    /* Wait until PRU1 has finished execution */
    printf("Waiting for HALT command.\n");
    prussdrv_pru_wait_event(PRU_EVTOUT_1);

    printf("PRU1 completed transfer.\n");
    prussdrv_pru_clear_event(PRU1_ARM_INTERRUPT);

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM);
    prussdrv_exit();

    return(0);
}
