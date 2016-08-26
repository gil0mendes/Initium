#!/bin/bash -ex

# Define some path vars
builddir=build/x86-efi
pxedir=${builddir}/pxe

# Create the necessary directories
mkdir ${pxedir}
mkdir ${pxedir}/boot

cp ${builddir}/bin/initiumx64.efi ${pxedir}/boot/bootx64.efi
cp ${builddir}/test/test-ia32.elf ${builddir}/test/test-amd64.elf ${pxedir}/

cat > ${pxedir}/loader.cfg << EOF
set "timeout" 5

entry "Test (32-bit)" {
  initium "test-ia32.elf" ["test-ia32.elf"]
}

entry "Test (64-bit)" {
  initium "test-amd64.elf" ["test-amd64.elf"]
}
EOF

if [ ! -e ".ovmf-amd64-pxe.bin" ]; then
  cp test/qemu/efi/ovmf-amd64-pxe.bin .ovmf-amd64-pxe.bin
fi

qemu-system-x86_64 \
  -pflash .ovmf-amd64-pxe.bin -L test/qemu/efi \
  -tftp ${pxedir} -bootp efi/boot/bootx64.efi \
  -serial stdio -m 512 -monitor vc:1024x768 -s

rm -rf ${pxedir}
