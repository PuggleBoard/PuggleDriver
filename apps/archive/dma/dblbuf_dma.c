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
#include "pruss_intc_mapping.h"

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

} ads7945_data;

long int curtimems() {
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    long int res = tv.tv_usec;
    res /= 1000;
    res += tv.tv_sec * 1000;

    return res;
}

void sleepms(int ms) {
    usleep(1000 * ms);
	//nanosleep((struct timespec[]){{0, ms*100000}}, NULL);
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

static int load_pruss_dram_info(ads7945_data *info) {

	info->ddr_size = read_uint32_hex_from_file(UIO_PRUSS_DRAM_SIZE);
	info->ddr_base_location = read_uint32_hex_from_file(UIO_PRUSS_DRAM_ADDR);
    info->sample_bytes_available = ALIGN_TO_PAGE_SIZE(info->ddr_size-32, PRU_PAGE_SIZE);
    info->ddr_params_location = info->ddr_base_location + info->sample_bytes_available;
    info->ddr_pages_available = info->sample_bytes_available / PRU_PAGE_SIZE;

	printf("dram size: 0x%08lX, addr: 0x%08lX\n", (long unsigned int)info->ddr_size, (long unsigned int)info->ddr_base_location);
    printf("%d bytes of ddr available, ddr size: %d", info->sample_bytes_available, info->ddr_size);

	return 0;
}

static int init(ads7945_data *info)
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

void check(ads7945_data *info) {

}

ads7945_data info;

void deinit(ads7945_data *info) {
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
    uint32_t bytes_rx = 0;
    uint32_t total_bytes_rx = 0;

    uint32_t total_bytes_rx_last_period;
    long int  last_time = curtimems();
    long int  cur_time = 0;

    long int speed = 0;
    long int diff = 0;

    int i;
    int capture_file = 1;
    int capture_bytes = 1024 * 1024 * 1;

    while(info.pru_params->run_flag) {
        sleepms(100);
        system("");

        uint32_t last_written_ddr_page = info.ddr_params->last_written_ddr_page;
        uint32_t total_pages_written = info.ddr_params->total_pages_written;

        if (last_written_ddr_page != last_page) {
            int bytes;
            if (last_written_ddr_page > last_page) {
                int pages = last_written_ddr_page - last_page;
                int size = pages * PRU_PAGE_SIZE;
                bytes = size;
            }
            else {
                int pages = info.ddr_pages_available  - last_page - 1;
                int size = pages * PRU_PAGE_SIZE;
                bytes = size;
                pages = last_written_ddr_page;
                size = pages * PRU_PAGE_SIZE;
                bytes += size;
            }
            total_bytes_rx += bytes;
            total_bytes_rx_last_period = bytes;
        }

        last_page = last_written_ddr_page;
        uint16_t *src = (uint16_t *) &ddr[(last_page * PRU_PAGE_SIZE) - 2];
        uint16_t sample = src[0];
        // sample >>= 2;

        cur_time = curtimems();
        diff = cur_time - last_time;

        speed = total_bytes_rx_last_period * 1000 / diff;
        speed /= 1024;

        printf("%d\t%d\t%d\t%ldkb/s\t%ldks/s\t\t0x%x\n", last_page, total_pages_written, total_bytes_rx, speed, speed/2, sample);

                last_time = cur_time;

        if (capture_file && (total_bytes_rx >= capture_bytes)) {
            break;
        }

        fflush(stdout);
    }

    if (capture_file) {
        FILE *fp;
        unlink("samples.raw");
        fp = fopen("samples.raw", "wb");
        for(i = 0; i < total_bytes_rx; i++) {
            fwrite(&ddr[i], 1, 1, fp);
        }
        fclose(fp);
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
    unsigned int ret;
	pthread_t tid;
    // struct pru_data	pru;ls

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\nINFO: Starting %s example.\r\n", "ads7945");
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
    printf("\tINFO: Executing dblbuf_pru1_streamer.bin\r\n");
    prussdrv_exec_program (PRU_NUM1, "./dblbuf_dma_pru1_streamer.bin");
    printf("\t\tINFO: Executing example on PRU1.\r\n");
    prussdrv_exec_program (PRU_NUM0, "./dblbuf_dma_pru0_sampler.bin");

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

