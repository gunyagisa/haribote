OUTPUT_FORMAT("binary")

stack_size = DEFINED(stack_size) ? stack_size : 64K;
heap_size = DEFINED(heap_size) ? heap_size : 1024K;

MEMORY {
  rom (rx) : ORIGIN = 0, LENGTH = 1024K
  ram (rwx) : ORIGIN = 64K, LENGTH = 1024K
}

SECTIONS
{
  .head : {
    LONG((stack_size + SIZEOF(.data) + SIZEOF(.bss) + heap_size + 0xfff) &~ 0xfff)
    LONG(0x69726148)
    LONG(0)
    LONG(stack_size)
    LONG(SIZEOF(.data))
    LONG(LOADADDR(.data))
    LONG(0xe9000000)
    LONG(HariMain - 0x20)
    LONG((ADDR(.bss) + SIZEOF(.bss) + 0xf) &~ 0xf)
  } > rom

  .text : { 
    *(.text.startup.*)
    *(.text) 
  } > rom

  .data : {
    *(.data)
    *(.rodata*)
    
    . = ALIGN(4);
    __preinit_array_start = .;
    *(.preinit_array)
    __preinit_array_start = .;

    __init_array_start = .;
    *(SORT(.init_array))
    __init_array_end = .;

    __fini_array_start = .;
    *(.fini_array)
    __fini_array_end = .;

    . = ALIGN(16);
  } > ram AT > rom

  .bss : AT(LOADADDR(.data) + SIZEOF(.data)) SUBALIGN(4) {
    *(.bss)
    . = ALIGN(16);
  } > ram

  /DISCARD/ : { *(.eh_frame) }
}
