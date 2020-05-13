bits 32

global api_beep

section .text

api_beep:
  mov   edx, 20
  mov   eax, [esp + 4]
  int   0x40
  ret
