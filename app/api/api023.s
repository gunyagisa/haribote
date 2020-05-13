bits 32

global api_free

api_free:
  push  ebx
  mov   edx, 23
  mov   eax, [esp + 8]
  mov   ecx, [esp + 12]
  mov   ebx, [esp + 16]
  int   0x40
  pop   ebx
  ret
