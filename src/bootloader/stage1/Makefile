BUILD_DIR?=build
ASM?=nasm
ASMFLAGS?=-f bin

.PHONY: all stage1 clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: boot.asm
	$(ASM) $< $(ASMFLAGS) -o $@

clean:
	rm -f $(BUILD_DIR)/stage1.bin