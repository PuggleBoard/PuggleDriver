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
#define UIO_PRUSS_SYSFS_BASE	  					"/sys/class/uio/uio0/maps/map1"
#define UIO_PRUSS_DRAM_SIZE	  		        UIO_PRUSS_SYSFS_BASE "/size"
#define UIO_PRUSS_DRAM_ADDR 			        UIO_PRUSS_SYSFS_BASE "/addr"
#define PRU_PAGE_SIZE	 										2048
#define ALIGN_TO_PAGE_SIZE(x, pagesize)  ((x)-((x)%pagesize))

#define DDR_BASEADDR 											0x80000000
#define DDR_RESERVED 											0x00008000
#define DDR_RES_SIZE 											0x0FFFFFFF
#define DDR_SHIFT													0x00001000
#define DDR_OFFSET												4096

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

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

app_data info;

void sleepms(int ms) {
	nanosleep((struct timespec[]){{0, ms*100000}}, NULL);
}

static uint32_t read_uint32_hex_from_file(const char *file) {
	size_t len = 0;
	ssize_t bytes_read;
	char *line;
	uint32_t value = 0;
	FILE *f = fopen(file, "r");
	if (f) {
		bytes_read = getline(&line, &len, f);
		if (bytes_read > 0) {
			value = strtoul(line, NULL, 0);
		}
	}
	if (f) fclose(f);
	if (line) free(line);
	return value;
}

static int load_pruss_dram_info(app_data *info) {
	info->ddr_size = read_uint32_hex_from_file(UIO_PRUSS_DRAM_SIZE);
	info->ddr_base_address = read_uint32_hex_from_file(UIO_PRUSS_DRAM_ADDR);
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

	prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void *) &info->pru_memory);

	if(info->pru_memory == NULL) {
		printf("Cannot map PRU1 memory buffer.\n");
		return -ENOMEM;
	}

	printf("Initializing memory.\n");
	info->pru_params = info->pru_memory;
	memset((void *)info->ddr_memory, 0, info->ddr_size);

	printf("Initializing PRU parameters.\n");
	
	// Set the run flag to 1
	info->pru_params->run_flag = 1;

	// Write DRAM base addr into PRU memory
	info->pru_params->ddr_base_address = info->ddr_base_address;
	
	// Write # bytes available
	info->pru_params->ddr_bytes_available = info->ddr_size;
	
	printf("Initialization complete.\n");
	return(0);
}

void check(app_data *info) {
	int i = 0;
	uint32_t *ddr = info->ddr_memory;
	for (i = 0; i < 10; i++) {
		printf("%i: 0x%X\n", i, ddr[i]);
	}
}

void deinit(app_data *info) {
	prussdrv_pru_disable(PRU_NUM0);
	prussdrv_pru_disable(PRU_NUM1);
	prussdrv_exit();
	munmap(info->ddr_memory, info->ddr_size);
	close(info->mem_fd);
}

void intHandler(int val) {
	info.pru_params->run_flag = 0;
	printf("Data acquisition status: stopped.\n");
}

void* work_thread(void *arg) {
	printf("Data acquisition status: running. Press ctrl-c to stop.\n");
	while(info.pru_params->run_flag) {
	}
	return NULL;
}

int main (void) {

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

	// Initialize flags
	signal(SIGINT, intHandler);

	// Generate SPI on PRU1 and Transfer data
	// from PRU Shared space to User Space on PRU0
	prussdrv_exec_program(PRU_NUM1, "./spiagent.bin");
	prussdrv_exec_program(PRU_NUM0, "./dataxferagent.bin");

	// Create worker thread
	pthread_create(&tid, NULL, &work_thread, NULL);

	while(work_thread) {
		sleepms(250);
	}

	// Wait until PRU1 has finished execution
	prussdrv_pru_wait_event(PRU_EVTOUT_1);
	printf("SPIAgent complete.\n");

	// Wait until PRU0 has finished execution
	prussdrv_pru_wait_event(PRU_EVTOUT_0);
	printf("DataXferAgent complete.\n");
	
	// Clear PRU interrupts
	prussdrv_pru_clear_event(PRU1_ARM_INTERRUPT);
	prussdrv_pru_clear_event(PRU0_ARM_INTERRUPT);

	// Deinitialize
	check(&info);
	deinit(&info);
	return(0);
}
