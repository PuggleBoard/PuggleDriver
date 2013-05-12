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

#define AM33XX
#define PRU_NUM0 	0
#define PRU_NUM1 	1
#define DEBUG
#define UIO_PRUSS_SYSFS_BASE				"/sys/class/uio/uio0/maps/map1"
#define UIO_PRUSS_DRAM_SIZE			        UIO_PRUSS_SYSFS_BASE "/size"
#define UIO_PRUSS_DRAM_ADDR			        UIO_PRUSS_SYSFS_BASE "/addr"
#define PRU_PAGE_SIZE 2048
#define ALIGN_TO_PAGE_SIZE(x, pagesize)  ((x)-((x)%pagesize))

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
	uint32_t    ddr_pages_available;

	void *ddr_memory;
	void *pru_memory;

	pru_params* pru_params;
	volatile ddr_params* ddr_params;

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
	info->sample_bytes_available = ALIGN_TO_PAGE_SIZE(info->ddr_size-32, PRU_PAGE_SIZE);
	info->ddr_params_location = info->ddr_base_location + info->sample_bytes_available;
	info->ddr_pages_available = info->sample_bytes_available / PRU_PAGE_SIZE;

	printf("dram size: 0x%08lX, addr: 0x%08lX\n", (long unsigned int)info->ddr_size, (long unsigned int)info->ddr_base_location);
	printf("%d bytes of ddr available, ddr size: %d", info->sample_bytes_available, info->ddr_size);

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

	info->pru_params = info->pru_memory;
	uint8_t *ddr = (uint8_t *)info->ddr_memory;

	info->ddr_params = &ddr[info->sample_bytes_available];

	fprintf(stderr, "Zeroing DDR memory\n");

	memset((void *)info->ddr_memory, 0, info->ddr_size);

	fprintf(stderr, "Writing PRU params\n");

	// Set the run flag to 1
	info->pru_params->run_flag = 1;
	// Write DRAM base addr into PRU memory
	info->pru_params->ddr_base_address = info->ddr_base_location;
	// Write # bytes available for samples
	info->pru_params->sample_bytes_available = info->sample_bytes_available;
	// Sample memory info struct offset
	info->pru_params->ddr_params_location = info->ddr_params_location;
	// # of pages
	info->pru_params->ddr_pages_available = info->ddr_pages_available;
	fprintf(stderr, "Init complete\n");
	return(0);
}

void check(ads8331_data *info) {

}

ads8331_data info;

void deinit(ads8331_data *info) {
	prussdrv_pru_disable(PRU_NUM0);
	prussdrv_pru_disable(PRU_NUM1);
	prussdrv_exit();
	munmap(info->ddr_memory, info->ddr_size);
	close(info->mem_fd);
}


void intHandler(int dummy) {
	// Set the run flag to 0
	printf("Setting run flag to 0\n");
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
		uint32_t loc = last_page * PRU_PAGE_SIZE;
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
	uint16_t u;

	// for (i = 0; i < current_pos/2; i++) {
	//     u = b[i] >> 2;
	//     if (u & 0x2000) u |= 0x8000;
	//     b[i] = u;
	// }

	FILE *fp;
	fp = fopen("samples.out", "wb");
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

int main (void)
{
	FILE *fp;
	unsigned int ret;
	pthread_t tid;
	// struct pru_data	pru;

	/* Set ADC SCLK */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return(1);
	}
	fprintf(fp,"36");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio36/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);

	/* Set ADC CS */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return (1);
	}
	fprintf(fp,"33");
	fclose(fp);
	
	if((fp=fopen("/sys/class/gpio/gpio33/direction", "w"))==NULL){
		printf("cannot open gpio direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);

	/* Set ADC SDI */
	if ((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
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

	/* Set ADC SDO */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
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

	/* Set ADC CNV */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return (1);
	}
	fprintf(fp,"37");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio37/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);

	/* set DAC SCLK */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return (1);
	}
	fprintf(fp,"47");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio47/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);

	/* set DAC CS */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return (1);
	}
	fprintf(fp,"46");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio46/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);

	/* set DAC SDI */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return (1);
	}
	fprintf(fp,"44");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio44/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"out");
	fclose(fp);

	/* set DAC SDO */
	if((fp=fopen("/sys/class/gpio/export", "w"))==NULL){
		printf("Cannot open GPIO file.\n");
		return (1);
	}
	fprintf(fp,"45");
	fclose(fp);

	if((fp=fopen("/sys/class/gpio/gpio45/direction", "w"))==NULL){
		printf("Cannot open GPIO direction file.\n");
		return(1);
	}
	fprintf(fp,"in");
	fclose(fp);

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

	/* Open PRU Interrupt */
	ret = prussdrv_open(PRU_EVTOUT_1);
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

	//	pthread_create(&tid, NULL, &consumer, NULL);
	pthread_create(&tid, NULL, &rt_print_consumer, NULL);

	/* Execute example on PRU */
	printf("\tINFO: Executing PRU1\r\n");
	prussdrv_exec_program (PRU_NUM1, "./PRU1.bin");
	printf("\t\tINFO: Executing PRU0\r\n");
	prussdrv_exec_program (PRU_NUM0, "./PRU0.bin");

	/* Wait until PRU1 has finished execution */
	printf("\t\tINFO: Waiting for HALT command.\r\n");
	prussdrv_pru_wait_event (PRU_EVTOUT_1);
	printf("\t\tINFO: PRU1 completed transfer.\r\n");
	prussdrv_pru_clear_event (PRU1_ARM_INTERRUPT);

	printf("Waiting for consumer to finish");
	while(consumer_running) {
		sleepms(250);
	}

	/* Wait until PRU0 has finished execution */
	printf("\tINFO: Waiting for HALT command.\r\n");
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	printf("\tINFO: PRU completed transfer.\r\n");
	prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);
	printf("CHECK\r\n");
	check(&info);
	printf("deinit\r\n");
	deinit(&info);
	return(0);
}
