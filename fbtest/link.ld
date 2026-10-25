ENTRY(_start)
SECTIONS
{
    . = 0;
    .text   : { KEEP(*(.text.boot)) *(.text*) }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }
    .bss    : { *(.bss*) *(COMMON) }
    . = ALIGN(16);
    .stack (NOLOAD) : { . += 0x4000; _stack_top = .; }
    _image_size = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
