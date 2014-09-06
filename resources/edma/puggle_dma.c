/*
	 -------------------------------------------------------------------------

	 This file is part of the Puggle Data Conversion and Processing System
	 Copyright (C) 2013 Puggle

	 -------------------------------------------------------------------------

	 Written in 2013 by: Yogi Patel <yapatel@gatech.edu

	 Parts of this code are modified from the TI EDMA sample application
	 and also from code developed by Terrence McGuckin <terrence@ephemeron-labs.com>
	 and Andrew Righter <q@crypto.com> from Ephemeron Labs

	 To the extent possible under law, the author(s) have dedicated all copyright
	 and related and neighboring rights to this software to the public domain
	 worldwide. This software is distributed without any warranty.

	 You should have received a copy of the CC Public Domain Dedication along with
	 this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
	 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/moduleparam.h>
#include <linux/sysctl.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/kfifo.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/uaccess.h>
#include <linux/platform_data/edma.h>
#include <linux/memory.h>
#include <linux/kdev_t.h>

//#undef EDMA3_DEBUG
#define EDMA3_DEBUG

#ifdef EDMA3_DEBUG
#define DMA_PRINTK(ARGS...)  printk(KERN_INFO "<%s>: ",__FUNCTION__);printk(ARGS)
#define DMA_FN_IN printk(KERN_INFO "[%s]: start\n", __FUNCTION__)
#define DMA_FN_OUT printk(KERN_INFO "[%s]: end\n",__FUNCTION__)
#else
#define DMA_PRINTK( x... )
#define DMA_FN_IN
#define DMA_FN_OUT
#endif

#define MAX_DMA_TRANSFER_IN_BYTES   (32768)
#define STATIC_SHIFT                3
#define TCINTEN_SHIFT               20
#define ITCINTEN_SHIFT              21
#define TCCHEN_SHIFT                22
#define ITCCHEN_SHIFT               23

#define FIFO_SIZE										0x100000
#define MCSPI0_FIFO_ADDR						0x480301a0 // McSPI0 DMA Aligned FIFO
#define MCSPI1_FIFO_ADDR 						0x481a01a0 // McSPI1 DMA Algined FIFO
#define BUF_HEADER									0xffffffff
#define BUF_HEADER_TAIL							0x00ffffff

#define PUGGLE_START		0x1
#define PUGGLE_STOP			0x0

static volatile int irqraised1 = 0;
static volatile int irqraised2 = 0;

/* Function prototypes */
int edma3_memtomemcpytest_dma_link(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue);
int edma3_fifotomemcpytest_dma_link(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue);
static int stop_ppbuffer(int *ch, int *slot1, int *slot2);

struct  puggle_cnt{
	unsigned int bcnt;
	unsigned int ccnt;
};

int ch = 17;
int slot1;
int slot2;
int *ch_ptr = &ch;
int *slot1_ptr = &slot1;
int *slot2_ptr = &slot2;

/* Transfer counter */
unsigned int transfer_counter = 0;
int ping = 1;
int ping_counter = 0;
int pong_counter = 0;
int buf_header = BUF_HEADER;
int buf_header_tail = BUF_HEADER_TAIL;

/* Variable declarations */
dev_t rev = MKDEV(1, 0);
int ack = 0;

dma_addr_t dmaphyssrc1 = 0;
dma_addr_t dmaphyssrc2 = 0;
dma_addr_t dmaphysdest1 = 0;
dma_addr_t dmaphysdest2 = 0;

char *dmabufsrc1 = NULL;
char *dmabufsrc2 = NULL;
char *dmabufdest1 = NULL;
char *dmabufdest2 = NULL;

dma_addr_t dmaphysping = 0;
dma_addr_t dmaphyspong = 0;

int cirbuff = 0;
static DEFINE_KFIFO(test, int, FIFO_SIZE);

int *dmabufping = NULL;
int *dmabufpong = NULL;

static int acnt = 4;
static int bcnt = 8;
static int ccnt = 8;

