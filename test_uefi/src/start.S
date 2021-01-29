	.text
	.align 4

	.globl _start
_start:
	subq $8, %rsp
	pushq %rcx
	pushq %rdx

0:
	lea ImageBase(%rip), %rdi
	lea _DYNAMIC(%rip), %rsi

	popq %rcx
	popq %rdx
	pushq %rcx
	pushq %rdx
	call _relocate

	popq %rdi
	popq %rsi

	call efi_main
	addq $8, %rsp

.exit:	
  	ret

_relocate:
    cli
    hlt

 	// hand-craft a dummy .reloc section so EFI knows it's a relocatable executable
 
 	.data
dummy:	.long	0

 	.section .reloc, "a"
label1:
	.long	dummy-label1				// Page RVA
 	.long	10					// Block Size (2*4+2)
	.word	(0<<12) +  0		// reloc for dummy
