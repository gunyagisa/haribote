bits 32

global api_write_serial

section .text

api_write_serial:
  mov   edx, 28
  mov   al, [esp + 8]
  int   0x40
  ret
