#!/bin/bash -ex

builddir=build/x86-efi
fsdir=${builddir}/testfs

mkdir ${fsdir}
mkdir -p ${fsdir}/efi/boot

cp ${builddir}/bin/bootx64.efi ${fsdir}/efi/boot/
cp ${builddir}/test/test-ia32.elf ${builddir}/test/test-amd64.elf ${fsdir}/

cat > ${fsdir}/loader.cfg << EOF
set "timeout" 5

entry "Test (32-bit)" {
	initium "/test-ia32.elf" ["/test-ia32.elf"]
}

entry "Test (64-bit)" {
	initium "/test-amd64.elf" ["/test-amd64.elf"]
}
EOF

if [ ! -e ".ovmf-x86_64.bin" ]; then
    cp test/qemu/efi/ovmf-x86_64.bin .ovmf-x86_64.bin
fi

qemu-system-x86_64 -pflash .ovmf-x86_64.bin -hda fat:${fsdir} -serial stdio -m 512 -monitor vc:1024x768 -s

rm -rf ${fsdir}
