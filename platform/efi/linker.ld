OUTPUT_FORMAT("pei-x86-64")
OUTPUT_ARCH("i386:x86-64")
ENTRY(efi_main)

SECTIONS
{
  . = 0;
  __start = .;

  .hash : { *(.hash) }

  . = ALIGN(0x1000);
  __text_start = .;
	.text : {
		*(.init.text)
		*(.text)
		*(.text.*)
	}

  /* Dummy relocation section for the PE binary so that EFI recognizes
	 * as as a relocatable image. */
	. = ALIGN(0x1000);
	.reloc : { *(.reloc) }

  . = ALIGN(0x1000);
  __data_start = .;
	.data : {
		/* Roll the read-only data into the main data section. It would
		 * not be write protected even if we put it separately, and PE
		 * requires page aligned sections so we only waste space by
		 * splitting it into another section. */
		*(.rodata)
		*(.rodata.*)
		*(.got)
		*(.got.plt)
    *(.init.data)
		*(.data)
		*(.data.*)


        /* builtins section */
        __builtins_start = .;
        *(.builtins)
        __builtins_end = .;
	}

	. = ALIGN(0x1000);
	.dynamic : { *(.dynamic) }

	. = ALIGN(0x1000);
  .rela.dyn : {
    *(.rela.init.text .rela.text .rela.text.*)
    *(.rela.rodata .rela.rodata.*)
    *(.rela.got)
    *(.rela.init.data .rela.data .rela.data.* .rela.builtins)
    *(.rela.bss .rela.bss.*)
  }

	. = ALIGN(0x1000);
  .dynsym : { *(.dynsym) }

	. = ALIGN(0x1000);
  .dynstr : { *(.dynstr) }

	. = ALIGN(0x1000);
  __bss_start = .;
	.bss : {
		__bss_start = .;
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		__bss_end = .;
	}

	. = ALIGN(0x1000);
	__end = .;

	/DISCARD/ : {
		*(.note.*)
		*(.comment*)
		*(.eh_frame*)

		/* Don't need ELF relocations for the dummy PE relocations. */
		*(.rela.reloc)
	}
}
