bits 32

global api_putstr0

section .text

api_putstr0: ; void api_putstr0(char *s)
  mov   ebx, [esp + 4]
  mov   edx, 2
  int   0x40
  ret
