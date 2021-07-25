OUTPUT_FORMAT("elf32-m68k", "elf32-m68k", "elf32-m68k")
OUTPUT_ARCH(m68k)
EXTERN(start)
ENTRY(start)

SECTIONS {
	.text : {
		*(.text_startup)
		/*
		*(.text .text.*)
		*(.rodata .rodata.*)
		*/
		*(.text .text.* .rodata .rodata.*)
		/* *(.init)
		   *(.fini) */
		. = ALIGN(2);
	}

	/* __gp = . - 0x8000;   /* not implemented yet */
	. = -0x8000; /* 32K only */

	/* OS-9/68000 linkers often place .data after .bss, but we follow other standard. */
	.data : {
		*(.data .data.*)
		. = ALIGN(2);
	}

	.bss(NOLOAD) : {
		*(.bss .bss*)
		*(COMMON)
		. = ALIGN(2);
	}

	end = .;
}
