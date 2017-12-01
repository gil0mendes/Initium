bits 64

section .text
global loader_entry
loader_entry:
  cli
  hlt
  jmp $

section .data
__dummy db 0

section .reloc
__dummy_reloc:
  dq 0
