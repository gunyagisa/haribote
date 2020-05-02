bits 32

section .text

  mov   edx, 2
  mov   ebx, msg
  int   0x40
  retf

msg:
  db    "HELLO", 0