module_param(acnt, int, S_IRUGO);
module_param(bcnt, int, S_IRUGO);
module_param(ccnt, int, S_IRUGO);

dev_t puggle_dev;
unsigned int puggle_first_minor = 0;
unsigned int puggle_count = 1;
struct cdev cdev;
struct cdev *puggle_cdev = &cdev;
struct puggle_cnt puggle;

struct file_operations puggle_fops;
struct stop_ppbuffer;

/* Begin File Operations for Communication with Userland */
static int puggle_open(struct inode *inode, struct file *file) {
	DMA_PRINTK("Puggle: Open successful");
	return 0;
}

static int puggle_release(struct inode *inode, struct file *file) {
	DMA_PRINTK("Puggle: Release successful");
	return 0;
}

static ssize_t puggle_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	//Returns number requested otherwise sends back the amount of data that is in the fifo
	int ret ;
	unsigned int copied;
	unsigned int length;

	//divide by 4 is neccassry to get the right number of ints in the array
	if (kfifo_len(&test) >= (count >> 2) ) {
		ret = kfifo_to_user(&test, buf, count, &copied);
	}
	else if(kfifo_len(&test) > 0 ) {
		length = kfifo_len(&test) << 2; // need to convert to number of chars in fifo
		ret = kfifo_to_user(&test, buf, length, &copied);
	}
	else
		ret = -1;
	return ret ? ret : copied;
}

static ssize_t puggle_write(struct file *file, const char *buf, size_t count, loff_t * ppos) {
	DMA_PRINTK("Puggle: Accepting zero bytes");
	return 0;
}

static long puggle_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int result = 0;
	int ret = 0;
	void __user *argp = (void __user *)arg;
	DMA_PRINTK("Puggle: cmd=%d, arg=%ld", cmd, arg);

	switch (cmd) {
		case PUGGLE_STOP:
			DMA_PRINTK("Puggle: Stopping Puggle DMA");
			stop_ppbuffer(&ch, &slot1, &slot2);
			ack = 0;
			break;
		case PUGGLE_START:
			// Make sure we stop channel first before we allocate otherwise may fail
			stop_ppbuffer(&ch, &slot1, &slot2);
			ret = copy_from_user(&puggle, argp, sizeof(puggle));
			DMA_PRINTK("Puggle: Starting Puggle DMA %d BCNT %d CCNT", puggle.bcnt, puggle.ccnt);
			acnt = 4;
			bcnt = puggle.bcnt;
			ccnt = puggle.ccnt;
			result = edma3_fifotomemcpytest_dma_link(acnt,bcnt,ccnt,1,1);
			ack = 1;

			//Reset transfer counters
			transfer_counter = 0;
			ping_counter = 0;
			pong_counter = 0;
			break;
		default:
			return -1;
	}
	return 0;
}

// Register Char Device
static int setup_puggle_dev(void){
	if (alloc_chrdev_region(&puggle_dev, puggle_first_minor, 1, "puggle_dma") < 0) {
		DMA_PRINTK(KERN_ERR "Puggle: unable to find free device numbers");
		return -EIO;
	}

	cdev_init(puggle_cdev, &puggle_fops);

	if (cdev_add(puggle_cdev, puggle_dev, 1) < 0) {
		DMA_PRINTK(KERN_ERR "Puggle: Broken: unable to add a character device");
		unregister_chrdev_region(puggle_dev, puggle_count);
		return -EIO;
	}

	DMA_PRINTK(KERN_INFO "Puggle: Loaded the puggle driver: major = %d, minor = %d", 
			MAJOR(puggle_dev), MINOR(puggle_dev));

	return 0;
}

