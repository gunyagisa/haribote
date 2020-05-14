; geocide
CYLS EQU	20
org 0x7c00

; FAT12 setting
    JMP    SHORT entry
    DB      0x90
    DB      "GEOCIDE "
    DW      512
    DB      1
    DW      1
    DB      2
    DW      224
    DW      2880
    DB      0xf0
    DW      9
    DW      18
    DW      2
    DD      0
    DD      2880
    DB      0,0,0x29
    DD      0xffffffff
    DB      "HELLO-OS   "
    DB      "FAT12   "
    TIMES    18 DB 0

entry:
    MOV     AX, 0
    MOV     SS, AX
    MOV     SP, 0x7c00
    MOV     DS, AX
;read disk    
	MOV		AX, 0x0820		
	MOV		ES, AX			;ES = 0x0820 
	MOV		CH, 0			;cylinder number & 0xff
	MOV		DH, 0			;head number(=0)
	MOV		CL, 2			;sector number(=2)
readloop:
	MOV		SI, 0
retry:
	MOV		AH, 0x02		;AH = 0x2 , 
	MOV		AL, 1			;
	MOV		BX, 0			
	MOV		DL, 0x00		;A drive
	INT		0x13
	JNC		next
	ADD		SI, 1
	CMP		SI, 5
	JAE		error
	MOV		AH, 0x00
	MOV		DL, 0x00
	INT		0x13			;system reset (AH=0x00, DL=0x00, INT 0x13)
	JMP 	retry
next:
	MOV		AX, ES
	ADD		AX, 0x20		;512/16
	MOV		ES, AX
	ADD		CL, 1			;CL++
	CMP		CL, 18
	JBE		readloop		;sector number(=CL) <= 18 -> readloop
	MOV		CL, 1
	ADD		DH, 1
	CMP		DH, 2
	JB		readloop		;head number(=DH) < 2 -> readloop
	MOV		DH, 0			;DH = 0
	ADD		CH, 1			;CH += 1
	CMP		CH, CYLS
	JBE		readloop		;cylinder number(=CH) <= CYLS -> readloop
	
	MOV		[0x0ff0], CH
	JMP		0xc200
	JMP 	success
fin:
	HLT
	JMP 	fin

success:
	MOV		SI, msg2
	JMP		putloop
error:
	MOV		SI, msg1
putloop:
    MOV     AL, [SI]
    ADD     SI, 1
    CMP     AL, 0
    JE		fin
    MOV		AH, 0x0e
	MOV		BX,	15
	INT		0x10
	JMP		putloop

msg1:
	DB		0x0a, 0x0a
	DB		"ERROR!"
	DB		0x0a
	DB		0
msg2:
	DB		0x0a, 0x0a
	DB		"success!"
	DB		0x0a
	DB		0

	TIMES	0x01fe-($-$$) DB 0
; 0x7dfe‚Ü‚Å‚ð0x00‚Å–„‚ß‚é–½—ß
	DB		0x55, 0xaa
