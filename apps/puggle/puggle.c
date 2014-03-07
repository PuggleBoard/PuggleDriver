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

// System includes
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
#include <sys/resource.h>

// Xenomai native skin inludes
#include <native/task.h>
#include <native/timer.h>

// Custom includes
#include "puggle.h"
#include "debug.h"

#define AM33XX
#define PRU_NUM0 													0
#define PRU_NUM1 													1
#define ALIGN_TO_PAGE_SIZE(x, pagesize)   ((x)-((x)%pagesize))
#define handle_error(msg) 								do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define UIO_PRUSS_SYSFS_BASE        			"/sys/class/uio/uio0/maps/map1"
#define UIO_PRUSS_DRAM_SIZE             	UIO_PRUSS_SYSFS_BASE "/size"
#define UIO_PRUSS_DRAM_ADDR             	UIO_PRUSS_SYSFS_BASE "/addr"
#define PRU_SHARED_OFFSET									0

#define DEBUG															1

typedef struct {
	long long puggle_rt_period;
	RT_TASK puggle_rt_task;
} xenomai_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;

typedef struct {
	uint32_t    run_flag;
	uint32_t    ddr_base_address;
	uint32_t    ddr_bytes_available;
	uint32_t		dac_offset;
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
	//printf("DDR size is %dMB starting at address 0x%08lX\n", (unsigned int)info->ddr_size/1024, info->ddr_base_address);
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
	memset((void *)info->ddr_memory, 0, info->ddr_size);
	info->pru_params = info->pru_memory;

	printf("Initializing PRU parameters.\n");

	// Set the run flag to stop
	info->pru_params->run_flag = 0;

	// Printout configuration
	//sharedMem_int[PRU_SHARED_OFFSET+1], sharedMem_int[PRU_SHARED_OFFSET+2], sharedMem_int[PRU_SHARED_OFFSET+3]);

	// Write DRAM base addr into PRU memory
	info->pru_params->ddr_base_address = info->ddr_base_address;

	// Write # bytes available
	info->pru_params->ddr_bytes_available = info->ddr_size;

	// Write dac offset
	info->pru_params->dac_offset = 0x000000ff;

	printf("Initialization complete.\n");
	return(0);
}

void deinit(int val) {
	prussdrv_pru_disable(PRU_NUM0);
	prussdrv_exit();
	munmap(info.ddr_memory, info.ddr_size);
	close(info.mem_fd);
}

void intHandler(int val) {
	thread_status = (info.pru_params->run_flag == 1 ? 0 : 1);
	if(!thread_status) {
		// Disable run/stop bit and reset PRUs
		info.pru_params->run_flag = 0;
		printf("Data acquisition status: stopped.\n");
	}
	else {
		/* Enable run/stop bit and start PRUs
		 * Generate SPI on PRU1 and Transfer data
		 * from PRU Shared space to User Space on PRU0
		 */
		info.pru_params->run_flag = 1;
		// prussdrv_exec_program(PRU_NUM0, "./puggle.bin");
		prussdrv_exec_program(PRU_NUM0, "./intan.bin");
		printf("Data acquisition status: started.\n");
	}
}

void *module_thread(void *arg) {
	int i = 0;
	int j = 1;
	double val;
	while(system_status) {
		//while(info.pru_params->run_flag == 1) {
			uint32_t *readme = info.ddr_memory;
			uint32_t *ddr = info.ddr_memory+64;
			//printf("%d \n", *readme);
			val = 2.0*4.096*(.00001525902)*(525)-4.096;
			//printf("%f 0x%08lX\n", val, ddr);
			val =  (val+4.096)/(2.0*4.096*0.00001525902);
			*ddr = *readme;
		//}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	printf("Starting PuggleDriver.\n");

	system_status = 1;

	unsigned int retval = 0;
	unsigned int ret;
	pthread_t tid;

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
	//signal(SIGINT, intHandler);

	// Initiate RT thread
	initiate();

	// Create worker thread
	pthread_create(&tid, NULL, &module_thread, NULL);
	/*xenomai_task_t *puggle;
	retval = createTask(&puggle);
	printf("%d\n",retval);*/

	//prussdrv_exec_program(PRU_NUM0, "./puggle.bin");
	prussdrv_exec_program(PRU_NUM0, "./intan.bin");
	printf("Data acquisition status: started.\n");

	// Wait until PRU0 has finished execution
	prussdrv_pru_wait_event(PRU_EVTOUT_0);

	// clear pru interrupts
	prussdrv_pru_clear_event(PRU0_ARM_INTERRUPT);

	// Deinitialize
	deinit(&info);
	return(0);
}
	
int initiate(void) {
	rt_timer_set_mode(TM_ONESHOT);

	// overrie blocking limit on memory
	struct rlimit rlim = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_MEMLOCK,&rlim);

	if(mlockall(MCL_CURRENT | MCL_FUTURE)) {
		ERROR_MSG("Failed to lock memory.\n");
		return -EPERM;
	}

	pthread_key_create(&is_rt_key,0);
	init_rt = true;

	printf("Thread initiation complete.\n");
	return 0;
}

void shutdown(void) {
	pthread_key_delete(is_rt_key);
}

int createTask(Task *task) {
	int resval = 0;
	xenomai_task_t *t = &task;

	if((resval = rt_task_create(&t->puggle_rt_task, "Puggle", 0, 99, T_FPU | T_JOINABLE))) {
		ERROR_MSG("Failed to create task.\n");
		return resval;
	}

	pthread_setspecific(is_rt_key, (const void *)(t));

	if((resval = rt_task_start(&t->puggle_rt_task, &module_thread, 0))) {
		ERROR_MSG("Failed to start task.\n");
		return resval;
	}

	printf("RT thread started.\n");
	return 0;
}

void deleteTask(Task task) {
	xenomai_task_t *t = (xenomai_task_t *)(task);
	rt_task_delete(&t->puggle_rt_task);
}
	
bool isRealtime(void) {
	if(init_rt && pthread_getspecific(is_rt_key))
		return true;
	return false;
}

long long getTime(void) {
	return rt_timer_tsc2ns(rt_timer_tsc());
}

int setPeriod(Task task, long long period) {
	xenomai_task_t *t = (xenomai_task_t *)(task);
	t->puggle_rt_period = period;
	return rt_task_set_periodic(&t->puggle_rt_period, TM_NOW, period);
}

void sleepTimestep(Task task) {
	rt_task_wait_period(0);
}
