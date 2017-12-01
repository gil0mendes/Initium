#
# This script is used to build Initium and the depdent components
#

# Include user specific configuration file
-include config.mak

# Include configurations files
include mk/config.mk
include mk/docker.mk
include mk/virtualbox.mk

# TODO: move this into a separated file
arch ?= x86_64
platform ?= efi
target ?= $(arch)-initium-$(platform)

bootloader := build/initium.bin
artifact := target/$(target)/debug/libinitium.a
artifact_dir := target/$(target)/debug/
linker_script := platform/$(platform)/linker.ld
iso := build/initium-$(arch).iso

build_dir=build/$(arch)-$(platform)
FS_DIR=$(build_dir)/testfs

# Platform assembly file
assembly_source_files := $(wildcard platform/$(platform)/*.asm)
assembly_object_files := $(patsubst platform/$(platform)/%.asm, \
	$(build_dir)/platform/%.o, $(assembly_source_files))

# Qemu variables
QEMU=qemu-system-$(arch)
QEMUFLAGS=-serial mon:stdio -d cpu_reset -d guest_errors
QEMUFLAGS+=-smp 4 -m 1024 -monitor vc:1024x768 -s

HDIMAGE = $(build_dir)/$(target).img

.PHONY: all clean cargo run iso

all: $(bootloader)

# Clean generated objects
clean:
	@xargo clean
	@rm -rf build

# Build rust code
cargo:
	xargo build --target=$(target)

# Link bootloader and produce PE compatible exe.
$(bootloader): cargo $(assembly_object_files)
	@$(LD) $(LDFLAGS) \
		-T $(linker_script) \
		-L $(artifact_dir) \
		-l initium \
		-o $@ \
		$(assembly_object_files) $(artifact)
	mkdir -p $(FS_DIR)/efi/boot
	$(OBJCOPY) \
		-j .text \
		-j .reloc \
		-j .data \
		-j .dynamic \
		-j .rela.dyn \
		-j .dynsym \
		-j .bss \
		--target efi-app-x86_64 \
		$(bootloader) $(FS_DIR)/efi/boot/bootx64.efi

# Compile platform assembly files
$(build_dir)/platform/%.o: platform/$(platform)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -felf64 $< -o $@

# Build ISO image
$(iso): $(bootloader)
	@mkisofs -V "CDROM" -o $@ $(FS_DIR)

build: $(iso)

# Run on qemu
run: $(iso)
	$(QEMU) $(QEMUFLAGS) \
		-drive if=pflash,format=raw,unit=0,file=.ovmf-amd64.bin,readonly=on \
		-drive file=$(iso),media=cdrom -boot d
	rm -rf $(FS_DIR)

## -- Not used for now

# Create HD image
$(HDIMAGE): $(bootloader)
	truncate -s 512M $(HDIMAGE).tmp
	parted $(HDIMAGE).tmp -s -a minimal mklabel gpt
	parted $(HDIMAGE).tmp -s -a minimal mkpart EFI FAT32 2048s 600000s
	parted $(HDIMAGE).tmp -s -a minimal toggle 1 boot
	truncate -s 307M $(HDIMAGE).part.img
	# dd if=/dev/zero of=$(HDIMAGE).part.img bs=512 count=600000
	mformat -i $(HDIMAGE).part.img -F -h 1 -t 1000 -n 500 -c 1
	mcopy -i $(HDIMAGE).part.img $(bootloader) ::
	dd if=$(HDIMAGE).part.img of=$(HDIMAGE).tmp bs=512 count=550000 seek=2048 conv=notrunc
	mv $(HDIMAGE).tmp $(HDIMAGE)
