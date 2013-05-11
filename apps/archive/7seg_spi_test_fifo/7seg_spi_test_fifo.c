#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define DEBUG

#define FIFO_FILE	"/dev/pruss7seg"
#define msecs(i)	(i * 1000)

#define RUN_FLAG_IDX	4 

struct pru_data {
    /* data section - copy to PRU */
    uint32_t *prumem;
};

uint32_t running = 1; /* start running */

static int update_buffer(const char *buf, struct pru_data *pru, int rdsize)
{
    uint32_t *prumem = (uint32_t *) pru->prumem;
    int digit = 0;
    int i;

    for (i = 0; i < rdsize; i++) {
        char chr = buf[i];
        if ((chr >= '0' && chr <='9') || (chr >= 'A' && chr <= 'F')) {
            prumem[digit] = chr;        
            printf("Setting %d to '%c'\n", digit, chr);
            digit++;
            if (digit > 4) digit = 0;
        }
    }
    
    return 0;
}

static void shutdown_clock(int signo)
{
    running = 0;
}

int main(int argc, char **argv) {
    char buf[32];
    struct pru_data	pru;
    int fd, rdsize;

#ifndef DEBUG
    if (fork() != 0) {
        return 0;
    }
#endif

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("Initializing pruss\n");
    prussdrv_init();
    if (prussdrv_open(PRU_EVTOUT_0)) {
        fprintf(stderr, "Cannot setup PRU_EVTOUT_0.\n");
        return -EINVAL;
    }
    
    printf("Initializing interrupts\n");
    prussdrv_pruintc_init(&pruss_intc_initdata);

    printf("Mapping PRUSS0 RAM\n");
    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void *) &pru.prumem);
    if (pru.prumem == NULL) {
        fprintf(stderr, "Cannot map PRU0 memory buffer.\n");
        return -ENOMEM;
    }


    printf("Creating FIFO\n");
    /* Create the FIFO if it does not exist */
    umask(0);
    mknod(FIFO_FILE, S_IFIFO|0666, 0);

    printf("Opening FIFO\n");
    fd = open(FIFO_FILE, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Cannot open FIFO.\n");
        return -EINVAL;
    }

    pru.prumem[RUN_FLAG_IDX] = 1; /* startup */

    printf("Loading PRU0 program\n");
    prussdrv_exec_program(0, "7seg_spi_test_fifo.bin");

    signal(SIGINT, shutdown_clock);
    signal(SIGTERM, shutdown_clock);

    printf("Entering runloop\n");
    while (running) {	
        if ((rdsize = read(fd, buf, sizeof(buf) - 1)) > 0) {
            update_buffer((const char *) &buf, &pru, rdsize);
        }
        usleep(msecs(10));
    }
    close(fd);

    pru.prumem[RUN_FLAG_IDX] = 0;
    
#ifdef DEBUG
    fprintf(stdout, "Waiting for PRU core to shutdown..\n");
#endif

    prussdrv_pru_wait_event(PRU_EVTOUT_0);
    prussdrv_pru_clear_event(PRU0_ARM_INTERRUPT);

    prussdrv_pru_disable(0);
    prussdrv_exit();

    return 0;
}