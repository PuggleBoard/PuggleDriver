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
#define UIO_PRUSS_SYSFS_BASE							"/sys/class/uio/uio0/maps/map1"
#define UIO_PRUSS_DRAM_SIZE			    			UIO_PRUSS_SYSFS_BASE "/size"
#define UIO_PRUSS_DRAM_ADDR			    			UIO_PRUSS_SYSFS_BASE "/addr"
#define PRU_PAGE_SIZE 										2048
#define ALIGN_TO_PAGE_SIZE(x, pagesize)  	((x)-((x)%pagesize))

#define DDR_BASEADDR 											0x00080000
#define DDR_RESERVED 											0x00008000
#define DDR_RES_SIZE 											0x00008000
#define DDR_SHIFT													0x00080000

#define PRINTDEC(str,addr)								printf("%s: %d\n",str,addr);
#define PRINTHEX(str,addr)								printf("%s: 0x%08lX\n",str,(long unsigned int)addr);
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct {
	uint32_t    run_flag;
	uint32_t    ddr_base_address;
	uint32_t    sample_bytes_available;
	uint32_t    ddr_params_location;
	uint32_t    ddr_pages_available;
} pru_params;

typedef struct {
	volatile uint32_t last_written_ddr_page;
	volatile uint32_t total_pages_written;
} ddr_params;

typedef struct {
	size_t ddr_size;
	uint32_t ddr_base_location;
	uint32_t ddr_params_location;
	uint32_t sample_bytes_available;
	uint32_t ddr_pages_available;
	void *ddr_memory;
	void *pru_memory;
	pru_params* pru_params;
	volatile ddr_params* ddr_params;
	int mem_fd;
} ads8331_data;

void sleepms(int ms) {
	nanosleep((struct timespec[]) {
			{0, ms*100000}}, NULL);
}

ads8331_data info;

void deinit(ads8331_data *info) {
	printf("Disabling PRUs.\n");
	prussdrv_pru_disable(PRU_NUM0);
	prussdrv_pru_disable(PRU_NUM1);
	prussdrv_exit();

	printf("Unmapping memory.\n");
	munmap(info->ddr_memory, info->ddr_size);
	close(info->mem_fd);
	printf("Goodbye.\n");
}

void intHandler(int dummy) {
	// Set the run flag to 0
	info.pru_params->run_flag = 0;
}

int consumer_running = 0;

void* rt_print_consumer(void *arg) {
	consumer_running = 1;
	uint32_t last_page = 0;
	uint8_t *ddr = info.ddr_memory;
	while(info.pru_params->run_flag) {
		sleepms(1000);
		//        msync(info.ddr_memory, info.ddr_size, MS_SYNC);
		uint32_t last_written_ddr_page = info.ddr_params->last_written_ddr_page;
		uint32_t total_pages_written = info.ddr_params->total_pages_written;

		//        if (last_written_ddr_page != last_page) {
		last_page = last_written_ddr_page;
		//uint32_t loc = last_page * PRU_PAGE_SIZE;
		uint16_t *src = &ddr[(last_page * PRU_PAGE_SIZE) - 2];
		uint16_t sample = src[0];
		sample >>= 2;
		printf("%d/%d: 0x%x\n", last_page, total_pages_written, sample);
		//        }
		//        printf(".");
		fflush(stdout);
	}
	consumer_running = 0;
}

void* consumer(void *arg) {
	consumer_running = 1;
	uint8_t *ddr = info.ddr_memory;
	uint8_t *buffer;
	int last_page = 0;
	int current_pos = 0;
	int buffer_size = 2048 * 1024 * 2;
	int record_size = 1024 * 1024;
	buffer = malloc(buffer_size); // 4mb buffer

	if (!buffer) {
		printf("Error allocating buffer");
		return NULL;
	}

	int pages_written = 0;

	while(info.pru_params->run_flag) {
		sleepms(10);

		uint16_t last_written_ddr_page = info.ddr_params->last_written_ddr_page;

		if (last_written_ddr_page != last_page) {
			// printf("last page: %d, new page: %d\n", last_page, last_written_ddr_page);
			if (last_written_ddr_page > last_page) {
				// We haven't wrapped yet
				int pages = last_written_ddr_page - last_page;
				int size = pages * PRU_PAGE_SIZE;
				void *src = &ddr[last_page * PRU_PAGE_SIZE];
				void *dst = &buffer[current_pos];

				memcpy(dst, src, size);
				current_pos += size;
				pages_written += pages;
			}
			else {
				int pages = info.ddr_pages_available  - last_page - 1;
				int size = pages * PRU_PAGE_SIZE;
				void *src = &ddr[last_page * PRU_PAGE_SIZE];
				void *dst = &buffer[current_pos];

				memcpy(dst, src, size);
				current_pos += size;
				pages_written += pages;
				pages = last_written_ddr_page;
				size = pages * PRU_PAGE_SIZE;

				src = &ddr[0];
				dst = &buffer[current_pos];

				memcpy(dst, src, size);
				current_pos += size;
				pages_written += pages;
			}
			last_page = last_written_ddr_page;

			if (current_pos >= record_size) {
				break;
			}

			if (pages_written > 32) {
				// printf("block\n");
				pages_written = 0;
			}
		}
	}

	int i;
	uint16_t *b = buffer;
	//uint16_t u;

	// for (i = 0; i < current_pos/2; i++) {
	//     u = b[i] >> 2;
	//     if (u & 0x2000) u |= 0x8000;
	//     b[i] = u;
	// }

	FILE *fp;
	fp = fopen("samples.out", "wb");
	printf("File created.\n");
	fwrite(buffer, 1, current_pos, fp);
	fclose(fp);

	for(i = 0; i < 16; i++) {
		uint16_t val = b[i];
		printf("%d: 0x%x\n", i, val);
	}

	free(buffer);
	printf("Exiting consumer thread\n");
	consumer_running = 0;
	return NULL;
}

