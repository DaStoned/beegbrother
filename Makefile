# Makefile for this project

PROJ_NAME := beegbrother

TOOLCHAIN_PREFIX := xtensa-lx106-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
CXX := $(TOOLCHAIN_PREFIX)g++
SIZE := $(TOOLCHAIN_PREFIX)size

BIN_DIR := bin
SRC_DIR := src
INC_DIR := inc

DEFINES += -DICACHE_FLASH -DUSE_OPTIMIZE_PRINTF
CCFLAGS += \
	-Os \
	-Wpointer-arith \
	-Wundef \
	-Werror \
	-Wl,-EL \
	-fno-inline-functions \
	-nostdlib \
	-mlongcalls \
	-mtext-section-literals \
	-ffunction-sections \
	-fdata-sections \
	-fno-builtin-printf \
	-Wall

INCLUDES = -I$(INC_DIR)
CFLAGS = $(CCFLAGS) $(DEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-non-call-exceptions -fno-rtti -fno-use-cxa-atexit -ffunction-sections -fdata-sections -fno-builtin -std=c++11
LDLIBS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -Wl,--start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip -lwpa -lcrypto -lmain -Wl,--end-group
LDFLAGS = -Tsupport/ld/eagle.app.v6.ld -Wl,--gc-sections
MAPFLAGS = -Wl,-Map=$(BIN_DIR)/$(PROJ_NAME).map

VPATH = $(SRC_DIR):$(INC_DIR)
CXX_SOURCES := $(notdir $(wildcard $(SRC_DIR)/*.cpp))
OBJECTS := $(patsubst %.cpp,$(BIN_DIR)/%.o,$(CXX_SOURCES)) $(patsubst %.c,$(BIN_DIR)/%.o,$(C_SOURCES))

# Pull in settings which are local to developer's environment
include Makefile.local

.PHONY: all clean flash

# We really need both the "-0x00000.bin" and "-0x10000.bin" files, but
# make doesn't understand recipes which update two targets at once. It would
# try to execute the recipe twice in such case.
all: $(BIN_DIR)/$(PROJ_NAME).elf-0x00000.bin

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/$(PROJ_NAME).elf-0x00000.bin $(BIN_DIR)/$(PROJ_NAME).elf-0x10000.bin: $(BIN_DIR)/$(PROJ_NAME).elf
	$(SIZE) $^
	esptool elf2image $^

$(BIN_DIR)/$(PROJ_NAME).elf: $(OBJECTS) | $(BIN_DIR)
	$(info Building source files: $(C_SOURCES) $(CXX_SOURCES))
	$(CXX) $^ $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $(MAPFLAGS) -o $@

$(BIN_DIR)/%.o: %.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $^ -o $@

$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $^ -o $@

flash: $(BIN_DIR)/$(PROJ_NAME).elf-0x00000.bin $(BIN_DIR)/$(PROJ_NAME).elf-0x10000.bin
	esptool --baud $(ESP_FLASH_BAUD_RATE) write_flash --flash_mode $(ESP_FLASH_MODE) --flash_freq $(ESP_FLASH_FREQ) --flash_size detect 0 $(BIN_DIR)/$(PROJ_NAME).elf-0x00000.bin 0x10000 $(BIN_DIR)/$(PROJ_NAME).elf-0x10000.bin


clean:
	rm -f $(BIN_DIR)/*
