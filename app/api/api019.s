bits 32

global api_freetimer

section .text

api_freetimer:
  push  ebx
  mov   edx, 19
  mov   ebx, [esp + 8]
  pop   ebx
  ret
