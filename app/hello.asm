; hlt.asm
bits 32


global HariMain

[section .text]

HariMain:
  mov   ebx, msg
  mov   edx, 2
  int   0x40
  mov   edx, 4
  int   0x40

section .data
msg:
  db    "Hello world", 0, 0x0a

