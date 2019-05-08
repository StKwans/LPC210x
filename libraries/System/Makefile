# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
#Used by everything, but doesn't need to be first. Section definitions are
CPPSRC += ../libraries/System/Startup.cpp
CPPSRC += ../libraries/System/hardware_stack.cpp
# Every Loginator sketch uses its own main.cpp, so we reference it here. Link it last so that it can use all
# staticaly declared, dynamically initialized objects.
CPPSRC += main.cpp

#INIT_CPPSRC - Solution to "static initialization fiasco". Initialization (especially dynamic initialization)
#is ordered in one translation unit and in an unspecified order between translation units, but our linker script
#specifies that dinamic initialization code for each translation unit is called in order that the translation
#units are specified in the linker command line. These routines will be linked first, so that any constructors
#in any translation unit can use these secure in the knowledge that they are already initialized. All other
#initializers are ultimately determined by the Makefile for each library and the order that the library Makefiles
#are included in the top-level program Makefile. Only include things in here that actually need to be initialized
#before anything else.

#Set up VIC first to catch any spurious interrupts.
INIT_CPPSRC  = ../libraries/System/vic.cpp
#Timer doesn't depend on VIC, but just in case, we do VIC first. Function delay() depends on PCLK being set right.
#Since delay() is pretty low-level, we want it available to other constructors, but VIC construction doesn't use it.
INIT_CPPSRC += ../libraries/System/Time.cpp

LIBMAKE+=Makefile ../libraries/System/Makefile

TARGETPATH:=$(CURDIR)
TARGETBASE=$(notdir $(TARGETPATH))

ifndef TARGET
TARGET=$(TARGETBASE)
endif

ABI = arm-none-eabi
LD_SCRIPT = ../libraries/System/lowmem_cpp_eabi.ld

reduce = $(if $(strip $2),$(call reduce,$1,$(wordlist 2,$(words $2),$2),$(call $1,$(firstword $2),$3)),$3)

#reduce takes two arguments: a function that will be called by reduce and a list to process.  
#The first argument is called with two arguments: each element of the list in reduce's second 
#argument and the result of the previous call to the function.

#To see this in action we can create a list unique function that removes duplicates without reordering:

check_uniq = $(if $(filter $1,$2),$2,$2 $1)

