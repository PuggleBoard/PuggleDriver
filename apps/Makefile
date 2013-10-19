##
##	 -------------------------------------------------------------------------
##
##	 This file is part of the Puggle Data Conversion and Processing System
##	 Copyright (C) 2013 Puggle
##
##	 -------------------------------------------------------------------------
##
##	Written in 2013 by: Yogi Patel <yapatel@gatech.edu>
##
##	To the extent possible under law, the author(s) have dedicated all copyright
##	and related and neighboring rights to this software to the public domain
##	worldwide. This software is distributed without any warranty.
##
##	You should have received a copy of the CC Public Domain Dedication along with
##	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
##

-include DIRS

LIBDIR_APP_LOADER?=../../app_loader/lib
INCDIR_APP_LOADER?=../../app_loader/include
BINDIR_APPLICATIONS?=../bin
BINDIR_FW?=bin
INT_DIR=../interface
PASM?=../utils/pasm

all:
	@mkdir -p bin
	@make -C ../app_loader/interface
	@for dir in $(APP_DIRS); do make -C $$dir LIBDIR_APP_LOADER="${LIBDIR_APP_LOADER}" INCDIR_APP_LOADER="${INCDIR_APP_LOADER}" BINDIR="${BINDIR_APPLICATIONS}"; done
	@for a_file in ${ASSEM_FILES} ; do ${PASM} -b $$a_file ; done ;
	@mv *.bin ${BINDIR_FW}
	@cp bin/*.bin ${INT_DIR}
	@cp bin/puggle ${INT_DIR}
	#@rm -rf bin

clean:
	@for dir in $(APP_DIRS); do make -C $$dir clean LIBDIR_APP_LOADER="${LIBDIR_APP_LOADER}" LIBDIR_EDMA_DRIVER="${LIBDIR_EDMA_DRIVER}" INCDIR_APP_LOADER="${INCDIR_APP_LOADER}" INCDIR_EDMA_DRIVER="${INCDIR_EDMA_DRIVER}" BINDIR="${BINDIR_APPLICATIONS}"; done
	@for bins in ${BINDIR_FW};	do rm -rf ${BINDIR_FW}/${bins}; done
	@rm ${INT_DIR}/*.bin
	@rm ${INT_DIR}/puggle
