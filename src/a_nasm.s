; a_nasm.s

bits 32

global api_putchar, api_end

section .text

api_putchar:
  mov   edx, 1
  mov   al, [esp + 4]
  int   0x40
  ret

api_putstr0: ; void api_putstr0(char *s)
  mov   ebx, [esp + 4]
  mov   edx, 2
  int   0x40
  ret

api_end:
  mov   edx, 4
  int   0x40
