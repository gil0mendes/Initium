#!/bin/bash -ex

# Define some path vars
builddir=build/x86-bios
pxedir=${builddir}/pxe

# Make necessary directories
mkdir ${pxedir}
mkdir ${pxedir}/boot

cp ${builddir}/bin/pxeboot.img ${pxedir}/boot/
cp ${builddir}/test/test-ia32.elf ${builddir}/test/test-amd64.elf ${pxedir}/

cat > ${pxedir}/boot/loader.cfg << EOF
set "timeout" 5

entry "Test (32-bit)" {
  initium "test-ia32.elf" ["test-ia32.elf"]
}

entry "Test (64-bit)" {
  initium "test-amd64.elf" ["test-amd64.elf"]
}
EOF

qemu-system-x86_64 -bootp boot/pxeboot.img -tftp ${pxedir} -boot n -serial stdio -vga std -m 512 -monitor vc:1024x768 -s
rm -rf ${pxedir}
