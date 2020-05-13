bits 32

global api_settimer

section .text

api_settimer:
  push  ebx
  mov   edx, 18
  mov   ebx, [esp + 8]
  mov   eax, [esp + 12]
  int   0x40
  pop   ebx
  ret
