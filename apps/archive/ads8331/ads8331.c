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

#include "prussdrv.h"
#include <pruss_intc_mapping.h>	 

#define PRU_NUM 	0
#define AM33XX

#define DEBUG

#define UIO_PRUSS_SYSFS_BASE				"/sys/class/uio/uio0/maps/map1"
#define UIO_PRUSS_DRAM_SIZE			        UIO_PRUSS_SYSFS_BASE "/size"
#define UIO_PRUSS_DRAM_ADDR			        UIO_PRUSS_SYSFS_BASE "/addr"

typedef struct {
    size_t ddr_size;
	uint32_t ddr_base_location;
	
	void *ddr_memory;
	void *pru_memory;
	
	int mem_fd;
	
} ads8331_data;

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

static int load_pruss_dram_info(ads8331_data *info) {
	info->ddr_size = read_uint32_hex_from_file(UIO_PRUSS_DRAM_SIZE);
	info->ddr_base_location = read_uint32_hex_from_file(UIO_PRUSS_DRAM_ADDR);
	
	#ifdef DEBUG
	printf("dram size: 0x%08lX, addr: 0x%08lX\n", (long unsigned int)info->ddr_size, (long unsigned int)info->ddr_base_location);
	#endif
	
	return 0;
}

static int init(ads8331_data *info)
{
	
	load_pruss_dram_info(info);
	
    info->mem_fd = open("/dev/mem", O_RDWR);
    if (info->mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }	

    info->ddr_memory = mmap(0, info->ddr_size, PROT_WRITE | PROT_READ, MAP_SHARED, info->mem_fd, info->ddr_base_location);
    if (info->ddr_memory == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(info->mem_fd);
        return -1;
    }
	
    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void *) &info->pru_memory);
	
	if (info->pru_memory == NULL) {
	    fprintf(stderr, "Cannot map PRU0 memory buffer.\n");
	    return -ENOMEM;
	}

	memset((void *)info->ddr_memory, 0, info->ddr_size);
	
	// Write DRAM base addr into PRU memory
	((unsigned long*)info->pru_memory)[0] = (uint32_t)info->ddr_base_location;
	// Write # of samples to collect
	((unsigned long*)info->pru_memory)[1] = (info->ddr_size - 32) / 2; // (16-bits per sample, so divide by 2, 16 bytes reserved
	// Set the run flag to 1
	((unsigned long*)info->pru_memory)[2] = 1;
	
    return(0);
}

void check(ads8331_data *info) {
	
	int i;
	for(i = 0; i < 30; i++) {
		uint16_t val =	((uint16_t*)info->ddr_memory)[i];
		val >>= 1;
		// Extend the sign
		// if (val >= 0x2000) val |= 0xA000;
		// int16_t sval = (int16_t)val;
		// sval += 0x2000;
		printf("%d: 0x%x\n", i, val);
	}
	
	/*
    void *DDR_regaddr1;
    int i;
    for(i = 0; i < 10; i+=4) {
        DDR_regaddr1 = ddrMem + OFFSET_DDR + i;
        unsigned long val = *(unsigned long*) DDR_regaddr1;
        printf("\t%d: %#lx\n", i, val);
    }
	*/
}

ads8331_data info;

unsigned long get_sample_number() {
	int i = (info.ddr_size - 32) / 2;
	void *addr = &((uint16_t*)info.ddr_memory)[i];
	unsigned long val = *((unsigned long*)addr);
	val >>= 1;
	return val;
}

void deinit(ads8331_data *info) {
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();
    munmap(info->ddr_memory, info->ddr_size);
	close(info->mem_fd);
}


void intHandler(int dummy) {
	// Set the run flag to 0
    printf("Setting run flag to 0\n");
	((unsigned long*)info.pru_memory)[2] = 0;
}


void* consumer(void *arg) {
	while(((unsigned long*)info.pru_memory)[2]) {
		sleepms(250);
		unsigned long sample = get_sample_number();
		//printf("%d\n", sample);
		uint16_t val =	((uint16_t*)info.ddr_memory)[sample];
		val >>= 2;
		val &= 0x3FFF;
//		if (val & 0x2000) val |= 0xA000;
		printf("%ld: 0x%x\n", sample, val);
	}
    printf("Exiting consumer thread\n");
	return NULL;
}

int main (void)
{
    unsigned int ret;
	pthread_t tid;
    // struct pru_data	pru;ls
	
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\nINFO: Starting %s example.\r\n", "ads8331");
    /* Initialize the PRU */
    prussdrv_init();		

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    printf("\tINFO: init\r\n");
	
    init(&info);

	signal (SIGQUIT, intHandler);
	signal (SIGINT, intHandler);
	
	pthread_create(&tid, NULL, &consumer, NULL);
	
    /* Execute example on PRU */
    printf("\tINFO: Executing ads8331.bin\r\n");
	prussdrv_exec_program (PRU_NUM, "./ads8331.bin");

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
	prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    check(&info);
    deinit(&info);

    return(0);
}

