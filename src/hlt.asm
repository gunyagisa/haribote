; hlt.asm
bits 32

section .text
global HariMain

HariMain:
  cli
fin:
  hlt
  jmp   fin
