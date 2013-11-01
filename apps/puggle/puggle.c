/*
	 -------------------------------------------------------------------------

	 This file is part of the Puggle Data Conversion and Processing System
	 Copyright (C) 2013 Puggle

	 -------------------------------------------------------------------------

	 Written in 2013 by: Yogi Patel <yapatel@gatech.edu>

	 To the extent possible under law, the author(s) have dedicated all copyright
	 and related and neighboring rights to this software to the public domain
	 worldwide. This software is distributed without any warranty.

	 You should have received a copy of the CC Public Domain Dedication along with
	 this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
	 */

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

#define AM33XX
#define PRU_NUM0 													0
#define PRU_NUM1 													1
#define DEBUG
#define ALIGN_TO_PAGE_SIZE(x, pagesize)   ((x)-((x)%pagesize))
#define handle_error(msg) 								do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define PRU_SHARED_OFFSET									0
#define DDR_BASE_ADDR											0x80000000
#define OFFSET_DDR												0x00001000

typedef struct {
	uint32_t    run_flag;
	uint32_t    ddr_base_address;
	uint32_t    ddr_bytes_available;
} pru_params;

typedef struct {
	uint32_t ddr_size;
	uint32_t ddr_base_address;
	void *ddr_memory;
	void *pru_memory;
	pru_params* pru_params;
	int mem_fd;
} app_data;

// System flags
app_data info;
int thread_status = 0;
int system_status = 0;
static unsigned int *sharedMem_int;

// I/O Configuration
static int num_ai_channels;
static int num_ao_channels;
static int sampling_freq;

static int load_pruss_dram_info(app_data *info) {
	info->ddr_size = 0x0FFFFFFF;
	info->ddr_base_address = DDR_BASE_ADDR;
	//printf("DDR size is %dMB starting at address 0x%08lX\n", (int)info->ddr_size/1024/1024, info->ddr_base_address);
	return 0;
}

static int init(app_data *info) {

	load_pruss_dram_info(info);

	info->mem_fd = open("/dev/mem", O_RDWR);
	if (info->mem_fd < 0) {
		printf("Failed to open /dev/mem (%s)\n", strerror(errno));
		return -1;
	}

	info->ddr_memory = mmap(0, info->ddr_size, PROT_WRITE | PROT_READ, MAP_SHARED, info->mem_fd, info->ddr_base_address);
	if (info->ddr_memory == NULL) {
		printf("Failed to map the device (%s)\n", strerror(errno));
		close(info->mem_fd);
		return -1;
	}

	prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &info->pru_memory);
	sharedMem_int = (unsigned short int*) info->pru_memory;

	if(info->pru_memory == NULL) {
		printf("Cannot map PRU1 memory buffer.\n");
		return -ENOMEM;
	}

	printf("Initializing memory.\n");
	info->pru_params = info->pru_memory;

	printf("Initializing PRU parameters.\n");

	// Set the run flag to 1
	sharedMem_int[PRU_SHARED_OFFSET] = 0;

	// Which I/O channels did the user select?
	
	// ADC channel control bits [4:1]
	if(num_ai_channels == 1){
		sharedMem_int[PRU_SHARED_OFFSET+1] = 1;
	}
	else if(num_ai_channels == 2){
		sharedMem_int[PRU_SHARED_OFFSET+1] = 2;
	}
	else if(num_ai_channels == 3){
		sharedMem_int[PRU_SHARED_OFFSET+1] = 3;
	}
	else if(num_ai_channels == 4){
		sharedMem_int[PRU_SHARED_OFFSET+1] = 4;
	}

	// DAC channel control bits [8:5]
	if(num_ai_channels == 1){
		sharedMem_int[PRU_SHARED_OFFSET+2] = 1;
	}
	else if(num_ai_channels == 2){
		sharedMem_int[PRU_SHARED_OFFSET+2] = 2;
	}
	else if(num_ai_channels == 3){
		sharedMem_int[PRU_SHARED_OFFSET+2] = 3;
	}
	else if(num_ai_channels == 4){
		sharedMem_int[PRU_SHARED_OFFSET+2] = 4;
	}

	// Set frequency
	sharedMem_int[PRU_SHARED_OFFSET+3] = sampling_freq;

	// Printout configuration
	printf("System configuration is:\n #AI: %d\n #AO: %d\n Sampling Frequency: %d\n", sharedMem_int[PRU_SHARED_OFFSET+1], sharedMem_int[PRU_SHARED_OFFSET+2], sharedMem_int[PRU_SHARED_OFFSET+3]);

	// Write DRAM base addr into PRU memory
	info->pru_params->ddr_base_address = info->ddr_base_address;

	// Write # bytes available
	info->pru_params->ddr_bytes_available = info->ddr_size;

	printf("Initialization complete.\n");
	return(0);
}

