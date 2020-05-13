bits 32

global api_putstrwin

section .text

api_putstrwin: ; void api_putstrwin(int win, int x, int y, int col, int len, char *str)
  push  edi
  push  esi
  push  ebp
  push  ebx
  mov   edx, 6
  mov   ebx, [esp + 20]
  mov   esi, [esp + 24]
  mov   edi, [esp + 28]
  mov   eax, [esp + 32]
  mov   ecx, [esp + 36]
  mov   ebp, [esp + 40]
  int   0x40
  pop   ebx
  pop   ebp
  pop   esi
  pop   edi
  ret
