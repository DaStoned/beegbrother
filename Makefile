CC = xtensa-lx106-elf-g++

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

INCLUDES = -Iinc
CFLAGS = $(CCFLAGS) $(DEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)
LDLIBS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -Wl,--start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip -lwpa -lcrypto -lmain -Wl,--end-group
LDFLAGS = -Teagle.app.v6.ld -Wl,--gc-sections

# Pull in settings which are local to developer's environment
include Makefile.local

all: bin/beegbrother-0x00000.bin

bin:
	mkdir -p bin

bin/beegbrother-0x00000.bin: bin/beegbrother
	esptool elf2image $^

bin/beegbrother: bin/beegbrother.o
	$(CC) $^ $(LDFLAGS) $(LOADLIBES) $(LDLIBS) -o $@

bin/beegbrother.o: src/main.cpp | bin
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

flash: bin/beegbrother-0x00000.bin bin/beegbrother-0x10000.bin
	esptool write_flash --flash_mode $(ESP_FLASH_MODE) --flash_freq $(ESP_FLASH_FREQ) --flash_size detect 0 bin/beegbrother-0x00000.bin 0x10000 bin/beegbrother-0x10000.bin

clean:
	rm -f bin/beegbrother bin/beegbrother.o bin/beegbrother-0x00000.bin bin/beegbrother-0x10000.bin
