bits 32

global api_alloctimer

section .text

api_alloctimer:
  mov  edx, 16
  int   0x40
  ret
