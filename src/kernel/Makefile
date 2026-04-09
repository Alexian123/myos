TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I.
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

ISRS_GEN_C=arch/i686/isrs_gen.c
ISRS_GEN_ASM=arch/i686/isrs_gen.inc

HEADERS_C=$(shell find . -type f -name "*.h")
SOURCES_C=$(shell find . -type f -name "*.c") $(ISRS_GEN_C)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))

HEADERS_ASM=$(shell find . -type f -name "*.inc") $(ISRS_GEN_ASM)
SOURCES_ASM=$(shell find . -type f -name "*.asm")
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Built  kernel.bin"

$(BUILD_DIR)/kernel/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Done: " $<

$(BUILD_DIR)/kernel/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Done: " $<

$(ISRS_GEN_C) $(ISRS_GEN_ASM):
	@$(SOURCE_DIR)/scripts/generate_isrs.sh $(ISRS_GEN_C) $(ISRS_GEN_ASM)

clean:
	rm -f $(BUILD_DIR)/kernel.bin
	rm -f arch/i686/isrs_gen.c
	rm -f arch/i686/isrs_gen.inc