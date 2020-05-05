; a_nasm.s

bits 32

global api_putchar, api_end, api_putstr0, api_openwin, api_openwin, api_boxfillwin, api_putstrwin

section .text

api_putchar:
  mov   edx, 1
  mov   al, [esp + 4]
  int   0x40
  ret

api_putstr0: ; void api_putstr0(char *s)
  mov   ebx, [esp + 4]
  mov   edx, 2
  int   0x40
  ret

api_openwin: ; int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title)
  push  edi
  push  esi
  push  ebx
  mov   edx, 5
  mov   ebx, [esp + 16]
  mov   esi, [esp + 20]
  mov   edi, [esp + 24]
  mov   eax, [esp + 28]
  mov   ecx, [esp + 32]
  int   0x40
  pop   ebx
  pop   esi
  pop   edi
  ret

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

api_boxfillwin:
  push  edi
  push  esi
  push  ebp
  push  ebx
  mov   edx, 7
  mov   ebx, [esp + 20]
  mov   eax, [esp + 24]
  mov   ecx, [esp + 28]
  mov   esi, [esp + 32]
  mov   edi, [esp + 36]
  mov   ebp, [esp + 40]
  int   0x40
  pop   ebx
  pop   ebp
  pop   esi
  pop   edi
  ret

api_end:
  mov   edx, 4
  int   0x40
