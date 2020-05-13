bits 32

global api_end

section .text

api_end:
  mov   edx, 4
  int   0x40
