;geocide-os
BOTPAK	EQU		0x00280000
DSKCAC	EQU		0x00100000
DSKCAC0	EQU		0x00008000


;BOOT_INFO
CYLS	EQU		0x0ff0
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2
SCRNX	EQU		0x0ff4
SCRNY	EQU		0x0ff6
VRAM	EQU		0x0ff8

VBEMODE EQU             0x105

	ORG		0xc200

; check vbe
        mov             ax, 0x9000
        mov             es, ax
        MOV             di, 0
        mov             ax, 0x4f00
        int             0x10
        cmp             ax, 0x004f
        jne             scrn320
        mov             ax, [es:di+4]
        cmp             ax, 0x0200
        jb              scrn320

        mov             cx, VBEMODE
        mov             ax, 0x4f01
        int             0x10
        cmp             ax, 0x004f
        jne             scrn320
        cmp             byte [es:di+0x19], 8
        jne             scrn320
        cmp             byte [es:di+0x1b], 4
        jne             scrn320
        mov             ax, [es:di+0x00]
        and             ax, 0x0080
        jz              scrn320

        mov             bx, VBEMODE+0x4000
        mov             ax, 0x4f02
	INT		0x10			;VGAgraphics 320x200x8bit color
	MOV		BYTE [VMODE], 8
        mov             ax, [es:di+0x12]
	MOV		WORD [SCRNX], ax
        mov             ax, [es:di+0x14]
	MOV		WORD [SCRNY], ax
        mov             eax, [es:di+0x28]
	MOV		DWORD [VRAM], eax
        jmp             keystatus

scrn320:
        mov             al, 0x13
        mov             ah, 0x00
        int             0x10
        mov             BYTE [VMODE], 8
        mov             WORD [SCRNX], 320
        mov             WORD [SCRNY], 200
        mov             DWORD [VRAM], 0x000a0000

;keyboard led
keystatus:
	MOV		AH, 0x02
	INT		0x16
	MOV		[LEDS], AL
	
;
	MOV		AL, 0xff
	OUT		0x21, AL
	NOP
	OUT		0xa1, AL

	CLI
	
	CALL	waitkbdout
	MOV		AL, 0xd1
	OUT		0x64, AL
	CALL	waitkbdout
	MOV		AL, 0xdf
	OUT		0x60, AL
	CALL	waitkbdout

;protect mode

	LGDT	[GDTR0]
	MOV		EAX, CR0
	AND		EAX, 0x7fffffff
	OR		EAX, 0x00000001
	MOV		CR0, EAX
	JMP		pipelineflush
pipelineflush:
	MOV		AX, 1*8
	MOV 	DS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		SS, AX

;transport bootpack
	MOV		ESI, bootpack
	MOV		EDI, BOTPAK
	MOV		ECX, 512*1024/4
	CALL 	memcpy

	MOV		ESI, DSKCAC0+512
	MOV		EDI, DSKCAC+512
	MOV		ECX, 0
	MOV		CL, BYTE [CYLS]
	IMUL	ECX, 512*18*2/4
	SUB		ECX, 512/4
	CALL	memcpy

;excute bootpack
	MOV		EBX, BOTPAK
	MOV 	ECX, [EBX+16]
	ADD		ECX, 3
	SHR		ECX, 2
	JZ		skip
	MOV		ESI, [EBX+20] 
	ADD		ESI, EBX
	MOV		EDI, [EBX+12]
	CALL 	memcpy
skip:
	MOV		ESP, [EBX+12]
	JMP		DWORD 2*8:0x0000001B

waitkbdout:
	IN		AL, 0x64
	AND		AL, 0x02
	JNZ		waitkbdout
	RET

memcpy:
	MOV		EAX, [ESI]
	ADD		ESI, 4
	MOV		[EDI], EAX
	ADD		EDI, 4
	SUB		ECX, 1
	JNZ		memcpy
	RET
	
	TIMES 16 DB 0
GDT0:
	TIMES 8 DB 0
	DW		0xffff, 0x0000, 0x9200, 0x00cf
	DW		0xffff, 0x0000, 0x9a28, 0x0047

	DW		0
GDTR0:
	DW		8*3-1
	DD		GDT0

	TIMES 16 DB 0
bootpack:
