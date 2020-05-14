bits 32

global api_getlang

api_getlang:
  mov   edx, 27
  int   0x40
  ret
