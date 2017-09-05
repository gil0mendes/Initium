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
linker_script := src/platform/$(platform)/linker.ld
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
	build/arch/$(arch)/%.o, $(assembly_source_files))

.PHONY: all clean cargo

all: $(bootloader)

clean:
	@cargo clean
	@rm -rf build

$(bootloader): cargo $(assembly_object_files) $(linker_script)
	@$(LD) $(LDFLAGS) -n --gc-sections -T $(linker_script) -o $(bootloader) $(assembly_object_files) $(artifact)

cargo:
	@xargo build --target $(target)

# compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@