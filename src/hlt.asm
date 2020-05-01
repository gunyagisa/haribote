; hlt.asm
bits 32

section .text
global HariMain

HariMain:
  hlt
  jmp   HariMain
