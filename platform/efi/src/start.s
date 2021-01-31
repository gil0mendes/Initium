.section .text, "ax", @progbits
	
.global _start;
.type _start, @function;
_start:
	cli
    hlt
    .size _start, . - _start

.type _relocate, @function;
_relocate:
    cli
    hlt
    .size _relocate, . - _relocate

.section .data, "aw", @progbits

__dummy:
    .long   0

.section .reloc, "aw", @progbits

__dummy_reloc:
    .long   __dummy - __dummy_reloc
    .long   10
    .word   0
