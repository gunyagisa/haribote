; a_nasm.s

bits 32

global api_putchar

api_putchar:
  mov   edx, 1
  mov   al, [esp + 4]
  int   0x40
  ret
