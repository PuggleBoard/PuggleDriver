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