int mux(char *name, int val) {
	char cmd[1024];
	sprintf(cmd, "echo %x > /sys/kernel/debug/omap_mux/%s", val, name);
	if (system(cmd) != 0) {
		printf("ERROR: Failed to set pin mux %s = %x\n", name, val);
		return -1;
	}
	return 0;
}

int main (void) {

	PRINTDEC("HEXADDR", UIO_PRUSS_DRAM_ADDR);
	PRINTDEC("HEXSIZE", UIO_PRUSS_DRAM_SIZE);

	printf("Starting PuggleDriver.\n");
	
	// Make sure PRU kernel module is running
	system("modprobe uio_pruss");
	printf("PRU kernel module initialized.\n");

	unsigned int ret;
	pthread_t tid;
	// struct pru_data	pru;
	static void *ddrMem = 0;
	static void *pruMem = 0;
	static FILE *fp = 0;
	static int mem_fd;

	// Initialize data
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

	// Initialize the PRU
	prussdrv_init();
	printf("PRUs initialized.\n");

	// Open PRU Interrupt
	ret = prussdrv_open(PRU_EVTOUT_0);
	if(ret) {
		printf("Error: prussdrv_open open failed\n");
		return (ret);
	}

	// Open PRU Interrupt
	ret = prussdrv_open(PRU_EVTOUT_1);
	if(ret) {
		printf("Error: prussdrv_open open failed\n");
		return (ret);
	}
	printf("PRU interrupts opened.\n");

	// Get the interrupt initialized
	prussdrv_pruintc_init(&pruss_intc_initdata);
	printf("PRU interrupts initialized.\n");

	// Initialize PRU and memory
	//init(&info);

	signal(SIGQUIT, intHandler);
	signal(SIGINT, intHandler);
	printf("Flags initialized.\n");

	//pthread_create(&tid, NULL, &consumer, NULL);
	//pthread_create(&tid, NULL, &rt_print_consumer, NULL);
	printf("Consumer thread created.\n");

	// Open device
	mem_fd = open("/dev/mem", O_RDWR);
	if (mem_fd < 0) {
		printf("Failed to open /dev/mem (%s)\n", strerror(errno));
		return -1;
	}
	printf("Device opened.\n");

	// Map the memory
	ddrMem = mmap(0, DDR_RES_SIZE, PROT_WRITE |
			PROT_READ, MAP_SHARED, mem_fd, DDR_RESERVED);
	
	if(ddrMem == MAP_FAILED) {
		handle_error("mmap");
	}

	if (ddrMem == NULL) {
		printf("Failed to map the device (%s)\n", strerror(errno));
		close(mem_fd);
		return -1;
	}
	printf("Memory mapping complete.\n");

	// Set ADC SCLK
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 76.\n");
		return(1);
	}
	fprintf(fp,"76");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio76/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file 76.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_data6",0x0d);

	// Set ADC CS
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 75.\n");
		return (1);
	}
	fprintf(fp,"75");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio75/direction", "w"))==NULL){
		printf("cannot open gpio direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_data5",0x0d);

	// Set ADC SDI
	if ((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 87.\n");
		return (1);
	}
	fprintf(fp,"87");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio87/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_hsync",0x0d);

	// Set ADC SDO
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 77.\n");
		return (1);
	}
	fprintf(fp,"77");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio77/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"in");
	fclose(fp);
	mux("lcd_data7",0x2e);

	// Set ADC CNV
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 86.\n");
		return (1);
	}
	fprintf(fp,"86");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio86/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_vsync",0x0d);

	// Set DAC SCLK
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 89.\n");
		return (1);
	}
	fprintf(fp,"89");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio89/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_ac_bias_en",0x0d);

	// Set DAC CS
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 88.\n");
		return (1);
	}
	fprintf(fp,"88");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio88/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("lcd_pclk",0x0d);

	// Set DAC SDI
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 62.\n");
		return (1);
	}
	fprintf(fp,"62");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio62/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);
	mux("gpmc_csn1",0x0d);

	// Set DAC SDO
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file 63.\n");
		return (1);
	}
	fprintf(fp,"63");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio63/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"in");
	fclose(fp);
	mux("gpmc_csn2",0x2e);

	printf("GPIO initialized.\n");

	// Locate PRU0 memory
	prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pruMem);
	printf("Located PRU0 memory.\n");

	// Execute on PRU1
	prussdrv_exec_program(PRU_NUM1, "./PRU1.bin");
	printf("Executing PRU1.\n");

	//prussdrv_exec_program(PRU_NUM0, "./blink.bin");
	//printf("Executing Blink.\n");

	prussdrv_exec_program(PRU_NUM0, "./PRU0.bin");
	printf("Executing PRU0.\n");

	prussdrv_pru_clear_event(PRU1_ARM_INTERRUPT);
	printf("PRU1 completed transfer.\n");

	printf("Waiting for consumer to finish\n");
	while(consumer_running) {
		sleepms(250);
	}

	// Wait until PRU0 has finished execution
	printf("Waiting for HALT command.\n");
	prussdrv_pru_wait_event(PRU_EVTOUT_0);

	prussdrv_pru_clear_event(PRU0_ARM_INTERRUPT);
	printf("PRU0 completed transfer.\n");

	// Deinitialize everything
	deinit(&info);
	printf("Deinitialization complete.\n");
	return(0);
}
