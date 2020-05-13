bits 32

global api_cmdline

api_cmdline:
  push  ebx
  mov   edx, 26
  mov   ecx, [esp + 12]
  mov   ebx, [esp + 8]
  int   0x40
  pop   ebx
  ret
