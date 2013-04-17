#-------------------------------------------------------------------------

#	This file is part of the BoneClamp Data Conversion and Processing System
#	Copyright (C) 2013 BoneClamp

#-------------------------------------------------------------------------

#	Filename: Makefile for BoneClamp PRU

#	To the extent possible under law, the author(s) have dedicated all copyright
#	and related and neighboring rights to this software to the public domain
#	worldwide. This software is distributed without any warranty.

#	You should have received a copy of the CC Public Domain Dedication along with
#	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.

-include ../../DIRS

CROSS_COMPILE?=""
LIBDIR_APP_LOADER?=../loader/lib
INCDIR_APP_LOADER?=../loader/include
BINDIR?=../../bin
PASM?=../../utils/pasm

CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a -g
LDFLAGS+=-L$(LIBDIR_APP_LOADER) -lprussdrv -lpthread 
OBJDIR=obj
TARGET=$(BINDIR)

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))

_OBJ = something.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	#$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $< 
	gcc $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	#$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
