#
#-------------------------------------------------------------------------

#	This file is part of the BoneClamp Data Conversion and Processing System
#	Copyright (C) 2013 BoneClamp

#-------------------------------------------------------------------------

#	Filename: Makefile for BoneClamp PRU code

#	To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

#	You should have received a copy of the CC Public Domain Dedication along with this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
#

-include DIRS

# determine if cross-compilation necessary
SYS := $(shell uname -a | grep "beaglebone")
ifneq ($(SYS), beaglebone)
    CROSS_COMPILE?=arm-none-linux-gnueabi-
else
    CROSS_COMPILE?=""
endif

LIBDIR_APP_LOADER?=../loader/lib
INCDIR_APP_LOADER?=../loader/include
BINDIR_APPLICATIONS?=bin
BINDIR_FW?=bin
PASM?=utils/pasm

all:
# Make PRU example applications
	mkdir -p bin

	for dir in $(APP_DIRS); do make -C $$dir CROSS_COMPILE="${CROSS_COMPILE}" LIBDIR_APP_LOADER="${LIBDIR_APP_LOADER}" LIBDIR_EDMA_DRIVER="${LIBDIR_EDMA_DRIVER}" INCDIR_APP_LOADER="${INCDIR_APP_LOADER}" INCDIR_EDMA_DRIVER="${INCDIR_EDMA_DRIVER}" BINDIR="${BINDIR_APPLICATIONS}"; done

# Pass PRU assembly code for each example through assembler
	for a_file in ${ASSEM_FILES} ; \
	do \
          ${PASM} -b $$a_file ; \
	done ; \
	mv *.bin ${BINDIR_FW}

clean:
	for dir in $(APP_DIRS); do make -C $$dir clean LIBDIR_APP_LOADER="${LIBDIR_APP_LOADER}" LIBDIR_EDMA_DRIVER="${LIBDIR_EDMA_DRIVER}" INCDIR_APP_LOADER="${INCDIR_APP_LOADER}" INCDIR_EDMA_DRIVER="${INCDIR_EDMA_DRIVER}" BINDIR="${BINDIR_APPLICATIONS}"; done

	for bin_file in ${BIN_FILES}; do rm -fr ${BINDIR_FW}/$$bin_file; done

