/*
-------------------------------------------------------------------------

	This file is part of the BoneClamp Data Conversion and Processing System
	Copyright (C) 2013 BoneClamp

-------------------------------------------------------------------------

	Filename: maps.h

	To the extent possible under law, the author(s) have dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.

	You should have received a copy of the CC Public Domain Dedication along with
	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
*/

#ifndef _MAPS_H
#define _MAPS_H

/*  ATTENTION: This file is used both by C and PASM. Strictly defines 
 *  and includes only!!
 *  
 *
 *  Here the user can define where to keep paramaters quantitatively
 *  describing the data acquisition, as well as where the datapoints
 *  are kept during acquisition process.
 *
 *  By default, these are all in the shared memory of the PRU. Be
 *  extremely careful when editing these addresses.
 */

//Add this to addresses when using the constants defined below
//in userspace!
#define USERSPACE_OFFSET    0x4a300000

//Addresses from PRU perspective
#define RESOLUTION          0x00010000
#define SAMPLING_FREQ       0x00010004
#define SAMPLING_PERIOD     0x00010008
#define NUMBER_OF_CHANNELS  0x0001000C
#define VOLTAGE_LIMIT       0x00010010
#define CHANNEL_1           0x00010020
#define CHANNEL_2           0x00010024
#define CHANNEL_3           0x00010028
#define CHANNEL_4           0x0001002C
#define CHANNEL_5           0x00010030
#define CHANNEL_6           0x00010034
#define CHANNEL_7           0x00010038
#define CHANNEL_8           0x0001003C
#define CHANNEL_9           0x00010040
#define CHANNEL_10          0x00010044
#define CHANNEL_11          0x00010048
#define CHANNEL_12          0x0001004C
#define CHANNEL_13          0x00010050
#define CHANNEL_14          0x00010054
#define CHANNEL_15          0x00010058
#define CHANNEL_16          0x0001005C

#define FIFO_IN             0x00010060


#endif

