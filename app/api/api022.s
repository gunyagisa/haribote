bits 32 

global api_fclose

api_fclose:
  mov   edx, 22
  mov   eax, [esp + 4]
  int   0x40
  ret
