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

//#include "puggle.h"

#undef EDMA3_DEBUG
/*#define EDMA3_DEBUG*/

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

static volatile int irqraised1 = 0;
static volatile int irqraised2 = 0;

int edma3_memtomemcpytest_dma(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue);
int edma3_memtomemcpytest_dma_link(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue);
int edma3_memtomemcpytest_dma_chain(int acnt, int bcnt, int ccnt, int sync_mode, int event_queue);

dma_addr_t dmaphyssrc1 = 0;
dma_addr_t dmaphyssrc2 = 0;
dma_addr_t dmaphysdest1 = 0;
dma_addr_t dmaphysdest2 = 0;

char *dmabufsrc1 = NULL;
char *dmabufsrc2 = NULL;
char *dmabufdest1 = NULL;
char *dmabufdest2 = NULL;

static int acnt = 512;
static int bcnt = 8;
static int ccnt = 8;

module_param(acnt, int, S_IRUGO);
module_param(bcnt, int, S_IRUGO);
module_param(ccnt, int, S_IRUGO);

/* Required init function for all kernel modules */
static int __init puggle_init(void) {
	int result = 0;
	int iterations = 0;
	int numTCs = 3;
	int modes = 2;
	int i,j;

	printk ("\nInitializing Puggle DMA module\n");

	DMA_PRINTK ( "\nACNT=%d, BCNT=%d, CCNT=%d", acnt, bcnt, ccnt);

	/* allocate consistent memory for DMA
	 * dmaphyssrc1(handle)= device viewed address.
	 * dmabufsrc1 = CPU-viewed address
	 */

	dmabufsrc1 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES,	&dmaphyssrc1, 0);
	DMA_PRINTK( "\nSRC1:\t%x", dmaphyssrc1);
	if (!dmabufsrc1) {
		DMA_PRINTK ("dma_alloc_coherent failed for dmaphyssrc1\n");
		return -ENOMEM;
	}

	dmabufdest1 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES, &dmaphysdest1, 0);
	DMA_PRINTK( "\nDST1:\t%x", dmaphysdest1);
	if (!dmabufdest1) {
		DMA_PRINTK("dma_alloc_coherent failed for dmaphysdest1\n");
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
		return -ENOMEM;
	}

	dmabufsrc2 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES,	&dmaphyssrc2, 0);
	DMA_PRINTK( "\nSRC2:\t%x", dmaphyssrc2);
	if (!dmabufsrc2) {
		DMA_PRINTK ("dma_alloc_coherent failed for dmaphyssrc2\n");

		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest1,	dmaphysdest1);
		return -ENOMEM;
	}

	dmabufdest2 = dma_alloc_coherent (NULL, MAX_DMA_TRANSFER_IN_BYTES, &dmaphysdest2, 0);
	DMA_PRINTK( "\nDST2:\t%x", dmaphysdest2);
	if (!dmabufdest2) {
		DMA_PRINTK ("dma_alloc_coherent failed for dmaphysdest2\n");

		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest1, dmaphysdest1);
		dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc2, dmaphyssrc2);
		return -ENOMEM;
	}

	for (iterations = 0 ; iterations < 10 ; iterations++) {
		DMA_PRINTK ("Iteration = %d\n", iterations);

		for (j = 0 ; j < numTCs ; j++) { //TC
			DMA_PRINTK ("TC = %d\n", j);

			for (i = 0 ; i < modes ; i++) {	//sync_mode
				DMA_PRINTK ("Mode = %d\n", i);

				/* Run all EDMA3 test cases */
				DMA_PRINTK ("Starting edma3_memtomemcpytest_dma\n");
				result = edma3_memtomemcpytest_dma(acnt, bcnt, ccnt, i, j);
				if (0 == result) {
					printk("edma3_memtomemcpytest_dma passed\n");
				} else {
					printk("edma3_memtomemcpytest_dma failed\n");
				}

				if (0 == result) {
					DMA_PRINTK ("Starting edma3_memtomemcpytest_dma_link\n");
					result = edma3_memtomemcpytest_dma_link(acnt, bcnt, ccnt, i, j);
					if (0 == result) {
						printk("edma3_memtomemcpytest_dma_link passed\n");
					} else {
						printk("edma3_memtomemcpytest_dma_link failed\n");
					}
				}

				if (0 == result) {
					DMA_PRINTK ("Starting edma3_memtomemcpytest_dma_chain\n");
					result = edma3_memtomemcpytest_dma_chain(acnt, bcnt, ccnt, i, j);
					if (0 == result) {
						printk("edma3_memtomemcpytest_dma_chain passed\n");
					} else {
						printk("edma3_memtomemcpytest_dma_chain failed\n");
					}
				}
			}
		}
	}
	return result;
}

/* Required exit function for all kernel modules */
void puggle_exit(void) {
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc1, dmaphyssrc1);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest1, dmaphysdest1);

	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufsrc2, dmaphyssrc2);
	dma_free_coherent(NULL, MAX_DMA_TRANSFER_IN_BYTES, dmabufdest2,	dmaphysdest2);

	printk ("\nExiting puggle module\n");
}

module_init(puggle_init);
module_exit(puggle_exit);

MODULE_AUTHOR("Yogi Patel <yapatel@gatech.edu>");
MODULE_LICENSE("Creative Commons 3.0");
