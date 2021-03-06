;asmfunc.asm

bits 32

global	io_hlt , io_cli, io_sti, io_stihlt
global 	io_out8 ,io_out16, io_out32, io_in8, io_in16, io_in32
global	io_store_eflags ,io_load_eflags, store_cr0, load_cr0
global	load_gdtr, load_idtr
global  inthandler21_asm, inthandler2c_asm, inthandler20_asm, inthandler0d_asm, inthandler0c_asm
global  load_tr, farjmp, farcall
global  hrb_api_asm, start_app, end_app_asm

EXTERN  inthandler21, inthandler2c, inthandler20, inthandler0d, inthandler0c
extern  hrb_api

;write function below

section .text

io_hlt:	;void io_hlt(void)
	HLT
	RET

io_cli:
	CLI
	RET

io_sti:
	STI
	RET

io_stihlt:
	STI
	HLT
	RET

io_in8:	;int io_in8(int port)
	mov		edx, [esp+4]	;port
	mov		eax, 0
	in		al, dx
	ret

io_in16: ;int io_in16(int port)
	mov		edx, [esp+4]	;port
	mov		eax, 0
	in		ax, dx
	ret

io_in32: ;int io_in32(int port)
	mov		edx, [esp+4]	;port
	in		eax, dx
	ret


io_out8: ;void io_out8(int port, int data)
	mov		edx, [esp+4]	;port
	mov		al, [esp+8]		;data
	out		dx, al
	ret

io_out16: ;void io_out16(int port, int data)
	mov		edx, [esp+4]
	mov		ax, [esp+8]
	out		dx, ax
	ret

io_out32: ;void io_out32(int port, int data)
	mov		edx, [esp+4]
	mov		eax, [esp+8]
	out		dx, eax
	ret

io_load_eflags: ;int io_load_eflags(void)
	pushfd			;push eflags
	pop		eax
	ret

io_store_eflags: ;void io_store_eflags(int eflags)
	mov		eax, [esp+4]
	push	        eax
	popfd			;pop eflags
	ret

load_cr0: ; void load_cr0(void)
        mov             eax, cr0
        ret

store_cr0: ; void store_cr0(int cr0)
        mov             eax, [esp+4]
        mov             cr0, eax
        ret

load_gdtr: ;void load_gdt(int limit, int addr)
	mov		ax, [esp+4]
	mov		[esp+6], ax
	lgdt	        [esp+6]
	ret

load_idtr:
	mov		ax, [esp+4]
	mov		[esp+6], ax
	lidt	        [esp+6]
	ret

load_tr:
        ltr             [esp + 4]
        ret

farjmp: ; farjmp (int eip, int cs);
        jmp             far [esp + 4]
        ret

farcall: 
        call            far [esp + 4]
        ret

inthandler21_asm:
        push            es
        push            ds
        pushad
        mov             eax, esp
        push            eax
        mov             ax,ss
        mov             ds,ax
        mov             es,ax
        call    inthandler21
        pop             eax
        popad
        pop             ds
        pop             es
        iretd

inthandler2c_asm:
        push            es
        push            ds
        pushad
        mov             eax, esp
        push            eax
        mov             ax,ss
        mov             ds,ax
        mov             es,ax
        call    inthandler2c
        pop             eax
        popad
        pop             ds
        pop             es
        iretd

inthandler20_asm:
        push            es
        push            ds
        pushad
        mov             eax, esp
        push            eax
        mov             ax,ss
        mov             ds,ax
        mov             es,ax
        call    inthandler20
        pop             eax
        popad
        pop             ds
        pop             es
        iretd

inthandler0d_asm:
        sti 
        push            es
        push            ds
        pushad
        mov             eax, esp
        push            eax
        mov             ax, ss
        mov             ds, ax
        mov             es, ax
        call            inthandler0d
        cmp             eax, 0
        jne             end_app_asm
        pop             eax
        popad
        pop             ds
        pop             es
        add             esp, 4
        iretd

inthandler0c_asm:
        sti
        push            es
        push            ds
        pushad
        mov             eax, esp
        push            eax
        mov             ax, ss
        mov             ds, ax
        mov             es, ax
        call            inthandler0c
        cmp             eax, 0
        jne             end_app_asm
        pop             eax
        popad
        pop             ds
        pop             es
        add             esp, 4
        iretd

hrb_api_asm:
        sti
        push            ds
        push            es
        pushad
        pushad
        mov             ax, ss
        mov             ds, ax
        mov             es, ax
        call            hrb_api
        cmp             eax, 0
        jne             end_app_asm
        add             esp, 32
        popad
        pop             es
        pop             ds
        iretd

end_app_asm:
        mov             esp, [eax]
        mov             dword [eax + 4], 0
        popad
        ret

start_app:
        pushad
        mov             eax, [esp + 36]
        mov             ecx, [esp + 40]
        mov             edx, [esp + 44]
        mov             ebx, [esp + 48]
        mov             ebp, [esp + 52]
        mov             [ebp], esp
        mov             [ebp + 4], ss
        mov             es, bx
        mov             ds, bx
        mov             fs, bx
        mov             gs, bx

        or              ecx, 3
        or              ebx, 3
        push            ebx
        push            edx
        push            ecx
        push            eax
        retf

