BUILD_DIR?=build
ASM?=nasm
ASMFLAGS?=-f bin

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: main.asm
	$(ASM) $< $(ASMFLAGS) -o $@

clean:
	rm -f $(BUILD_DIR)/kernel.bin