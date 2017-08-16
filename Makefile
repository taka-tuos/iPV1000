TARGET = iPV1000
OBJS = iPV1000.o io80.o psg.o vdp.o z80.o libgpu.o

INCDIR = 
CFLAGS = -O3 -G0 -Wall -std=gnu99
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = iPV1000

PSPSDK=$(shell psp-config --pspsdk-path)

PSPBIN = $(PSPSDK)/../bin
CFLAGS += $(shell $(PSPBIN)/sdl-config --cflags)
LIBS += $(shell $(PSPBIN)/sdl-config --libs)

include $(PSPSDK)/lib/build.mak
