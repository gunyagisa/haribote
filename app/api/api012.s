bits 32 

global api_refreshwin

section .text

api_refreshwin:
  push  edi
  push  esi
  push  ebx
  mov   edx, 12
  mov   ebx, [esp + 16]
  mov   eax, [esp + 20]
  mov   ecx, [esp + 24]
  mov   esi, [esp + 28]
  mov   edi, [esp + 32]
  int   0x40
  pop   ebx
  pop   esi
  pop   edi
  ret
