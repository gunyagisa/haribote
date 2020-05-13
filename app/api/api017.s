bits 32 

global api_inittimer

section .text

api_inittimer:
  push  ebx
  mov   edx, 17
  mov   ebx, [esp + 8]
  mov   eax, [esp + 12]
  int   0x40
  pop   ebx
  ret
