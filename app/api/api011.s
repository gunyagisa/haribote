bits 32

global api_point

section .text

api_point:
  push  edi
  push  esi
  push  ebx
  mov   edx, 11
  mov   ebx, [esp + 16]
  mov   esi, [esp + 20]
  mov   edi, [esp + 24]
  mov   eax, [esp + 28]
  int   0x40
  pop   ebx
  pop   esi
  pop   edi
  ret
