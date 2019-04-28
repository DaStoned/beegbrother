CC = xtensa-lx106-elf-gcc
CXX = xtensa-lx106-elf-g++

BIN_DIR := bin
SRC_DIR := src
INC_DIR := inc

DEFINES += -DICACHE_FLASH
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
	-fno-builtin-printf

INCLUDES = -I$(INC_DIR)
CFLAGS = $(CCFLAGS) $(DEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)
CXXFLAGS = $(CFLAGS)
LDLIBS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -Wl,--start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip -lwpa -lcrypto -lmain -Wl,--end-group
LDFLAGS = -Teagle.app.v6.ld -Wl,--gc-sections

VPATH = $(SRC_DIR):$(INC_DIR)
SOURCES := $(notdir $(wildcard $(SRC_DIR)/*.cpp))
OBJECTS := $(patsubst %.cpp,$(BIN_DIR)/%.o,$(SOURCES))
#OBJECTS := $(SOURCES:.cpp=.o)

# Pull in settings which are local to developer's environment
include Makefile.local

all: $(BIN_DIR)/beegbrother-0x00000.bin $(BIN_DIR)/beegbrother-0x10000.bin

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/beegbrother-0x00000.bin $(BIN_DIR)/beegbrother-0x10000.bin: $(BIN_DIR)/beegbrother
	esptool elf2image $^

$(BIN_DIR)/beegbrother: $(OBJECTS) | $(BIN_DIR)
	$(info Building source files: $(SOURCES))
	$(CXX) $^ $(LDFLAGS) $(LOADLIBES) $(LDLIBS) -o $@

$(BIN_DIR)/%.o: %.cpp | $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

flash: $(BIN_DIR)/beegbrother-0x00000.bin $(BIN_DIR)/beegbrother-0x10000.bin
	esptool write_flash --flash_mode $(ESP_FLASH_MODE) --flash_freq $(ESP_FLASH_FREQ) --flash_size detect 0 $(BIN_DIR)/beegbrother-0x00000.bin 0x10000 $(BIN_DIR)/beegbrother-0x10000.bin

.PHONY: clean

clean:
	rm -f $(BIN_DIR)/*
