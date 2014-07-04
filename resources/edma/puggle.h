/*
	 Puggle DMA driver library and structures 

	 Parts of this code are modified from the TI EDMA sample application
	 and also from code developed by Terrence McGuckin <terrence@ephemeron-labs.com>
	 and Andrew Righter <q@crypto.com> from Ephemeron Labs
*/

#define PUGGLE_START		0x1
#define PUGGLE_STOP			0x0

struct  puggle_cnt{
	unsigned int bcnt;
	unsigned int ccnt;
};
