bits 32

global api_putstr1

section .text

api_putstr1:
  push  ebx
  mov   edx, 3
  mov   ebx, [esp + 8]
  mov   ecx, [esp + 12]
  int   0x40
  pop   ebx
  ret
