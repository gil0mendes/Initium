This is a build of OVMF revision 14854, taken from the OpenSUSE binary package:

- https://build.opensuse.org/package/binaries/Virtualization/OVMF?repository=openSUSE_13.1

The official prebuilt versions of OVMF are outdated and do not work on newer
versions of QEMU, hence why this build was used instead.

The PXE ROMs in this directory are for use by the EFI PXE test target. These ROMs are built with the following patches applied:   

- http://lists.ipxe.org/pipermail/ipxe-devel/2015-March/004008.html
- http://lists.ipxe.org/pipermail/ipxe-devel/2015-March/004009.html 

The latter prevents iPXE from registering its own load file protocol on network device handles, as doing so causes EDK to fail to register the PXE base code protocol (it tries to do so at the same time as installing its own load file protocol). These ROMs can be removed once the builds in upstream QEMU include these patches.

The firmware stores non-volatile settings in the ROM image itself (QEMU will write to the ROM), therefore the EFI test scripts take a copy of the image to prevent modifying the in-tree copy. We have multiple copies of the ROM image, one pre-configured to boot from HD and one to boot network, for the HD and PXE test scripts.