/* Required init function for all kernel modules */
static int __init puggle_init(void) {
	int result = 0;
	int iterations = 0;
	int numTCs = 3;
	int modes = 2;
	int i,j;
	int registered;

	DMA_PRINTK("Puggle: Initializing Puggle DMA module");

	registered = setup_puggle_dev();
	if (registered < 0) {
		DMA_PRINTK("Puggle: Error registering puggle device");
	}

	DMA_PRINTK("Puggle: Registered module successfully!");

	DMA_PRINTK("Puggle: ACNT=%d, BCNT=%d, CCNT=%d", acnt, bcnt, ccnt);

	/* allocate consistent memory for DMA
	 * dmaphyssrc1(handle)= device viewed address.
	 * dmabufsrc1 = CPU-viewed address
	 */

	dmabufsrc1 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES,	&dmaphyssrc1, 0);
	DMA_PRINTK("Puggle: SRC1:\t%x", dmaphyssrc1);
	if (!dmabufsrc1) {
		DMA_PRINTK("Puggle: dma_alloc_coherent failed for dmaphyssrc1");
		return -ENOMEM;
	}

	dmabufdest1 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES, &dmaphysdest1, 0);
	DMA_PRINTK("Puggle: DST1:\t%x", dmaphysdest1);
	if (!dmabufdest1) {
		DMA_PRINTK("Puggle: dma_alloc_coherent failed for dmaphysdest1");
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
		return -ENOMEM;
	}

	dmabufsrc2 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES,	&dmaphyssrc2, 0);
	DMA_PRINTK("Puggle: SRC2:\t%x", dmaphyssrc2);
	if (!dmabufsrc2) {
		DMA_PRINTK("Puggle: dma_alloc_coherent failed for dmaphyssrc2");

		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest1,	dmaphysdest1);
		return -ENOMEM;
	}

	dmabufdest2 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES, &dmaphysdest2, 0);
	DMA_PRINTK("Puggle: DST2:\t%x", dmaphysdest2);
	if (!dmabufdest2) {
		DMA_PRINTK("Puggle: dma_alloc_coherent failed for dmaphysdest2");

		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest1, dmaphysdest1);
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc2, dmaphyssrc2);
		return -ENOMEM;
	}

	dmabufping = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES, &dmaphysping, 0);
	DMA_PRINTK("Puggle: DST1:\t%x", dmaphysping);
	if (!dmabufping) {
		DMA_PRINTK("dma_alloc_coherent failed for dmaphysdping");
		//dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufping, dmaphysping);
		return -ENOMEM;
	}

	dmabufpong = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES, &dmaphyspong, 0);
	DMA_PRINTK("Puggle: DST1:\t%x", dmaphyspong);
	if (!dmabufpong) {
		DMA_PRINTK("Puggle: dma_alloc_coherent failed for dmaphysdest1");
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufping, dmaphysping);
		return -ENOMEM;
	}

	// Test routine
	for (iterations = 0 ; iterations < 10 ; iterations++) {
		DMA_PRINTK("Puggle: Iteration = %d", iterations);
		for (j = 0 ; j < numTCs ; j++) { //TC
			DMA_PRINTK("Puggle: TC = %d", j);
			for (i = 0 ; i < modes ; i++) {	//sync_mode
				DMA_PRINTK("Puggle: Mode = %d", i);
				if (0 == result) {
					DMA_PRINTK("Puggle: Starting edma3_fifotomemcpytest_dma_link");
					result = edma3_fifotomemcpytest_dma_link(acnt, bcnt, ccnt, i, j);
					if (0 == result) {
						DMA_PRINTK("Puggle: edma3_fifotomemcpytest_dma_link passed");
					} else {
						DMA_PRINTK("Puggle: edma3_fifotomemcpytest_dma_link failed");
					}
				}
			}
		}
	}
	return result;
}

static void callback1(unsigned lch, u16 ch_status, void *data) {
	switch(ch_status) {
		case DMA_COMPLETE:
			irqraised1 = 1;
			DMA_PRINTK("Puggle: From Callback 1: Channel %d status is: %u",lch, ch_status);
			break;										
		case DMA_CC_ERROR:
			irqraised1 = -1;
			DMA_PRINTK("Puggle: From Callback 1: DMA_CC_ERROR occured on Channel %d", lch);
			break;
		default:
			break;
	}
}

