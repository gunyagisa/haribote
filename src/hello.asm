; hlt.asm
bits 32

  mov   ecx, msg
  mov   edx, 1

loop:
  mov   al, [cs:ecx]
  cmp   al, 0
  je    fin
  int   0x40
  add   ecx, 1
  jmp   loop

fin:
  retf

msg:
  db    "Hello world", 0

