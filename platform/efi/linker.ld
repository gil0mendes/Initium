OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(boot-init)

SECTIONS
{
  . = 0x0800;
  boot-init = .;
}