static void callback_pingpong(unsigned lch, u16 ch_status, void *data) {
	switch(ch_status) {
		case DMA_COMPLETE:
			irqraised1 = 1;
			DMA_PRINTK("Puggle: From Callback PingPong: Channel %d status is: %u",lch, ch_status); 
			// TODO use callabck put data into proper buffer, incrment a counter etc...
			++transfer_counter;
			if(ping == 1){
				++ping_counter;
				DMA_PRINTK("Puggle: Transfer from Ping: ping_counter is %d transfer_counter is: %d", ping_counter, transfer_counter); 

				// Transfer to circular buffer
				cirbuff =  kfifo_len(&test);
				DMA_PRINTK("Puggle: puggle_dma cirbuff len is %d", cirbuff);

				// Add headers/counters to buffer
				kfifo_in(&test, &buf_header, 1); 
				kfifo_in(&test, &transfer_counter, 1);
				kfifo_in(&test, &ping_counter, 1);
				kfifo_in(&test, &buf_header_tail, 1); 

				// put the data in kfifo
				kfifo_in(&test, dmabufping, bcnt*ccnt);

				cirbuff =  kfifo_len(&test);
				DMA_PRINTK("Puggle: puggle_dma cirbuff len is %d", cirbuff);

				ping = 0;
				break;
			}
			else if(ping == 0){
				++pong_counter;
				DMA_PRINTK("Puggle: Transfer from Pong: pong_counter is %d transfer_counter is: %d", pong_counter, transfer_counter);

				// Transfer to circular buffer
				cirbuff =  kfifo_len(&test);
				DMA_PRINTK("Puggle: puggle_dma cirbuff len is %d", cirbuff);

				// Add headers/counters to buffer
				kfifo_in(&test, &buf_header, 1); 
				kfifo_in(&test, &transfer_counter, 1);
				kfifo_in(&test, &pong_counter, 1);
				kfifo_in(&test, &buf_header_tail, 1); 

				// put the data in kfifo
				kfifo_in(&test, dmabufpong, bcnt*ccnt);

				cirbuff =  kfifo_len(&test);
				DMA_PRINTK("Puggle: puggle_dma cirbuff len is %d", cirbuff);

				ping = 1;
				break;
			}
			else	
				break;										
		case DMA_CC_ERROR:
			irqraised1 = -1;
			DMA_PRINTK ("From Callback PingPong : DMA_CC_ERROR occured on Channel %d", lch);
			break;
		default:
			break;
	}
}
/* Required exit function for all kernel modules */
void puggle_exit(void) {
	stop_ppbuffer(&ch, &slot1, &slot2);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest1, dmaphysdest1);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc2, dmaphyssrc2);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest2, dmaphysdest2);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufping, dmaphysping);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufpong, dmaphyspong);
	DMA_PRINTK("Puggle: Unregistering Driver");

	cdev_del(puggle_cdev);
	unregister_chrdev_region(puggle_dev, puggle_count);
	DMA_PRINTK(KERN_INFO "Puggle: Unloaded the Puggle driver!");
	DMA_PRINTK("Puggle: Exiting edma_test module");
}

