bits 32

global api_openwin

section .text

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
