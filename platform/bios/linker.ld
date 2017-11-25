ENTRY(loader_entry)
OUTPUT_ARCH("i386:x86-64")
OUTPUT_FORMAT(binary)

SECTIONS {
    . = 0x20200;

    .text : {
        __text_start = .;
        *(.text .text.*)
        . = ALIGN(4K);
        __text_end = .;
    }

    .data : {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4K);
        __data_end = .;
    }

    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        . = ALIGN(4K);
        __bss_end = .;
    }
}