/* 2 DMA Channels Linked, Mem-2-Mem Copy, ASYNC Mode, INCR Mode */
int edma3_memtomemcpytest_dma_link(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue) {
	int result = 0;
	unsigned int dma_ch1 = 0;
	unsigned int dma_ch2 = 0;
	int i;
	int count = 0;
	unsigned int Istestpassed1 = 0u;
	unsigned int Istestpassed2 = 0u;
	unsigned int numenabled = 0;
	unsigned int BRCnt = 0;
	int srcbidx = 0;
	int desbidx = 0;
	int srccidx = 0;
	int descidx = 0;
	struct edmacc_param param_set;

	/* Initalize source and destination buffers */
	for (count = 0u; count < (acnt*bcnt*ccnt); count++) {
		dmabufsrc1[count] = 'A' + (count % 26);
		dmabufdest1[count] = 0;
		dmabufsrc2[count] = 'B' + (count % 26);
		dmabufdest2[count] = 0;
	}
	/* Set B count reload as B count. */
	BRCnt = bcnt;

	/* Setting up the SRC/DES Index */
	srcbidx = acnt;
	desbidx = acnt;

	/* AB Sync Transfer Mode */
	srccidx = bcnt * acnt;
	descidx = bcnt * acnt;

	result = edma_alloc_channel (EDMA_CHANNEL_ANY, callback1, NULL, event_queue);
	if (result < 0) {
		DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link::edma_alloc_channel failed for dma_ch1, error:%d", result);
		return result;
	}
	dma_ch1 = result;

	// Setup channel 1
	edma_set_src (dma_ch1, (unsigned long)(dmaphyssrc1), INCR, W32BIT);
	edma_set_dest (dma_ch1, (unsigned long)(dmaphysdest1), INCR, W32BIT);
	edma_set_src_index (dma_ch1, srcbidx, srccidx);
	edma_set_dest_index (dma_ch1, desbidx, descidx);
	edma_set_transfer_params (dma_ch1, acnt, bcnt, ccnt, BRCnt, ABSYNC);

	// Enable channel 1 interrupts 
	edma_read_slot (dma_ch1, &param_set);
	param_set.opt |= (1 << ITCINTEN_SHIFT);
	param_set.opt |= (1 << TCINTEN_SHIFT);
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(dma_ch1));
	edma_write_slot(dma_ch1, &param_set);

	// Request a Link Channel
	result = edma_alloc_slot (0, EDMA_SLOT_ANY);
	if (result < 0) {
		DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link::edma_alloc_slot failed for dma_ch2, error:%d", result);
		return result;
	}
	dma_ch2 = result;

	// Setup channel 2
	edma_set_src (dma_ch2, (unsigned long)(dmaphyssrc2), INCR, W32BIT);
	edma_set_dest (dma_ch2, (unsigned long)(dmaphysdest2), INCR, W32BIT);
	edma_set_src_index (dma_ch2, srcbidx, srccidx);
	edma_set_dest_index (dma_ch2, desbidx, descidx);
	edma_set_transfer_params (dma_ch2, acnt, bcnt, ccnt, BRCnt, ABSYNC);

	// Enable the Interrupts on Channel 2
	edma_read_slot (dma_ch2, &param_set);
	param_set.opt |= (1 << ITCINTEN_SHIFT);
	param_set.opt |= (1 << TCINTEN_SHIFT);
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(dma_ch1));
	edma_write_slot(dma_ch2, &param_set);

	// Link both the channels
	edma_link(dma_ch1, dma_ch2);

	//numenabled = bcnt * ccnt; /* For A Sync Transfer Mode */
	numenabled = ccnt;	/* For AB Sync Transfer Mode */

	for (i = 0; i < numenabled; i++) {
		irqraised1 = 0;
		/*
		 * Now enable the transfer as many times as calculated above.
		 */
		result = edma_start(dma_ch1);
		if (result != 0) {
			DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link: davinci_start_dma failed");
			break;
		}

		/* Wait for the Completion ISR. */
		while (irqraised1 == 0u);
		/* Check the status of the completed transfer */
		if (irqraised1 < 0) {
			/* Some error occured, break from the FOR loop. */
			DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link: Event Miss Occured!!!");
			break;
		}
	}

	if (result == 0) {
		for (i = 0; i < numenabled; i++) {
			irqraised1 = 0;
			/*
			 * Now enable the transfer as many times as calculated above
			 * on the LINK channel.
			 */
			result = edma_start(dma_ch1);
			if (result != 0) {
				DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link: davinci_start_dma failed");
				break;
			}
			/* Wait for the Completion ISR. */
			while (irqraised1 == 0u);
			/* Check the status of the completed transfer */
			if (irqraised1 < 0) {
				/* Some error occured, break from the FOR loop. */
				DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link: Event Miss Occured!!!");
				break;
			}
		}
	}

	if (0 == result) {
		for (i = 0; i < (acnt*bcnt*ccnt); i++) {
			if (dmabufsrc1[i] != dmabufdest1[i]) {
				DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link(1): Data "
						"write-read matching failed at = %u",i);
				Istestpassed1 = 0u;
				break;
			}
		}
		if (i == (acnt*bcnt*ccnt)) {
			Istestpassed1 = 1u;
		}
		for (i = 0; i < (acnt*bcnt*ccnt); i++) {
			if (dmabufsrc2[i] != dmabufdest2[i]) {
				DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link(2): Data "
						"write-read matching failed at = %u",i);
				Istestpassed2 = 0u;
				break;
			}
		}
		if (i == (acnt*bcnt*ccnt)) {
			Istestpassed2 = 1u;
		}
		edma_stop(dma_ch1);
		edma_free_channel(dma_ch1);
		edma_stop(dma_ch2);
		edma_free_slot(dma_ch2);
		edma_free_channel(dma_ch2);
	}

	if ((Istestpassed1 == 1u) && (Istestpassed2 == 1u)) {
		DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link: EDMA Data Transfer Successful");
	} else {
		DMA_PRINTK("Puggle: edma3_memtomemcpytest_dma_link: EDMA Data Transfer Failed");
	}

	return result;
}

