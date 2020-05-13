bits 32

global api_fopen

api_fopen:
  push  ebx
  mov   edx, 21
  mov   ebx, [esp + 8]
  int   0x40
  pop   ebx
  ret
