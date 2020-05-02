; hlt.asm
bits 32

section .text

  mov   al, 'A'
  call  2*8:0x2da1 ; far call cons_putchar_asm
  retf

