bits 32

global api_write_serial

api_write_serial:
  push  ebx
  mov   edx, 28
  mov   ebx, [esp + 8]
  int   0x40
  pop   ebx
  ret
