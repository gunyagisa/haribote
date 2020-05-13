bits 32

global api_initmalloc

section .text

api_initmalloc: ; void api_initmalloc(void)
  push  ebx
  mov   edx, 8
  mov   ebx, [cs:0x0020]
  mov   eax, ebx
  add   eax, 32 * 1024
  mov   ecx, [cs:0x0000]
  sub   ecx, eax
  int   0x40
  pop   ebx
  ret
