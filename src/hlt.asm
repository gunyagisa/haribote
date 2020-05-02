; hlt.asm
bits 32

section .text
global HariMain

HariMain:
  mov   al, 'A'
  call  2*8:0x2da4 ; far call cons_putchar_asm
  retf

