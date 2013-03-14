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
BINDIR?=../../bin
PASM?=../../utils/pasm

CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a
LDFLAGS+=-L$(LIBDIR_APP_LOADER) -lprussdrv -lpthread
OBJDIR=obj
TARGET=$(BINDIR)

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))

_OBJ = something.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
