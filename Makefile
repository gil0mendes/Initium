#
# This script is used to build Initium and the depdent components
#

# Include user specific configuration file
-include config.mak

# Include the default configuration file
include mk/config.mk

# TODO: move this into a separated file
arch ?= x86_64
target ?= $(arch)-unknown-none
platform ?= bios
bootloader := build/initium.bin
artifact := target/$(target)/debug/libinitium.a
linker_script := platform/$(platform)/linker.ld
iso := build/initium-$(arch).iso

# Qemu variables
QEMU=qemu-system-$(arch)
QEMUFLAGS=-serial mon:stdio -d cpu_reset -d guest_errors
QEMUFLAGS+=-smp 4 -m 1024 -monitor vc:1024x768 -s

.PHONY: all clean cargo run iso

all: $(bootloader)

clean:
	@cargo clean
	@rm -rf build

$(bootloader): cargo $(linker_script)
	@$(LD) $(LDFLAGS) -n --gc-sections -T $(linker_script) -o $(bootloader) $(artifact)

cargo:
	@xargo build --target $(target)

# Build ISO image
$(iso):
	@rm -rf build/iso/
	@mkdir -p build/iso/boot/
	@mkisofs -J -R -l -b boot/cdboot.bin -V "CDROM" \
		-boot-load-size 4 -boot-info-table -no-emul-boot \
		-o $@ build/iso/

# Run on qemu
run: $(iso)
	$(QEMU) $(QEMUFLAGS) -cdrom $(iso) -s

# Run EFI on qemu
run_efi:
	$(QEMU) $(QEMUFLAGS) -pflash .ovmf-amd64.bin -hda fat:${fsdir}
