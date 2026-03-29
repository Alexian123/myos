BUILD_DIR?=build
ASM?=nasm
ASMFLAGS?=-f bin

.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: boot.asm
	@$(ASM) $< -f bin -o $@
	@echo "--> Built  stage1.bin"

clean:
	@rm -f $(BUILD_DIR)/stage1.bin