/* 2 DMA channels linked to each other, McSPI FIFO-2-Mem Copy, ABSYNC Mode, FIFO Mode, Ping Pong buffering scheme */
int edma3_fifotomemcpytest_dma_link(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue) {
	int result = 0;
	unsigned int dma_ch1 = 0;
	unsigned int dma_slot1 = 0;
	unsigned int dma_slot2 = 0;
	int count = 0;
	unsigned int BRCnt = 0;
	int srcbidx = 0;
	int desbidx = 0;
	int srccidx = 0;
	int descidx = 0;
	struct edmacc_param param_set;

	// Initalize source and destination buffers
	for (count = 0u; count < (bcnt*ccnt); count++) {
		dmabufping[count] = 0;
		dmabufpong[count] = 0;
	}

	// Set B count reload as B count.
	BRCnt = bcnt; //changed from bcnt may not affect anything

	// Setting up the SRC/DES Index
	srcbidx = 0;
	desbidx = acnt;

	// AB Sync Transfer Mode
	srccidx = 0;
	descidx = bcnt * acnt;

	// GRAB all the channels and slots we need for linking ping pong buffers and circling them
	result = edma_alloc_channel(ch, callback_pingpong, NULL, event_queue);
	if (result < 0) {
		DMA_PRINTK("Puggle: edma3_fifotomemcpytest_dma_link::edma_alloc_channel failed for dma_ch1, error:%d", result);
		return result;
	}
	dma_ch1 = result;
	*ch_ptr = result; //Make sure we keep the same info on the channel

	// Request a Link Channel for slot1
	result = edma_alloc_slot(0, EDMA_SLOT_ANY);
	if (result < 0) {
		DMA_PRINTK("Puggle: edma3_fifotomemcpytest_dma_link::edma_alloc_slot failed for slot 1, error:%d", result);
		return result;
	}
	dma_slot1 = result;
	*slot1_ptr = dma_slot1;

	// Request a Link Channel for slot2
	result = edma_alloc_slot (0, EDMA_SLOT_ANY);
	if (result < 0) {
		DMA_PRINTK("Puggle: edma3_fifotomemcpytest_dma_link::edma_alloc_slot failed for slot 2, error:%d", result);
		return result;
	}
	dma_slot2 = result;
	*slot2_ptr = dma_slot2;

	// Link the channel and the two slots needed for continous operation
	edma_link(dma_ch1, dma_slot2);   // channel reloads param from slot2 goes to pong
	edma_link(dma_slot2, dma_slot1); // param in slot2 will reload from slot1
	edma_link(dma_slot1, dma_slot2); // slot1 will reload from slot2

	// Setup channel 1
	edma_set_src (dma_ch1, (unsigned long long)MCSPI0_FIFO_ADDR, FIFO, W32BIT);
	edma_set_dest (dma_ch1, (unsigned long)(dmaphysping), INCR, W32BIT);
	edma_set_src_index (dma_ch1, srcbidx, srccidx);
	edma_set_dest_index (dma_ch1, desbidx, descidx);
	edma_set_transfer_params (dma_ch1, acnt, bcnt, ccnt, BRCnt, ABSYNC);

	// Enable channel 1 interrupts
	edma_read_slot (dma_ch1, &param_set);
	//param_set.opt &= ~(1 << ITCINTEN_SHIFT | 1 << STATIC_SHIFT | 1 << TCCHEN_SHIFT | 1 << ITCCHEN_SHIFT );  
	param_set.opt |= (1 << TCINTEN_SHIFT); // | 1 << TCCHEN_SHIFT);
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(dma_ch1));
	edma_write_slot(dma_ch1, &param_set);
	DMA_PRINTK("Puggle: opt for ch %u", param_set.opt); 

	// Test to see if we can get CCNT to remain the same
	edma_set_src (dma_slot1, (unsigned long long)MCSPI0_FIFO_ADDR, FIFO, W32BIT);
	edma_set_dest (dma_slot1, (unsigned long)(dmaphysping), INCR, W32BIT);
	edma_set_src_index (dma_slot1, srcbidx, srccidx);
	edma_set_dest_index (dma_slot1, desbidx, descidx);
	edma_set_transfer_params (dma_slot1, acnt, bcnt, ccnt, BRCnt, ABSYNC);
	edma_write_slot(dma_slot1, &param_set);

	// Setup channel 1
	edma_set_src (dma_slot2, (unsigned long long)MCSPI0_FIFO_ADDR, FIFO, W32BIT);
	edma_set_dest (dma_slot2, (unsigned long)(dmaphyspong), INCR, W32BIT);
	edma_set_src_index (dma_slot2, srcbidx, srccidx);
	edma_set_dest_index (dma_slot2, desbidx, descidx);
	edma_set_transfer_params (dma_slot2, acnt, bcnt, ccnt, BRCnt, ABSYNC);

	// Enable channel 2 interrupts
	edma_read_slot (dma_slot2, &param_set);
	//param_set.opt &= ~(1 << ITCINTEN_SHIFT | 1 << STATIC_SHIFT | 1 << TCINTEN_SHIFT | 1 << ITCCHEN_SHIFT);  //Same here TM
	param_set.opt |= (1 << TCINTEN_SHIFT); // | 1 << TCCHEN_SHIFT);
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(dma_ch1)); // This May be key
	edma_write_slot(dma_slot2, &param_set);
	DMA_PRINTK("Puggle: opt for link %u slot %u",dma_slot2, param_set.opt); 

	result = edma_start(dma_ch1);
	if (result != 0) {
		DMA_PRINTK("Puggle: edma3_fifotomemcpytest_dma_link: davinci_start_dma failed");
	}

	return result;
}

/* Stop ping/pong buffer and free DMA channels */
static int stop_ppbuffer(int *ch, int *slot1, int *slot2) {
	edma_stop(*ch);
	edma_free_channel(*ch);
	edma_free_slot(*slot1);
	edma_free_slot(*slot2);
	DMA_PRINTK("Puggle: DMA channels stopped and freed");
	return 0;
}

/*  Interface options for Userland to communicate with this module */
struct file_operations puggle_fops = {
owner: THIS_MODULE,
			 read: puggle_read,
			 write: puggle_write,
			 unlocked_ioctl: puggle_ioctl,
			 open: puggle_open,
			 release: puggle_release,
};

module_init(puggle_init);
module_exit(puggle_exit);

MODULE_AUTHOR("Yogi Patel <yapatel@gatech.edu>");
MODULE_DESCRIPTION("Puggle EDMA3 Driver");
MODULE_LICENSE("Creative Commons 3.0");
