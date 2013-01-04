# New Makefile for CS452 Kernel
#
# Steven MacLeod <sdmacleo@uwaterloo.ca>
# William Saunders
#
## Configuration ##

MAIN = kernel
USER = smws

ARCH = arm920t
XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld

CFLAGS = -c -fPIC -Wall -Iinclude -Isrc -mcpu=$(ARCH) -msoft-float -O3
ASFLAGS	= -mcpu=$(ARCH) -mapcs-32
LDFLAGS = -init main -Map bin/$(MAIN).map -N -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -Llib

#########################################################################
################## LIB                                 ##################
#########################################################################
LIB_C_SRC = $(wildcard src/lib/*.c)
LIB_ASM_SRC = $(wildcard src/lib/*.S)
LIB_C_OBJ = $(addprefix build/lib/,$(notdir $(LIB_C_SRC:.c=.o)))
LIB_ASM_OBJ = $(addprefix build/lib/,$(notdir $(LIB_ASM_SRC:.S=.o)))

#########################################################################
################## System Calls                        ##################
#########################################################################
SYSCALL_C_SRC = $(wildcard src/sys_calls/*.c)
SYSCALL_ASM_SRC = $(wildcard src/sys_calls/*.S)
SYSCALL_C_OBJ = $(addprefix build/sys_calls/,$(notdir $(SYSCALL_C_SRC:.c=.o)))
SYSCALL_ASM_OBJ = $(addprefix build/sys_calls/,$(notdir $(SYSCALL_ASM_SRC:.S=.o)))

#########################################################################
################## Tasks                        ##################
#########################################################################
STASK_C_SRC = $(wildcard src/tasks/*.c)
STASK_ASM_SRC = $(wildcard src/tasks/*.S)
STASK_C_OBJ = $(addprefix build/tasks/,$(notdir $(STASK_C_SRC:.c=.o)))
STASK_ASM_OBJ = $(addprefix build/tasks/,$(notdir $(STASK_ASM_SRC:.S=.o)))

#########################################################################
################## Kernel                              ##################
#########################################################################
KERN_C_SRC = $(wildcard src/kern/*.c)
KERN_ASM_SRC = $(wildcard src/kern/*.S)
KERN_C_OBJ = $(addprefix build/kern/,$(notdir $(KERN_C_SRC:.c=.o)))
KERN_ASM_OBJ = $(addprefix build/kern/,$(notdir $(KERN_ASM_SRC:.S=.o)))

## OBJECTS ##
COBJECTS = $(LIB_C_OBJ) $(STASK_C_OBJ) $(SYSCALL_C_OBJ) $(KERN_C_OBJ)
ASMOBJECTS = $(LIB_ASM_OBJ) $(STASK_ASM_OBJ) $(SYSCALL_ASM_OBJ) $(KERN_ASM_OBJ)
ASMFILES = $(COBJECTS:.o=.s) $(ASMJECTS:.o=.s) 

## Rules for Making ##
.phony: install remove clean

all: $(ASMFILES) bin/$(MAIN).elf

install: bin/$(MAIN).elf
	rm -f /u/cs452/tftp/ARM/$(USER)/$(MAIN).elf
	cp -f bin/$(MAIN).elf /u/cs452/tftp/ARM/$(USER)/$(MAIN).elf
	chgrp cs452_06 /u/cs452/tftp/ARM/$(USER)/$(MAIN).elf
	chmod g+rwx /u/cs452/tftp/ARM/$(USER)/$(MAIN).elf
	chmod a+r /u/cs452/tftp/ARM/$(USER)/$(MAIN).elf

remove:
	rm -f /u/cs452/tftp/ARM/$(USER)/$(MAIN).elf

clean:
	rm -f bin/*.elf
	rm -f bin/*.map
	rm -f build/lib/*
	rm -f build/tasks/*
	rm -f build/sys_calls/*
	rm -f build/kern/*

## Final Elf File##
bin/$(MAIN).elf: $(COBJECTS) $(ASMOBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(COBJECTS) $(ASMOBJECTS) -lgcc

## Move from src to build ##
build/%.s: src/%.s
	cp $< $@
	rm -f $<

build/%.s: src/%.S
	cp $< $@

## Compile ##
src/%.s: src/%.c
	$(XCC) -S $(CFLAGS) -o $@ $<

## Assemble ##
build/%.o: build/%.s
	$(AS) $(ASFLAGS) -o $@ $<
