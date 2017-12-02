  .section .text, "ax", @progbits
  .align 4

/**
 * EFI entry point
 * 
 * @param %rcx     Handle to the loader image
 * @param %dx      Pointer to EFI system table
 */
  .global loader_entry
  .type loader_entry, @function
loader_entry:
  // Preserve RBP/RDI/RSI as required by the MS calling convention to
  // avoid clobbering them, so that we can return to the firmware
  push  %rbp
  push  %rdi
  push  %rsi

  // Clear the stack frame pointer/RFLAGS
  xor   %rbp, %rbp
  push  $0
  popf

  // EFI uses the Microsoft x86_64 ABI. Arguments are passed in RCX/RDX
  push  %rcx
  push  %rdx

  // TODO: realocate the loader

1:
  // zero the BSS section.
  leaq  __bss_start(%rip), %rdi
  leaq  __bss_end(%rip), %rcx
  subq  %rdi, %rcx
  xorb  %al, %al
  rep stosb

  // We are enter with interruprs enabled. We don't want them.
  cli

  // Save the EFI GDT and IDT pointers which we must restore before
  // calling any EFI function
  leaq    efi_gdtp(%rip), %rax
  sgdt    (%rax)
  leaq    efi_idtp(%rip), %rax
  sidt    (%rax)
  leaq    efi_cs(%rip), %rax
  movw    %cs, (%rax)

  // Load the GDT
  // leaq    loader_gdtp(%rip), %rax
  // lgdt    (%rax)
  // push    $0x18
  // leaq    2f(%rip), %rax
  // push    %rax
  // lretq

2:
  // call the main function
  popq  %rsi
  popq  %rdi
  jmp   efi_main

.size loader_entry, . - loader_entry

.section .data, "aw", @progbits

/** Saved EFI code segment. */
efi_cs:
  .word   0

/** Saved EFI GDT pointer. */
efi_gdtp:
  .word   0
  .word   0

/** Saved EFI IDT pointer. */
efi_idtp:
  .word   0
  .quad   0

__dummy:
    .long   0

// hand-craft a dummy .reloc section so EFI knows it's a relocatable 
// executable:

  .section .reloc, "aw", @progbits

__dummy_reloc:
  .long   __dummy - __dummy_reloc     // Page RVA
  .long   10                          // Block Size (2*4+2)
  .word   (0<<12) + 0                 // reloc for dummy
