bits 32

global api_fread

api_fread:
  push  ebx
  mov   edx, 25
  mov   eax, [esp + 16]
  mov   ecx, [esp + 12]
  mov   ebx, [esp + 8]
  int   0x40
  pop   ebx
  ret

