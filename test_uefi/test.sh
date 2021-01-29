#!/bin/sh

# compile
cargo b --target=x86_64-unknown-uefi.json || exit 1

# build qemu env and convert ELF inyo PE
mkdir qemu-env/
cp ./target/x86_64-unknown-uefi/debug/test_uefi.efi qemu-env/BootX64_elf.efi
objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .rel* -j .rela* \
    -j .reloc --target=efi-app-x86_64 ./qemu-env/BootX64_elf.efi ./qemu-env/BootX64.efi || exit 1

# start qemu
qemu-system-x86_64 -s \
-nodefaults \
-vga std \
-machine type=q35,accel=hvf \
-m 128M \
-drive if=pflash,format=raw,readonly,file=../OVMF_CODE.fd \
-drive if=pflash,format=raw,file=../OVMF_VARS-1024x768.fd \
-drive format=raw,file=fat:rw:qemu-env/ \
-serial stdio \
-monitor vc:1024x768 \
-device ahci,id=ahci,multifunction=on
