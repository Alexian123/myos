include config/config.mk

.PHONY: all run debug floppy_image kernel bootloader clean always tools_fat

all: floppy_image tools_fat

include config/toolchain.mk

#
# Run in QEMU
#
run: all
	@qemu-system-i386 -fda $(BUILD_DIR)/main_floppy.img

#
# Debug in Bochs
#
debug: all
	@bochs -f config/bochs.cfg

#
# Floppy image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	@dd if=/dev/zero of=$@ bs=512 count=2880
	@mkfs.fat -F 12 -n "NBOS" $@
	@dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc
	@mcopy -i $@ $(BUILD_DIR)/stage2.bin "::stage2.bin"
	@mcopy -i $@ $(BUILD_DIR)/kernel.bin "::kernel.bin"
	@mcopy -i $@ test/test.txt "::test.txt"
	@mmd -i $@ "::testdir"
	@mcopy -i $@ test/test.txt "::testdir/test.txt"
	@echo "--> Created: " $@

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	@$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat

$(BUILD_DIR)/tools/fat: $(TOOLS_SRC_DIR)/fat/fat.c always
	@$(MAKE) -C $(TOOLS_SRC_DIR)/fat BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/tools

#
# Clean
#
clean:
	@$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C $(TOOLS_SRC_DIR)/fat BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)