uniq = $(call reduce,check_uniq,$1)
SRC:=$(call uniq,$(SRC))
SRCARM:=$(call uniq,$(SRCARM))
CPPSRC:=$(call uniq,$(CPPSRC))
CPPSRCARM:=$(call uniq,$(CPPSRCARM))
ASRC:=$(call uniq,$(ASRC))
ASRCARM:=$(call uniq,$(ASRCARM))
INIT_CPPSRC:=$(call uniq,$(INIT_CPPSRC))
LIBMAKE:=$(call uniq,$(LIBMAKE))
EXTRAOBJ:=$(call uniq,$(EXTRAOBJ))
ALLSRC = $(INIT_CPPSRC) $(SRC) $(SRCARM) $(CPPSRC) $(CPPSRCARM) $(ASRC) $(ASRCARM)
INCDEF = $(shell cat .dep/* | sed -n 's/\([^:]*.h\)\s*:/\1/gp' | sort | uniq)

FLASH_TOOL = LPC12
#FLASH_TOOL = UVISION
#FLASH_TOOL = OPENOCD

#Passed as a -D to the compiler for each C and C++ file, to allow
#all the modules to know what hardware they are on. Choices are
#LOGOMATIC (old Sparkfun green board), LOGINATOR (purple Loginator
#board) and ROCKETOMETER (purple Rocketometer board)
ifndef HARDWARE
HARDWARE = ROCKETOMETER
endif

CDEFS += -DLPC2148

# MCU name and submodel
MCU      = arm7tdmi-s
SUBMDL   = LPC2148
#THUMB    = -mthumb

#This is needed to run IAP function call stuff
THUMB_IW = -mthumb-interwork

## Output format. (can be ihex or binary)
## (binary i.e. for openocd and SAM-BA, hex i.e. for lpc21isp and uVision)

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = -Og -fomit-frame-pointer

## Using the Atmel AT91_lib produces warning with
## the default warning-levels. 
## yes - disable these warnings; no - keep default settings
#AT91LIBNOWARN = yes
AT91LIBNOWARN = no

# Debugging format.
# Needed in order to get source code listings in .lst and .lss files
DEBUG = -gstabs
#DEBUG = -gdwarf-2

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS += ../libraries/System

# List any extra directories to look for library files here.
#     Each directory must be seperated by a space.
#EXTRA_LIBDIRS = ../arm7_efsl_0_2_4
EXTRA_LIBDIRS = 

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for ASM here
ADEFS +=  -D$(RUN_MODE)

# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = $(DEBUG) -D$(HARDWARE)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += $(OPT)
CFLAGS += -Wall -Wcast-align -Wsuggest-override
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wno-attributes
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fsingle-precision-constant -mfloat-abi=soft

# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

ifneq ($(AT91LIBNOWARN),yes)
#AT91-lib warnings with:
CFLAGS += -Wcast-qual
CONLYFLAGS += -Wmissing-prototypes 
CONLYFLAGS += -Wstrict-prototypes
CONLYFLAGS += -Wmissing-declarations
endif

# flags only for C++ (arm-elf-g++)
CPPFLAGS = -fno-rtti -fno-exceptions -fno-unwind-tables -std=c++17

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.S=.lst),-g$(DEBUG)


#Additional libraries.

# Extra libraries
#    Each library-name must be seperated by a space.
#    To add libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
#EXTRA_LIBS = efsl
EXTRA_LIBS =

#Support for newlibc-lpc (file: libnewlibc-lpc.a)
#NEWLIBLPC = -lnewlib-lpc

MATH_LIB = -lm

#CPLUSPLUS_LIB = -lstdc++


# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map,$(TARGET).map,--cref,--gc-sections,--demangle,-t
LDFLAGS += -lc
LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
#LDFLAGS += -lgcc
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))

# Set Linker-Script Depending On Selected Memory and Controller

#This line is used if you want to load main.hex into 0x00
#LDFLAGS +=-Tlpc2138.cmd
LDFLAGS += -T$(LD_SCRIPT)


# ---------------------------------------------------------------------------
# Flash-Programming support using lpc21isp by Martin Maurer 
# only for Philips LPC and Analog ADuC ARMs
#
# Settings and variables:
#LPC21ISP = lpc21isp
LPC21ISP = lpc21isp
LPC21ISP_PORT = /dev/ttyUSB0
LPC21ISP_BAUD = 38400
LPC21ISP_XTAL = 12000
LPC21ISP_FLASHFILE = $(TARGET).hex
# verbose output:
#LPC21ISP_DEBUG = -debug
# enter bootloader via RS232 DTR/RTS (only if hardware supports this
# feature - see Philips AppNote):
LPC21ISP_CONTROL = -control
# ---------------------------------------------------------------------------


# Define directories, if needed.
##DIRARM = c:/WinARM/
## DIRARMBIN = $(DIRAVR)/bin/
## DIRAVRUTILS = $(DIRAVR)/utils/bin/

# Define programs and commands.
SHELL = sh
CC = $(ABI)-gcc
CPP = $(ABI)-g++
OBJCOPY = $(ABI)-objcopy
OBJDUMP = $(ABI)-objdump
SIZE = $(ABI)-size
NM = $(ABI)-nm
REMOVE = rm -f
COPY = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = "-------- begin (mode: $(RUN_MODE)) --------"
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_DISASM_LISTING = Creating Disassembly:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_ARM = "Compiling C (ARM-only):"
MSG_COMPILINGCPP = Compiling C++:
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARM = "Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.

# Define all object files.
COBJ      = $(SRC:.c=.o)
AOBJ      = $(ASRC:.S=.o)
COBJARM   = $(SRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)
CPPOBJ    = $(INIT_CPPSRC:.cpp=.o) $(CPPSRC:.cpp=.o)
CPPOBJARM = $(CPPSRCARM:.cpp=.o)


# Define all listing files.
LST = $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(SRC:.c=.lst) $(SRCARM:.c=.lst)
LST += $(INIT_CPPSRC:.cpp=.lst) $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MMD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
.DEFAULT_GOAL:= all
all: begin gccversion sizebefore build finished end

#ifeq ($(FORMAT),ihex)
#build: elf hex lss sym
#hex: $(TARGET).hex
#IMGEXT=hex
#else 
#ifeq ($(FORMAT),binary)
build: elf bin lss hex

bin: $(TARGET).bin
hex: $(TARGET).hex
elf: $(TARGET).elf
lss: $(TARGET).lss 

# Eye candy.
begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo

Doxyfile: ../libraries/System/Doxyfile.template
	echo INCDEF: $(INCDEF)
	sed -e 's%^[^I]*INPUT[ \t]*=.*$$%INPUT=$(ALLSRC) $(INCDEF)%' \
	    -e 's%^[^P]*PROJECT_NAME[ \t]*=.*$$%PROJECT_NAME="$(TARGET)"%' \
	    -e 's%^[^P]*PROJECT_BRIEF[ \t]*=.*$$%PROJECT_BRIEF=$(PROJECT_BRIEF)%' \
	    < $< > $@

html: Doxyfile $(ALLTAR)
	doxygen

# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A -x $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi


# Display compiler version information.
gccversion : 
	@$(CC) --version


# Program the device.  - lpc21isp will not work for SAM7
program: $(LPC21ISP_FLASHFILE) $(TARGET).lss sizeafter
	@echo
	@echo $(MSG_LPC21_RESETREMINDER)
	$(LPC21ISP) $(LPC21ISP_OPTIONS) $(LPC21ISP_DEBUG) $(LPC21ISP_FLASHFILE) $(LPC21ISP_PORT) $(LPC21ISP_BAUD) $(LPC21ISP_XTAL)


# Create final output file (.hex) from ELF output file.
%.hex: %.bin
	@echo
	@echo $(MSG_FLASH) $@
	srec_cat -O $@ -I $< -bin

# Create final output file (.bin) from ELF output file.
$(TARGET).bin: $(TARGET).elf 
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O binary $< $@

# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h $< > $@
	$(OBJDUMP) -S -j .text $< >> $@
	$(OBJDUMP) -s -j .vtable -j .rodata -j .ctors  -j .data $< >> $@
	$(OBJDUMP) -t $< | grep ^[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f] | sort >> $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)

$(TARGET).elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) $(EXTRAOBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) $(EXTRAOBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -E $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $(<:.c=.e)
	$(CC) -S $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) -fverbose-asm $< -o $(<:.c=.s)
	$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING_ARM) $<
	$(CC) -E $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $(<:.c=.e)
	$(CC) -S $(ALL_CFLAGS) $(CONLYFLAGS) -fverbose-asm $< -o $(<:.c=.s)
	$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM/Thumb
$(CPPOBJ) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP) $<
	$(CPP) -E $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $(<:.cpp=.e)
	$(CPP) -S $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) -fverbose-asm $< -o $(<:.cpp=.s)
	$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : %.o : %.cpp
	@echo
	echo $(CPPOBJARM)
	echo $(CPPSRCARM)
	@echo $(MSG_COMPILINGCPP_ARM) $<
	$(CPP) -E $(ALL_CFLAGS) $(CPPFLAGS) $< -o $(<:.cpp=.e)
	$(CPP) -S $(ALL_CFLAGS) $(CPPFLAGS) -fverbose-asm $< -o $(<:.cpp=.s)
	$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

prog_lin:
	./lpc21isp -control main.hex /dev/ttyS0 38400 1200

# Target: clean project.
clean: begin clean_list finished end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) ../Common_WinARM/$(TARGET).hex
	$(REMOVE) Doxyfile
	$(REMOVE) -r html
	$(REMOVE) -r latex
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).bin $(TARGET).nosource.bin
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).elf $(TARGET).nosource.elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).tar
	$(REMOVE) $(TARBALL)
	$(REMOVE) $(TARBALL).o
	$(REMOVE) $(TEST_ARCHIVE)
	$(REMOVE) $(COBJ)
	$(REMOVE) $(CPPOBJ)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(CPPOBJARM)
	$(REMOVE) $(AOBJARM)
	@echo $(REMOVE) $(BACKUP)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) $(SRC:.c=.e)
	$(REMOVE) $(SRCARM:.c=.s)
	$(REMOVE) $(SRCARM:.c=.d)
	$(REMOVE) $(SRCARM:.c=.e)
	$(REMOVE) $(CPPSRC:.cpp=.s) 
	$(REMOVE) $(CPPSRC:.cpp=.d)
	$(REMOVE) $(CPPSRC:.cpp=.e)
	$(REMOVE) $(CPPSRCARM:.cpp=.s) 
	$(REMOVE) $(CPPSRCARM:.cpp=.d)
	$(REMOVE) $(CPPSRCARM:.cpp=.e)
	$(REMOVE) .dep/*
	$(REMOVE) doxygen*.tmp

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(ALLDEP)

PHONY_TGT += all begin finish end sizebefore sizeafter gccversion
PHONY_TGT += build elf hex bin lss clean clean_list program test_archive

# Listing of phony targets.
.PHONY : $(PHONY_TGT)