void deinit(int val) {
	prussdrv_pru_disable(PRU_NUM0);
	prussdrv_pru_disable(PRU_NUM1);
	prussdrv_exit();
	munmap(info.ddr_memory, info.ddr_size);
	close(info.mem_fd);
}

void intHandler(int val) {
	thread_status = (sharedMem_int[PRU_SHARED_OFFSET]==1 ? 0 : 1);
	if(!thread_status) {
		// Disable run/stop bit and reset PRUs
		sharedMem_int[PRU_SHARED_OFFSET] = 0;
		printf("Data acquisition status: stopped.\n");
	}
	else {
		/* Enable run/stop bit and start PRUs
		 * Generate SPI on PRU1 and Transfer data
		 * from PRU Shared space to User Space on PRU0
		 */
		sharedMem_int[PRU_SHARED_OFFSET] = 1;
		prussdrv_exec_program(PRU_NUM1, "./spiagent.bin");
		prussdrv_exec_program(PRU_NUM0, "./dataxferagent.bin");
		printf("Data acquisition status: started.\n");
	}
}

void* work_thread(void *arg) {
	int i;
	unsigned short int* valp;
	unsigned short int val;
	while(system_status) {
		valp=(unsigned short int*)&sharedMem_int[PRU_SHARED_OFFSET];
		while(sharedMem_int[PRU_SHARED_OFFSET]==1) {
			printf("%d\n", *valp);
			valp++;
			valp++;
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {

	if(argc != 4){
		printf("Please enter following arguments to execute Puggle: #AI #AO Fs\n");
		return -1;
	}
	else {
		num_ai_channels = atoi(argv[1]);
		num_ao_channels = atoi(argv[2]);
		sampling_freq = atoi(argv[3]);
		system_status = 1;
	}

	unsigned int ret;
	pthread_t tid;

	printf("Starting PuggleDriver.\n");

	// Initialize data
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

	// Initialize the PRU
	prussdrv_init();

	// Open PRU Interrupts
	ret = prussdrv_open(PRU_EVTOUT_0);
	if(ret) {
		printf("Error: prussdrv_open open failed.\n");
		return (ret);
	}

	ret = prussdrv_open(PRU_EVTOUT_1);
	if(ret) {
		printf("Error: prussdrv_open open failed.\n");
		return (ret);
	}

	// Initialize interrupts
	prussdrv_pruintc_init(&pruss_intc_initdata);

	// Initialize memory settings
	init(&info);

	// Initialize flags and controller for start/stop of PRUs
	signal(SIGINT, intHandler);

	// Create worker thread
	pthread_create(&tid, NULL, &work_thread, NULL);

	// Run Puggle until worker thread is killed
	while(work_thread) {
		int i;
		unsigned short int* valp;
		unsigned short int val;
		while(system_status) {
			valp=(unsigned short int*)&sharedMem_int[PRU_SHARED_OFFSET+2];
			while(sharedMem_int[PRU_SHARED_OFFSET]==0) {
				//val=*valp;
				//printf("%d\n", val);
				//valp++;
				//valp++;
			}
		}
	}

	// Wait until PRU1 has finished execution
	prussdrv_pru_wait_event(PRU_EVTOUT_1);
	//printf("SPIAgent complete.\n");

	// Wait until PRU0 has finished execution
	prussdrv_pru_wait_event(PRU_EVTOUT_0);
	//printf("DataXferAgent complete.\n");

	// clear pru interrupts
	prussdrv_pru_clear_event(PRU1_ARM_INTERRUPT);
	prussdrv_pru_clear_event(PRU0_ARM_INTERRUPT);

	// Deinitialize
	deinit(&info);
	return(0);
}
