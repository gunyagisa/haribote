#pragma once

#define BOOTINFO_ADDR 	0x00000ff0

#define EFLAGS_AC_BIT           0x00040000
#define CR0_CACHE_DISABLE       0x60000000

#define MEMMAN_ADDR     0x003c0000

#include "fifo.h"

// nasmfunc.asm
extern void io_hlt(void);
extern void io_cli(void), io_sti(void), io_stihlt(void);
extern void io_out8(int port, int data), io_out16(int port, int data), io_out32(int port, int data);
extern int io_load_eflags(void), io_in8(int port), io_in16(int port), io_in32(int port),load_cr0();
extern void io_store_eflags(int eflags), store_cr0(int cr0);
extern void load_gdtr(int limit, int addr);
extern void load_idtr(int limit, int addr);
extern void inthandler21_asm(void);
extern void inthandler2c_asm(void);
extern void inthandler20_asm(void);

extern void init_pit(void);

//struct to store the boot information written in asmhead.asm
typedef struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
} BOOTINFO;

extern FIFO8  keyfifo;
extern FIFO8  mousefifo;

struct TIMERCTL {
    unsigned int count;
};
extern struct TIMERCTL timer;
