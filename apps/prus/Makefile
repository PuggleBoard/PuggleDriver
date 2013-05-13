##
##	 -------------------------------------------------------------------------
##
##	 This file is part of the BoneClamp Data Conversion and Processing System
##	 Copyright (C) 2013 BoneClamp
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

LIBDIR_APP_LOADER?=../../app_loader/lib
INCDIR_APP_LOADER?=../../app_loader/include
BINDIR?=../bin
GCC=gcc

CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a
LDFLAGS+=-L$(LIBDIR_APP_LOADER) -lprussdrv -lpthread
OBJDIR=obj
TARGET=$(BINDIR)/prus

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))

_OBJ = PRU.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(GCC) $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(GCC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
