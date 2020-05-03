;asmfunc.asm

bits 32

global	io_hlt , io_cli, io_sti, io_stihlt
global 	io_out8 ,io_out16, io_out32, io_in8, io_in16, io_in32
global	io_store_eflags ,io_load_eflags, store_cr0, load_cr0
global	load_gdtr, load_idtr
global  inthandler21_asm, inthandler2c_asm, inthandler20_asm, inthandler0d_asm
global  load_tr, farjmp, farcall
global  hrb_api_asm, start_app

EXTERN  inthandler21, inthandler2c, inthandler20, inthandler0d
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
        mov             ax, ss
        cmp             ax, 1*8
        jne             .from_app

        mov             eax, esp
        push            ss
        push            eax
        mov             ax, ss
        mov             ds, ax
        mov             es, ax
        call            inthandler0d
        add             esp, 8
        popad
        pop             ds
        pop             es
        add             esp, 4
        iretd

.from_app
        cli
        mov             eax, 1 * 8
        mov             ds, ax
        mov             ecx, [0xfe4]
        add             ecx, -8
        mov             [ecx + 4], ss
        mov             [ecx], esp
        mov             ss, ax
        mov             es, ax
        mov             esp, ecx
        sti
        call            inthandler0d
        cli
        cmp             eax, 0
        jne             .kill
        pop             ecx
        pop             eax
        mov             ss, ax
        mov             esp, ecx
        popad
        pop             ds
        pop             es
        add             esp, 4
        iretd
.kill
        mov             eax, 1 * 8
        mov             es, ax
        mov             ss, ax
        mov             ds, ax
        mov             fs, ax
        mov             gs, ax
        mov             esp, [0xfe4]
        sti
        popad
        ret

hrb_api_asm:
        push            ds
        push            es
        pushad
        mov             eax, 1 * 8
        mov             ds, ax
        mov             ecx, [0xfe4]
        add             ecx, -40
        mov             [ecx + 32], esp
        mov             [ecx + 36], ss
        mov             edx, [esp]
        mov             ebx, [esp + 4]
        mov             [ecx], edx
        mov             [ecx + 4], ebx
        mov             edx, [esp + 8]
        mov             ebx, [esp + 12]
        mov             [ecx + 8], edx
        mov             [ecx + 12], ebx
        mov             edx, [esp + 16]
        mov             ebx, [esp + 20]
        mov             [ecx + 16], edx
        mov             [ecx + 20], ebx
        mov             edx, [esp + 24]
        mov             ebx, [esp + 28]
        mov             [ecx + 24], edx
        mov             [ecx + 28], ebx

        mov             es, ax
        mov             ss, ax
        mov             es, ecx

        sti
        call            hrb_api
        mov             ecx, [esp + 32]
        mov             eax, [esp + 36]
        cli
        mov             ss, ax
        mov             esp, ecx
        popad
        pop             es
        pop             ds
        iretd

start_app:
        pushad
        mov             eax, [esp + 36]
        mov             ecx, [esp + 40]
        mov             edx, [esp + 44]
        mov             ebx, [esp + 48]
        mov             [0xfe4], esp
        cli
        mov             es, bx
        mov             ss, bx
        mov             ds, bx
        mov             fs, bx
        mov             gs, bx
        mov             esp, edx
        sti
        push            ecx
        push            eax
        call far        [esp]

        mov             eax, 1*8
        cli 
        mov             es, ax
        mov             ss, ax
        mov             ds, ax
        mov             fs, ax
        mov             gs, ax
        mov             esp, [0xfe4]
        sti
        popad
        ret

