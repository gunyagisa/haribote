; hlt.asm
bits 32

section .text

  sti
  mov   al, 'H'
  int   0x40
  mov   al, 'e'
  int   0x40
  mov   al, 'l'
  int   0x40
  mov   al, 'l'
  int   0x40
  mov   al, 'o'
  int   0x40
  retf

