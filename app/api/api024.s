bits 32

global api_fsize

api_fsize:
  mov   edx, 24
  mov   eax, [esp + 4]
  mov   ecx, [esp + 12]
  int   0x40
  ret
